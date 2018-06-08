#include "influxdb_metrics_service.h"

#include <time.h>
#include <assert.h>
#include <json-c/json.h>
#include <mysql/mysql.h>

#include "common/common.h"
#include "common/climit.h"
#include "common/log.h"
#include "common/message.h"
#include "minIni/minIni.h"

#include "common/kafka_util.h"

static char g_db_host[IP_ADDRESS_LEN_MAX] = {0};
static char g_db_uname[USERNAME_MAX] = {0};
static char g_db_pwd[PASSWORD_MAX] = {0};

static char g_kafka_brokers[KAFKA_BROKERS_LEN_MAX] = {0};
static rd_kafka_t *influxdb_metrics_srv_consumer_rk;
static rd_kafka_t *influxdb_metrics_srv_producer_rk;

MYSQL *g_mysql = NULL;

// function declare
int check_device_is_exsited(const char* id);
int save_inform_data(json_object* msg_json);
void notify_task_managemenet_service(const char* id);

static void msg_consume (rd_kafka_message_t *rkmessage, void *opaque) {
	if (rkmessage->err) {
		if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
			log_debug(
                      "%% Consumer reached end of %s [%"PRId32"] "
                      "message queue at offset %"PRId64"\n",
                      rd_kafka_topic_name(rkmessage->rkt),
                      rkmessage->partition, rkmessage->offset);
			return;
		}

		log_error("%% Consume error for topic \"%s\" [%"PRId32"] "
                  "offset %"PRId64": %s\n",
                  rd_kafka_topic_name(rkmessage->rkt),
                  rkmessage->partition,
                  rkmessage->offset,
                  rd_kafka_message_errstr(rkmessage));

        if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION ||
            rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC)
            // run = 0;
            return;
	}

    log_debug("%% Message (offset %"PRId64", %zd bytes):\n",
              rkmessage->offset, rkmessage->len);

	if (rkmessage->key_len) {
        log_debug("Key: %.*s\n",
                  (int)rkmessage->key_len, (char *)rkmessage->key);
	}

    log_debug("%.*s\n",
              (int)rkmessage->len, (char *)rkmessage->payload);



    char* temp = strndup((char*)rkmessage->payload, (int)rkmessage->len);
    json_object *msg_json = json_tokener_parse(temp);
    free(temp);
    temp = NULL;

    error_code_t code = ERROR_CODE_OK;

    char* identify = NULL;
    json_object *identify_json = NULL;
    if (json_object_object_get_ex(msg_json, "identify", &identify_json)) {
        if (json_object_get_string(identify_json))
            identify = strdup(json_object_get_string(identify_json));
    }

    json_object *msg_body_json = NULL;
    if (json_object_object_get_ex(msg_json, "msg_body", &msg_body_json)) {
        json_object* id_json = NULL;
        const char* id = NULL;
        if (json_object_object_get_ex(msg_body_json, "id", &id_json)) {
            id = json_object_get_string(id_json);
        }
        assert(id != NULL);

/*        json_object* data_json = NULL;
        if (json_object_object_get_ex(msg_body_json, "data", &data_json)){
            array_list* data_array = json_object_get_array(data_json);
            for (int i = 0;i< data_array->length;i++){
                json_object* temp_json = NULL;
                char* plugin = NULL;
                if (json_object_object_get_ex(data_array->array[i], "p", &temp_json)) {
                    plugin = json_object_get_string(temp_json);
                    printf("%s\n",plugin);
                }
            }          
        }
*/
//    }
        
        if (id[0] == '\0' || check_device_is_exsited(id) == 0) {
            code = ERROR_CODE_NO_DEVICE_FOUND;
        }
        else {
            json_object* type_json = NULL;
            const char* type = NULL;
            if (json_object_object_get_ex(msg_body_json, "type", &type_json)) {
                type = json_object_get_string(type_json);
            }

            if (type && strcmp(type, MESSAGE_TYPE_INFLUXDB_METRICS) == 0) {
                //insert metrics into mysql
		if (save_metrics_data(msg_body_json) < 0){
		    code = ERROR_CODE_SAVE_INFLUXDB_METRICS_ERROR;
		}

            }
            else {
                log_error("failed to match request type[%s]", type);
                code = ERROR_CODE_UNKNOWN_REQUEST;
            }
        }
    }
    else {
        code = ERROR_CODE_FORMAT_ERROR;
    }

    json_object_put(msg_json);

    const char* service_name = get_service_name(MESSAGE_TYPE_INFLUXDB_METRICS_RESPONSE);

    struct json_object *resp_json = json_object_new_object();
    struct json_object *resp_msg_body_json = json_object_new_object();
    json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_INFLUXDB_METRICS_RESPONSE));
//    json_object_object_add(resp_msg_body_json, "type", json_object_new_string("inform_resp"));
    json_object_object_add(resp_msg_body_json, "resp_code", json_object_new_int(code));
    json_object_object_add(resp_msg_body_json, "resp_msg", json_object_new_string(get_error_string(code)));

    json_object_object_add(resp_json, "identify", json_object_new_string(identify));
    json_object_object_add(resp_json, "msg_body", resp_msg_body_json);

    const char* msg = json_object_to_json_string(resp_json);
    send_msg(influxdb_metrics_srv_producer_rk, service_name, msg, strlen(msg));
    json_object_put(resp_json);

    free(identify);

}

int check_device_is_exsited(const char* id) {
    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);

    sprintf(sql, "select COUNT(*) from postinfo where id = \'%s\'", id);

    if (mysql_query(g_mysql, sql)) {
        log_error("failed to query postinfo[%s].", mysql_error(g_mysql));
        log_error("sql:[%s]", sql);
        free(sql);
        return -1;
    }

    MYSQL_RES *result = NULL;
    MYSQL_ROW row;
    result = mysql_store_result(g_mysql);
    if (!result) {
        log_error("failed to read query result.[%s]", mysql_error(g_mysql));
        free(sql);
        return -1;
    }

    int count = -1;
    if (mysql_num_rows(result) > 0 ) {
        if ((row = mysql_fetch_row(result))) {
            count = atoi(row[0]);
        }
    }
    mysql_free_result(result);
    free(sql);
    if (count > 1) {
        log_error("find device more than one with id[%s]", id);
    }
    return count;
}

int save_metrics_data(json_object* msg_json) {
    const char* id = NULL;
    json_object* id_json = NULL;
    if (!json_object_object_get_ex(msg_json, "id", &id_json)) {
        return -1;
    }
    id = json_object_get_string(id_json);

    json_object* data_json = NULL;
    if (!json_object_object_get_ex(msg_json, "data", &data_json)) {
        return -1;
    }
        
    if(json_object_object_get_ex(msg_json, "data", &data_json)) {
        if(data_json == NULL) {
             return -1;
        }
    }

    json_object* index_json = NULL;
    const char* index = "";
    if (!json_object_object_get_ex(msg_json, "index", &index_json)) {
        return -1;
    }
    index = json_object_get_string(index_json);

    json_object* total_json = NULL;
    const char* total = "";
    if (!json_object_object_get_ex(msg_json, "total", &total_json)) {
        return -1;
    }
    total = json_object_get_string(total_json);

    if(atoi(index) - atoi(total) >= 0 ){
        return -1;
    }

    if (strcmp(index,"0") == 0){
        //delete all metrics by id
        char *del_sql = (char*) malloc(1024);
        memset(del_sql, 0, 1024);
        sprintf(del_sql,"delete from post_metrics where postinfo_id = \'%s\';",id);
        if (mysql_query(g_mysql, del_sql)) {
            log_error("failed to delete post_metrics by id [%s].", mysql_error(g_mysql));
            printf("failed to delete post_metrics by id %s \n",mysql_error(g_mysql));
            log_error("del_sql:[%s]", del_sql);
            free(del_sql);
            return -1;
        }
    }

#define SQL_SIZE 10240
    char* sql = (char*) malloc(SQL_SIZE);
    memset(sql, 0, SQL_SIZE);

    strcpy(sql,"insert into post_metrics(postinfo_id,plugin,plugin_instance,type,type_instance,dsname,metric_index) values ");

    if (json_object_object_get_ex(msg_json, "data", &data_json)){
        array_list* data_array = json_object_get_array(data_json);
        char* all_values = (char*) malloc(SQL_SIZE-128);
        memset(all_values, 0, SQL_SIZE-128);
        for (int i = 0;i< data_array->length;i++){
            char *values = (char*) malloc(SQL_SIZE/2);
            memset(values, 0, SQL_SIZE/2);

            json_object* temp_json = NULL;
            const char* plugin = NULL;
            const char* plugin_instance = NULL;
            const char* type = NULL;
            const char* type_instance = NULL;
            int pi = 0;
            int ti = 0;
            
            if (json_object_object_get_ex(data_array->array[i], "p", &temp_json)) {
                plugin = json_object_get_string(temp_json);
//                printf("%s ",plugin);
            }
            
            if (json_object_object_get_ex(data_array->array[i], "pi", &temp_json)) {
                plugin_instance = json_object_get_string(temp_json);
//                printf("%s ",plugin_instance); 
                pi = 1;
            }

            if (json_object_object_get_ex(data_array->array[i], "t", &temp_json)) {
                type = json_object_get_string(temp_json);
//                printf("%s ",type);
            }

            if (json_object_object_get_ex(data_array->array[i], "ti", &temp_json)) {
                type_instance = json_object_get_string(temp_json);
//                printf("%s ",type_instance);
                ti = 1;
            }

            json_object* dsnames_json = NULL;
            if (!json_object_object_get_ex(data_array->array[i], "d", &dsnames_json)){
                char *value = (char*) malloc(SQL_SIZE/10);
                memset(value, 0, SQL_SIZE/10);
                if(pi == 0 && ti == 0){
                    char *index = (char*)malloc(256);
                    sprintf(index,"%s/%s",plugin,type);
                    sprintf(value,"(\'%s\',\'%s\',\'\',\'%s\',\'\',\'value\',\'%s\'),",id,plugin,type,index);
                    free(index);
                    index = NULL;
                }else if(pi == 1 && ti == 0){
                    char *index = (char*)malloc(256);
                    sprintf(index,"%s/%s",plugin,type);
                    sprintf(value,"(\'%s\',\'%s\',\'%s\',\'%s\',\'\',\'value\',\'%s\'),",id,plugin,plugin_instance,type,index);
                    free(index);
                    index = NULL;
                }else if(pi == 0 && ti == 1){
                    char *index = (char*)malloc(256);
                    sprintf(index,"%s/%s-%s",plugin,type,type_instance);
                    sprintf(value,"(\'%s\',\'%s\',\'\',\'%s\',\'%s\',\'value\',\'%s\'),",id,plugin,type,type_instance,index);
                    free(index);
                    index = NULL;
                }else{
                    char *index = (char*)malloc(256);
                    sprintf(index,"%s/%s-%s",plugin,type,type_instance);
                    sprintf(value,"(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'value\',\'%s\'),",id,plugin,plugin_instance,type,type_instance,index);
                    free(index);
                    index = NULL;
                }

//                printf("value \n");
                strcat(values,value);
                free(value);
                value = NULL;    
            }else{
                int dsnames_len = json_object_array_length(dsnames_json);
                for (int j = 0;j< dsnames_len;j++){
                    char *value = (char*) malloc(SQL_SIZE/20);
                    memset(value, 0, SQL_SIZE/20);
                    json_object* dsname_json = NULL;
                    dsname_json = json_object_array_get_idx(dsnames_json,j);
                    const char* dsname = NULL;
                    dsname = json_object_get_string(dsname_json);
//                    printf("%s ",dsname);
                    if(pi == 0 && ti == 0){
                        char *index = (char*)malloc(256);
                        sprintf(index,"%s/%s",plugin,type);
                        sprintf(value,"(\'%s\',\'%s\',\'\',\'%s\',\'\',\'%s\',\'%s\'),",id,plugin,type,dsname,index);
                        free(index);
                        index = NULL;
                    }else if(pi == 1 && ti == 0){
                        char *index = (char*)malloc(256);
                        sprintf(index,"%s/%s",plugin,type);
                        sprintf(value,"(\'%s\',\'%s\',\'%s\',\'%s\',\'\',\'%s\',\'%s\'),",id,plugin,plugin_instance,type,dsname,index);
                        free(index);
                        index = NULL;
                    }else if(pi == 0 && ti == 1){
			char *index = (char*)malloc(256);
                        sprintf(index,"%s/%s-%s",plugin,type,type_instance);
                        sprintf(value,"(\'%s\',\'%s\',\'\',\'%s\',\'%s\',\'%s\',\'%s\'),",id,plugin,type,type_instance,dsname,index);
                        free(index);
                        index = NULL;
                    }else{
                        char *index = (char*)malloc(256);
                        sprintf(index,"%s/%s-%s",plugin,type,type_instance);
                        sprintf(value,"(\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\'),",id,plugin,plugin_instance,type,type_instance,dsname,index);
                        free(index);
                        index = NULL;
                    }
                    strcat(values,value);
                    free(value);
                    value = NULL;
                }
//		printf("\n");
            }
            strcat(all_values,values);
        }  
        all_values[strlen(all_values)-1] = '\0';
        strcat(sql,all_values);
        strcat(sql,";");
        printf("%s \n",sql);
    }

    if (mysql_query(g_mysql, sql)) {
            log_error("failed to insert post_metrics[%s].", mysql_error(g_mysql));
            printf("failed to insert post_metrics%s \n",mysql_error(g_mysql));
            log_error("sql:[%s]", sql);
            free(sql);
            return -1;
        }

    free(sql);
    return 0;
}

static const char *group = "influxdb_metrics_srv_group";
static const char *topic = INFLUXDB_METRICS_SERVICE;
void start_message_handler() {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *topics = NULL;

    init_consumer_rk(&influxdb_metrics_srv_consumer_rk, group, g_kafka_brokers);

    /* Create topic */
    topics = rd_kafka_topic_partition_list_new(3);
    rd_kafka_topic_partition_list_add(topics, topic, -1);

    if ((err = rd_kafka_subscribe(influxdb_metrics_srv_consumer_rk, topics))) {
        log_error("%% Failed to start consuming topics: %s\n",
                rd_kafka_err2str(err));
        exit(1);
    }

    
    while (1) {
        rd_kafka_message_t *rkmessage;

        rkmessage = rd_kafka_consumer_poll(influxdb_metrics_srv_consumer_rk, 1000);

        if (rkmessage) {
            msg_consume(rkmessage, NULL);
            rd_kafka_message_destroy(rkmessage);
        }
    }

    /* Stop consuming */
    err = rd_kafka_consumer_close(influxdb_metrics_srv_consumer_rk);
    if (err)
        log_error("%% Failed to close consumer: %s\n",
                  rd_kafka_err2str(err));
    else
        log_debug("%% Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(topics);

    free_consumer_rk(influxdb_metrics_srv_consumer_rk);
}

int init_db_connection(const char* db_host, const char* db_username, const char* db_passwd)
{
    if (NULL == g_mysql)
        {
            g_mysql = (MYSQL *) malloc(sizeof(MYSQL));
            if (NULL == g_mysql)
                {
                    log_error("failed to malloc memory for mysql handler");
                    return -1;
                }
        }

    memset(g_mysql, 0, sizeof(MYSQL));
    
    mysql_init(g_mysql);
    char value = 1;
    mysql_options(g_mysql, MYSQL_OPT_RECONNECT, &value);
    if(!mysql_real_connect(g_mysql, db_host, db_username, db_passwd,
                           "infoDB", 0, NULL, 0)) 
        {
            log_error("failed to connection mysql service.");
            return -1;
        }
    mysql_set_character_set(g_mysql, "utf8");

    char* sql = 
        "create table IF NOT EXISTS post_metrics("
	"id int(11) unsigned NOT NULL AUTO_INCREMENT,"
        "postinfo_id varchar(64) DEFAULT NULL,"
        "plugin varchar(64) DEFAULT NULL,"
        "plugin_instance varchar(64) DEFAULT NULL,"
        "type varchar(64) DEFAULT NULL,"
        "type_instance varchar(64) DEFAULT NULL,"
        "dsname varchar(32) DEFAULT NULL,"
        "PRIMARY KEY (`id`)"
        ")";

    if (mysql_query(g_mysql, sql))
        {
            log_error("sql:[%s]", sql);
            log_error("failed to create table post_metrics[%s].", mysql_error(g_mysql));
            return -1;
        }

    return 0;
}

int destroy_db_connection() {
    if (g_mysql) {
        mysql_close(g_mysql);
        g_mysql = NULL;
    }
    return 0;
}

int main() {
    assert(initlog(NULL, "influxdb_metrics_service") >= 0);

    if (ini_gets("mysql", "host", "", g_db_host, sizeof(g_db_host), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get mysql host!");
        strncpy(g_db_host, DEFAULT_DATABASE_HOST, sizeof(g_db_host));
    }
    // get username for db service
    if (ini_gets("mysql", "username", "", g_db_uname, sizeof(g_db_uname), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get mysql username!");
        strncpy(g_db_uname, DEFAULT_DATABASE_USERNAME, sizeof(g_db_uname));
    }
    // get password for db service
    if (ini_gets("mysql", "password", "", g_db_pwd, sizeof(g_db_pwd), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get mysql passwd!");
        strncpy(g_db_pwd, DEFAULT_DATABASE_PASSWORD, sizeof(g_db_pwd));
    }

    if (ini_gets("kafka", "brokers", "localhost:9092", g_kafka_brokers, sizeof(g_kafka_brokers), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get kafka brokers!");
        strncpy(g_kafka_brokers, "localhost:9092", sizeof(g_kafka_brokers));
    }

    assert(init_db_connection(g_db_host, g_db_uname, g_db_pwd) >= 0);

    init_producer_rk(&influxdb_metrics_srv_producer_rk, g_kafka_brokers);
    start_message_handler();
    free_producer_rk(influxdb_metrics_srv_producer_rk);
    destroy_db_connection();
    releaseLog();
    return 0;
}

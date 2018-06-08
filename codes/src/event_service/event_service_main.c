#include "event_service_main.h"

#include <time.h>
#include <assert.h>
#include <json-c/json.h>
#include <mysql/mysql.h>

#include "minIni/minIni.h"
#include "common/common.h"
#include "common/climit.h"
#include "common/log.h"
#include "common/message.h"
#include "common/kafka_util.h"

#define SQL_SIZE 10240

static char g_db_host[IP_ADDRESS_LEN_MAX] = {0};
static char g_db_uname[USERNAME_MAX] = {0};
static char g_db_pwd[PASSWORD_MAX] = {0};

static char g_kafka_brokers[KAFKA_BROKERS_LEN_MAX] = {0};
static rd_kafka_t *event_srv_consumer_rk;
static rd_kafka_t *event_srv_producer_rk;

MYSQL *g_mysql = NULL;

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

int save_vpn_info(const char* id, json_object *vpn_json) {
    char* sql = (char*) malloc(SQL_SIZE);
    memset(sql, 0, SQL_SIZE);

    char server_time[DATETIME_LEN_MAX] = {0};
    time_t now = time(NULL);
    // strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", gmtime(&now));
    strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", localtime(&now));

    const char* vpn_name = "";
    int status = 0;
    const char* client_cn = "";
    const char* client_ip = "";
    const char* client_vpn_ip = "";
    const char* server_cn = "";
    const char* server_ip = "";
    const char* vpn_state = "";
    const char* vpn_cert_info = "";
    char* vpn_validity_start = NULL;
    char* vpn_validity_end = NULL;

    json_object* temp_json = NULL;
    if (json_object_object_get_ex(vpn_json, "name", &temp_json)) {
        vpn_name = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "status", &temp_json)) {
        status = json_object_get_boolean(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "ccn", &temp_json)) {
        client_cn = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "cip", &temp_json)) {
        client_ip = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "vpn_cip", &temp_json)) {
        client_vpn_ip = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "scn", &temp_json)) {
        server_cn = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "sip", &temp_json)) {
        server_ip = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "state", &temp_json)) {
        vpn_state = json_object_get_string(temp_json);
    }
    if (json_object_object_get_ex(vpn_json, "vpn_cert_info", &temp_json)) {
        vpn_cert_info = json_object_get_string(temp_json);
    }

    if (vpn_cert_info) {
        const char* vpn_cert_validity = strstr(vpn_cert_info, "validity:");
        if (vpn_cert_validity) {
            const char* start = strstr(vpn_cert_validity, "not before");
            const char* end = NULL;
            if (start) {
                start += strlen("not before");
                while (*start == ' ') start++;
                end = start;
                while (*end != '\n' && *end != '\0' && *end != ',') end++;
                int len = end - start;
                if (len > 0) vpn_validity_start = strndup(start, len);
            }
            start = strstr(vpn_cert_validity, "not after");
            if (start) {
                start += strlen("not after");
                while (*start == ' ') start++;
                end = start;
                while (*end != '\n' && *end != '\0' && *end != ',') end++;
                int len = end - start;
                if (len > 0) vpn_validity_end = strndup(start, len);
            }
        }

        /* memset(sql, 0, SQL_SIZE); */
        /* sprintf(sql, "UPDATE postinfo SET vpn_cert_start_date=str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y'), vpn_cert_end_date=str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y')  where id = \'%s\'",  */
        /*         vpn_validity_start ? vpn_validity_start : "", vpn_validity_end ? vpn_validity_end : "", id); */
        /* if (mysql_query(g_mysql, sql)) { */
        /*     log_error("failed to update postinfo[%s].", mysql_error(g_mysql)); */
        /*     log_error("sql:[%s]", sql); */
        /*     free(sql); */
        /*     free(vpn_validity_start); */
        /*     free(vpn_validity_end); */
        /*     return -1; */
        /* } */

    }

    memset(sql, 0, SQL_SIZE);
    sprintf(sql, "INSERT INTO device_vpn_info("
            "postinfo_id, vpn_name, status, client_cn, client_ip, client_vpn_ip, "
            "server_cn, server_ip, vpn_state, vpn_cert_info, "
            "vpn_cert_start_date, vpn_cert_end_date, "
            "updated_at)"
            " VALUES("
            "\'%s\', \'%s\', \'%d\', \'%s\', \'%s\', \'%s\',"
            "\'%s\', \'%s\', \'%s\', \'%s\', "
            "str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y'), str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y'),"
            "\'%s\')"
            "ON DUPLICATE KEY UPDATE "
            "vpn_name=\'%s\', status=\'%d\', client_cn=\'%s\', client_ip=\'%s\', client_vpn_ip=\'%s\',"
            "server_cn=\'%s\', server_ip=\'%s\', vpn_state=\'%s\', vpn_cert_info=\'%s\',"
            "vpn_cert_start_date=str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y'), vpn_cert_end_date=str_to_date(\'%s\', '%%b %%d %%H:%%i:%%S %%Y'),"
            "updated_at=\'%s\'",
            id, vpn_name, status, client_cn, client_ip, client_vpn_ip,
            server_cn, server_ip, vpn_state, vpn_cert_info, 
            vpn_validity_start?vpn_validity_start:"", vpn_validity_end?vpn_validity_end:"",
            server_time,
            vpn_name, status, client_cn, client_ip, client_vpn_ip, 
            server_cn, server_ip, vpn_state, vpn_cert_info, 
            vpn_validity_start?vpn_validity_start:"", vpn_validity_end?vpn_validity_end:"",
            server_time
            );
    if (vpn_validity_start) free(vpn_validity_start);
    if (vpn_validity_end) free(vpn_validity_end);

    if (mysql_query(g_mysql, sql)) {
        log_error("failed to insert device_vpn_info[%s].", mysql_error(g_mysql));
        log_error("sql:[%s]", sql);
        free(sql);
        return -1;
    }
    free(sql);
    return 0;
}

int save_setting_monitor_event(json_object *msg_json) {
    const char* id = NULL;
    json_object* id_json = NULL;
    if (!json_object_object_get_ex(msg_json, "id", &id_json)) {
        return -1;
    }
    id = json_object_get_string(id_json);

    const char* type = NULL;
    json_object* type_json = NULL;
    if (!json_object_object_get_ex(msg_json, "type", &type_json)) {
        return -1;
    }
    type = json_object_get_string(type_json);

    const char* sub_type = NULL;
    json_object* sub_type_json = NULL;
    if (!json_object_object_get_ex(msg_json, "sub_type", &sub_type_json)) {
        log_warning("cannot find subtype");
        return -2;
    }
    sub_type = json_object_get_string(sub_type_json);

    json_object* data_json = NULL;
    if (!json_object_object_get_ex(msg_json, "data", &data_json)) {
        log_error("cannot find data");
        return -3;
    }

    if (strcmp(sub_type, "vpn") == 0) {
        return save_vpn_info(id, data_json);
    }
    else {
        log_error("unknown sub_type[%s]", sub_type);
        return -1;
    }
}

int save_notice_event_data(json_object *msg_json) {
    unsigned long long event_id = 0;
    static char sql[SQL_SIZE] = {0};
    const char* type = NULL;
    json_object* type_json = NULL;
    MYSQL_RES *result = NULL;

    if (!json_object_object_get_ex(msg_json, "type", &type_json)) {
        return -1;
    }
    type = json_object_get_string(type_json);

    const char* id = NULL;
    json_object* id_json = NULL;
    if (!json_object_object_get_ex(msg_json, "id", &id_json)) {
        return -1;
    }
    id = json_object_get_string(id_json);

    const char* notice_type = NULL;
    json_object* notice_type_json = NULL;
    if (!json_object_object_get_ex(msg_json, "notice_type", &notice_type_json)) {
        return -1;
    }
    notice_type = json_object_get_string(notice_type_json);

    const char* event_time = NULL;
    json_object* event_time_json = NULL;
    if (!json_object_object_get_ex(msg_json, "timestamp", &event_time_json)) {
        return -1;
    }
    event_time = json_object_get_string(event_time_json);

    /* char server_time[DATETIME_LEN_MAX] = {0}; */
    /* time_t now = time(NULL); */
    /* // strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", gmtime(&now)); */
    /* strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", localtime(&now)); */

    // mysql_autocommit(g_mysql, 0);
    if (0 == strcmp(notice_type, "illegal_process_operate"))
    {
        memset(sql, 0, SQL_SIZE);
        sprintf(sql, "select event_id from notice_event where (status=1 or status=2) and host_id=\"%s\" and event_type=\"%s\" order by event_id desc limit 1", id, notice_type);

        if (mysql_query(g_mysql, sql)) {
            log_error("Failed to query notice event info from mysql[%s].", mysql_error(g_mysql));
            log_error("Sql:[%s]", sql);
            return -1;
        }

        result = mysql_store_result(g_mysql);
        if (NULL != result)
        {
            MYSQL_ROW sql_row;
            if ((sql_row = mysql_fetch_row(result)))
            {
                event_id = (unsigned long long) atoll(sql_row[0]);
                mysql_free_result(result);

                memset(sql, 0, SQL_SIZE);
                sprintf(sql, "update notice_event set update_time=\"%s\" where event_id=%lld", event_time, event_id);
                
                if (mysql_query(g_mysql, sql)) {
                    log_error("Failed to update notice event info from mysql[%s].", mysql_error(g_mysql));
                    log_error("Sql:[%s]", sql);
                    return -1;
                }

                memset(sql, 0, SQL_SIZE);
                sprintf(sql, "delete from notice_operation where event_id=%lld", event_id);
                
                if (mysql_query(g_mysql, sql)) {
                    log_error("Failed to delete notice operations info from mysql[%s].", mysql_error(g_mysql));
                    log_error("Sql:[%s]", sql);
                    return -1;
                }
            }
            else
            {
                mysql_free_result(result);
                memset(sql, 0, SQL_SIZE);
                sprintf(sql, "insert into notice_event(host_id, event_type, create_time, update_time) values (\'%s\', \'%s\', \'%s\', \'%s\')", id, notice_type, event_time, event_time);
                
                if (mysql_query(g_mysql, sql)) {
                    log_error("failed to insert notice_event[%s].", mysql_error(g_mysql));
                    log_error("sql:[%s]", sql);
                    return -1;
                }
                
                event_id = mysql_insert_id(g_mysql);
            }
        }
        else
        {
            log_error("Failed to get mysql query result[%s].", mysql_error(g_mysql));
            return -1;
        }
    }
    else
    {
        memset(sql, 0, SQL_SIZE);
        sprintf(sql, "insert into notice_event(host_id, event_type, create_time, update_time) values (\'%s\', \'%s\', \'%s\', \'%s\')", id, notice_type, event_time, event_time);

        if (mysql_query(g_mysql, sql)) {
            log_error("failed to insert notice_event[%s].", mysql_error(g_mysql));
            log_error("sql:[%s]", sql);
            return -1;
        }

        event_id = mysql_insert_id(g_mysql);
    }

    struct json_object *op_json = NULL;
    struct json_object *operations_json = NULL;
    if (json_object_object_get_ex(msg_json, "operations", &operations_json)) {
        for(int i = 0; i < json_object_array_length(operations_json); i++) {
            op_json = json_object_array_get_idx(operations_json, i);
            struct json_object *op_object_json = NULL;
            struct json_object *op_type_json = NULL;
            const char* op_object = NULL;
            const char* op_type = NULL;
            if (json_object_object_get_ex(op_json, "object", &op_object_json)) {
                op_object = json_object_get_string(op_object_json);
            }
            if (json_object_object_get_ex(op_json, "operate_type", &op_type_json)) {
                op_type = json_object_get_string(op_type_json);
            }
                
            memset(sql, 0, SQL_SIZE);
            sprintf(sql, "insert into notice_operation(event_id, object, operate_type) values (\'%llu\', \'%s\', \'%s\')", event_id, op_object, op_type);
                
            if (mysql_query(g_mysql, sql)) {
                log_warning("failed to insert notice_event[%s].", mysql_error(g_mysql));
                log_warning("sql:[%s]", sql);
                continue;
            }
        }
    }

    /* mysql_commit(g_mysql); */
    /* mysql_rollback(g_mysql); */
    /* mysql_autocommit(g_mysql, 1); */
    return 0;
}

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
        if (id[0] == '\0' || check_device_is_exsited(id) == 0) {
            code = ERROR_CODE_NO_DEVICE_FOUND;
        }
        else {
            json_object* type_json = NULL;
            const char* type = NULL;
            if (json_object_object_get_ex(msg_body_json, "type", &type_json)) {
                type = json_object_get_string(type_json);
            }

            if (id && type && strcmp(type, MESSAGE_TYPE_NOTICE_EVENT) == 0) {
                save_notice_event_data(msg_body_json);
            }
            else if (id && type && strcmp(type, MESSAGE_TYPE_SETTING_MONITOR_EVENT) == 0) {
                save_setting_monitor_event(msg_body_json);
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

    free(identify);
}

static const char *group = "event_srv_group";
static const char* topic = EVENT_SERVICE;
void start_message_handler() {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *topics = NULL;

    init_consumer_rk(&event_srv_consumer_rk, group, g_kafka_brokers);

    /* Create topic */
    topics = rd_kafka_topic_partition_list_new(3);
    rd_kafka_topic_partition_list_add(topics, topic, -1);

    if ((err = rd_kafka_subscribe(event_srv_consumer_rk, topics))) {
        log_error("%% Failed to start consuming topics: %s\n",
                  rd_kafka_err2str(err));
        exit(1);
    }

    while (1) {
        rd_kafka_message_t *rkmessage;
        rkmessage = rd_kafka_consumer_poll(event_srv_consumer_rk, 1000);

        if (rkmessage) {
            msg_consume(rkmessage, NULL);
            rd_kafka_message_destroy(rkmessage);
        }
    }

    /* Stop consuming */
    err = rd_kafka_consumer_close(event_srv_consumer_rk);
    if (err)
        log_error("%% Failed to close consumer: %s\n",
                rd_kafka_err2str(err));
    else
        log_debug("%% Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(topics);

    free_consumer_rk(event_srv_consumer_rk);
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
        "create table IF NOT EXISTS notice_event("
        "event_id int(11) unsigned NOT NULL AUTO_INCREMENT,"
        "host_id varchar(20) NOT NULL,"
        "event_type varchar(64) NOT NULL,"
        "priority int(1) unsigned NOT NULL DEFAULT 1,"
        "create_time datetime NOT NULL,"
        "update_time datetime NOT NULL,"
        "status int(1) unsigned NOT NULL DEFAULT 1,"
        "process_user varchar(255) NOT NULL DEFAULT '',"
        "remark text,"
        "PRIMARY KEY (event_id)"
        ")";

    if (mysql_query(g_mysql, sql))
    {
        log_error("sql:[%s]", sql);
        log_error("failed to create notice_event[%s].", mysql_error(g_mysql));
        return -1;
    }

    sql = 
        "create table IF NOT EXISTS notice_operation("
        "operation_id int(11) unsigned NOT NULL AUTO_INCREMENT,"
        "event_id int(11) unsigned NOT NULL,"
        "object varchar(255) NOT NULL,"
        "operate_type varchar(64) NOT NULL DEFAULT '',"
        "PRIMARY KEY (operation_id)"
        ")";

    if (mysql_query(g_mysql, sql))
    {
        log_error("sql:[%s]", sql);
        log_error("failed to create notice_operation[%s].", mysql_error(g_mysql));
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
  assert(initlog(NULL, "event_service") >= 0);

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
  init_producer_rk(&event_srv_producer_rk, g_kafka_brokers);
  start_message_handler();
  free_producer_rk(event_srv_producer_rk);
  destroy_db_connection();
  releaseLog();
  return 0;
}

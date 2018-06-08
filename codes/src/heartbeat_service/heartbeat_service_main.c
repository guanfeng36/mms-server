#include "heartbeat_service_main.h"

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
static rd_kafka_t *hb_srv_consumer_rk;
static rd_kafka_t *hb_srv_producer_rk;

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

int save_heartbeat_data(json_object* msg_json) {

  const char* id = NULL;
  json_object* id_json = NULL;
  if (!json_object_object_get_ex(msg_json, "id", &id_json)) {
    return -1;
  }
  id = json_object_get_string(id_json);

  char server_time[DATETIME_LEN_MAX] = {0};
  time_t now = time(NULL);
  // strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", gmtime(&now));
  strftime(server_time, DATETIME_LEN_MAX, "%Y-%m-%d %H-%M-%S", localtime(&now));

  #define SQL_SIZE 10240
  char* sql = (char*) malloc(SQL_SIZE);
  memset(sql, 0, SQL_SIZE);

  sprintf(sql, "UPDATE postinfo SET time_stamp_at_server = \'%s\' where id = \'%s\'", server_time, id);

  if (mysql_query(g_mysql, sql)) {
    log_error("failed to update postinfo[%s].", mysql_error(g_mysql));
    log_error("sql:[%s]", sql);
    free(sql);
    return -1;
  }

  free(sql);
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

            if (id && type && strcmp(type, MESSAGE_TYPE_HEARTBEAT) == 0) {
                if (save_heartbeat_data(msg_body_json) < 0) {
                    code = ERROR_CODE_SAVE_HEARTBEAT_ERROR;
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

    const char* service_name = get_service_name(MESSAGE_TYPE_HEARTBEAT_RESPONSE);

    struct json_object *resp_json = json_object_new_object();
    struct json_object *resp_msg_body_json = json_object_new_object();
    json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_HEARTBEAT_RESPONSE));
    json_object_object_add(resp_msg_body_json, "resp_code", json_object_new_int(code));
    json_object_object_add(resp_msg_body_json, "resp_msg", json_object_new_string(get_error_string(code)));

    json_object_object_add(resp_json, "identify", json_object_new_string(identify));
    json_object_object_add(resp_json, "msg_body", resp_msg_body_json);

    const char* msg = json_object_to_json_string(resp_json);
    send_msg(hb_srv_producer_rk, service_name, msg, strlen(msg));
    json_object_put(resp_json);

    free(identify);
}

static const char *group = "hb_srv_group";
static const char* topic = HEARTBEAT_SERVICE;
void start_message_handler() {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *topics;

    init_consumer_rk(&hb_srv_consumer_rk, group, g_kafka_brokers);

    /* Create topic */
    topics = rd_kafka_topic_partition_list_new(3);
    rd_kafka_topic_partition_list_add(topics, topic, -1);

    if ((err = rd_kafka_subscribe(hb_srv_consumer_rk, topics))) {
        log_error("%% Failed to start consuming topics: %s\n",
                  rd_kafka_err2str(err));
        exit(1);
    }
    
    while (1) {
        rd_kafka_message_t *rkmessage;
        rkmessage = rd_kafka_consumer_poll(hb_srv_consumer_rk, 1000);

        if (rkmessage) {
            msg_consume(rkmessage, NULL);
            rd_kafka_message_destroy(rkmessage);
        }
    }

    /* Stop consuming */
    err = rd_kafka_consumer_close(hb_srv_consumer_rk);
    if (err)
        log_error("%% Failed to close consumer: %s\n",
                  rd_kafka_err2str(err));
    else
        log_debug("%% Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(topics);

    free_consumer_rk(hb_srv_consumer_rk);
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
        "create table IF NOT EXISTS postinfo("
        "id varchar(20) NOT NULL,"
        "mac_addr varchar(20) NOT NULL,"
        "ipv4 varchar(20) NOT NULL,"
        "time_stamp datetime NOT NULL,"
        "time_stamp_at_server datetime NOT NULL,"
        "retry_count int(11),"
        "station_num varchar(16),"
        "station_info varchar(32),"
        "address varchar(128),"
        "location varchar(64),"
        "hw_type varchar(32),"
        "machine_model varchar(128),"
        "os_version varchar(64) NOT NULL,"
        "os_type varchar(64) NOT NULL,"
        "heartbeat_interval int(11),"
        "cpu_model_name varchar(48),"
        "kernel varchar(128),"
        "interface longtext,"
        "network longtext,"
        "ps2_device longtext,"
        "usb_device longtext,"
        "com longtext,"
        "graphics longtext,"
        "libs longtext,"
        "software longtext,"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql))
    {
      printf("sql:[%s]", sql);
        log_error("sql:[%s]", sql);
        log_error("failed to insert postinfo[%s].", mysql_error(g_mysql));
	return -1;
    }
    

    /* if((mysql_query(g_mysql, "create database if not exists infoDB")))  */
    /* { */
    /*     log_info("create infoDB failed."); */
    /*     return -1; */
    /* } */

    /* if(mysql_query(g_mysql, "use infoDB"))  */
    /* { */
    /*     log_info("use infoDB failed."); */
    /*     return -1; */
    /* } */
    
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
  assert(initlog(NULL, "heartbeat_service") >= 0);

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
  init_producer_rk(&hb_srv_producer_rk, g_kafka_brokers);
  start_message_handler();
  free_producer_rk(hb_srv_producer_rk);
  destroy_db_connection();
  releaseLog();
  return 0;
}

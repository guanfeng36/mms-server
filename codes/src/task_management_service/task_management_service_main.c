#include "task_management_service_main.h"

#include <time.h>
#include <assert.h>
#include <json-c/json.h>
#include <mysql/mysql.h>

#include "minIni/minIni.h"
#include "common/climit.h"
#include "common/common.h"
#include "common/log.h"
#include "common/message.h"
#include "common/kafka_util.h"

#include "config_task_message_handler.h"

static char g_db_host[IP_ADDRESS_LEN_MAX] = {0};
static char g_db_uname[USERNAME_MAX] = {0};
static char g_db_pwd[PASSWORD_MAX] = {0};

static char g_kafka_brokers[KAFKA_BROKERS_LEN_MAX] = {0};
rd_kafka_t *task_mgt_srv_consumer_rk;
rd_kafka_t *task_mgt_srv_producer_rk;

MYSQL *g_mysql = NULL;

void send_out_task(const char* cpe, struct json_object *task_data_json) {
    log_debug("send task to [%s]", cpe);

    const char* service_name = get_service_name(MESSAGE_TYPE_PUBLISH_TASK);

    struct json_object *resp_json = json_object_new_object();
    struct json_object *resp_msg_body_json = json_object_new_object();
    json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_PUBLISH_TASK));
    json_object_object_add(resp_msg_body_json, "topic", json_object_new_string(cpe));
    json_object_object_add(resp_msg_body_json, "data", json_object_get(task_data_json));

    json_object_object_add(resp_json, "identify", json_object_new_string(""));
    json_object_object_add(resp_json, "msg_body", resp_msg_body_json);

    const char* msg = json_object_to_json_string(resp_json);
    send_msg(task_mgt_srv_producer_rk, service_name, msg, strlen(msg));
    json_object_put(resp_json);
}

// request to publish task to specific device
int distribute_task_for_device(json_object *msg_json) {
    log_debug("start to distribute update task to specific device");

    int ret_val = 0;

    const char* cpe_id = NULL;
    json_object* cpe_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "cpe_id", &cpe_id_json)) {
        cpe_id = json_object_get_string(cpe_id_json);
    }  

    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);
    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    if (cpe_id)  {
        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "SET SESSION group_concat_max_len=102400");
        if (mysql_query(g_mysql, sql)) {
            log_error("failed to set group_concat_max_len[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }

        memset(sql, 0, SQL_SIZE_SMALL);
        snprintf(sql, SQL_SIZE_SMALL, "select task_cpe_map.cpe_id, task_cpe_map.task_id, task_cpe_map.status, task_cpe_map.transaction_id, group_concat(concat(packages.soft_name, if(packages.soft_version is not null, concat('-', packages.soft_version), ''), if(packages.soft_release is not null, concat('-', packages.soft_release), ''), if(packages.soft_arch is not null, concat('.', packages.soft_arch), ''))), task.update_time, task.update_date, concat('/jhcwmp/download/', software_repo.repo_path) from task_cpe_map,task,task_packages,packages,software_repo where (task.status != 'cancelled' or task.status is null) and task_cpe_map.cpe_id = '%s' and task_cpe_map.status in ('tobeupdate', 'toberollback') and task.task_id=task_cpe_map.task_id and task.task_id=task_packages.task_info_id and task_packages.softwarepackageinfo_id=packages.id and packages.repo_id = software_repo.id group by task_cpe_map.task_id,task_cpe_map.cpe_id", cpe_id);
        if (mysql_query(g_mysql, sql)) {
            log_error("failed to query task info[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        result = mysql_store_result(g_mysql);        
        if (!result) {
            log_error("failed to read query result.[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        if (mysql_num_rows(result) > 0 ) {
            while ((row = mysql_fetch_row(result))) {
                struct json_object *task_data_json = json_object_new_object();
                struct json_object *data_json = json_object_new_object(); 

                if (strcmp(row[2], "tobeupdate") == 0) {
                    json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_UPDATE));

                    json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[1])));
                    json_object_object_add(data_json, "files", json_object_new_string(row[4] ? row[4] : ""));
                    json_object_object_add(data_json, "uri", json_object_new_string(row[7] ? row[7] : ""));
                    json_object_object_add(data_json, "update_time", json_object_new_string(row[5]));
                }
                else if (strcmp(row[2], "toberollback") == 0) {
                    json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_ROLLBACK));

                    json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[1])));
                    json_object_object_add(data_json, "transaction_id", json_object_new_int(row[3] ? atoi(row[3]) : -1));
                }
                
                json_object_object_add(task_data_json, "data", data_json);
                send_out_task(row[0], task_data_json);
                json_object_put(task_data_json);
            }
        }
        mysql_free_result(result);
        result = NULL;
    }
    else {
        log_warning("task id and cpe id are invalid");
    }

 end:
    if (sql) free(sql);
    return ret_val;
}

// request to publish update task
int distribute_update_task(json_object *msg_json) {
    log_debug("start to distribute update task");

    int ret_val = 0;
    struct json_object *task_data_json = json_object_new_object();
    json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_UPDATE));

    int task_id = -1;

    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);
    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    json_object* task_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "task_id", &task_id_json)) {
        task_id = json_object_get_int(task_id_json);
    }

    if (task_id >= 0) {
        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "SET SESSION group_concat_max_len=102400");
        if (mysql_query(g_mysql, sql)) {
            log_error("failed to set group_concat_max_len[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        memset(sql, 0, SQL_SIZE_SMALL);
        snprintf(sql, SQL_SIZE_SMALL, "select task.task_id, group_concat(concat(packages.soft_name, if(packages.soft_version is not null, concat('-', packages.soft_version), ''), if(packages.soft_release is not null, concat('-', packages.soft_release), ''), if(packages.soft_arch is not null, concat('.', packages.soft_arch), ''))), task.update_time, concat('/jhcwmp/download/', software_repo.repo_path) from task, task_packages, packages, software_repo where task.task_id = %d and task.task_id=task_packages.task_info_id and task_packages.softwarepackageinfo_id=packages.id and packages.repo_id = software_repo.id and (task.status != 'cancelled' or task.status is null) group by task.task_id", task_id);
        if (mysql_query(g_mysql, sql)) {
            log_error("failed to query task info[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        result = mysql_store_result(g_mysql);
        if (!result) {
            log_error("failed to read query result.[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        if (mysql_num_rows(result) <= 0 ) {
            mysql_free_result(result);
            result = NULL;
            log_debug("task[%d] doesnot need to be published...", task_id);
            goto end;
        }
        struct json_object *data_json = json_object_new_object(); 
        if ((row = mysql_fetch_row(result))) {
            json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[0])));
            json_object_object_add(data_json, "files", json_object_new_string(row[1] ? row[1] : ""));
            json_object_object_add(data_json, "uri", json_object_new_string(row[3] ? row[3] : ""));
            json_object_object_add(data_json, "update_time", json_object_new_string(row[2]));
        }
        json_object_object_add(task_data_json, "data", data_json);
        mysql_free_result(result);
        result = NULL;

        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "select cpe_id from task_cpe_map where task_id = %d and status='tobeupdate'", task_id);
        if (mysql_query(g_mysql, sql))
            {
                log_error("failed to query task cpe info[%s]", mysql_error(g_mysql));
                ret_val = -1;
                goto end;
            }
        result = mysql_store_result(g_mysql);
        if (!result) {
            log_error("failed to read query result.[%s]", mysql_error(g_mysql));
            ret_val = -1;
            goto end;
        }
        if (mysql_num_rows(result) > 0 ) {
            while((row = mysql_fetch_row(result))) {
                send_out_task(row[0], task_data_json);
            }
        }
        else {
            log_warning("no device found in this task");
        }
        mysql_free_result(result);
        result = NULL;
    }
    else {
        log_warning("task id and cpe id are invalid");
    }

 end:
    if (sql) free(sql);
    json_object_put(task_data_json);
    return ret_val;
}

int distribute_rollback_task(json_object *msg_json) {
    int ret_val = 0;
    struct json_object *task_data_json = json_object_new_object();
    json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_ROLLBACK));

    int task_id = -1;
    json_object* task_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "task_id", &task_id_json)) {
        task_id = json_object_get_int(task_id_json);
    }

    const char* cpe_id = NULL;
    struct json_object *cpe_id_json = NULL;
    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    json_object* cpe_ids_json = NULL;
    if (json_object_object_get_ex(msg_json, "cpes", &cpe_ids_json)) {
        for(int i = 0; i < json_object_array_length(cpe_ids_json); i++) {
            cpe_id_json = json_object_array_get_idx(cpe_ids_json, i);
            cpe_id = json_object_get_string(cpe_id_json);

            memset(sql, 0, SQL_SIZE_SMALL);
            sprintf(sql, "select cpe_id, task_cpe_map.task_id, transaction_id from task, task_cpe_map where task.task_id=task_cpe_map.task_id and cpe_id = '%s' and task_cpe_map.task_id = %d and task_cpe_map.status='toberollback' and (task.status != 'cancelled' or task.status is null)", cpe_id, task_id);
            if (mysql_query(g_mysql, sql)) {
                log_error("failed to query task cpe info[%s]", mysql_error(g_mysql));
                ret_val = -1;
                goto end;
            }
            result = mysql_store_result(g_mysql);
            if (!result) {
                log_error("failed to read query result.[%s]", mysql_error(g_mysql));
                ret_val = -1;
                goto end;
            }
            if (mysql_num_rows(result) > 0 ) {
                while((row = mysql_fetch_row(result))) {
                    struct json_object *data_json = json_object_new_object();
                    json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[1])));
                    json_object_object_add(data_json, "transaction_id", json_object_new_int(row[2] ? atoi(row[2]) : -1));
                    json_object_object_add(task_data_json, "data", data_json);
                    send_out_task(row[0], task_data_json);
                }
            }
            mysql_free_result(result);
            result = NULL;
        }
    }

 end:
    if(sql) free(sql);
    json_object_put(task_data_json);
    return ret_val;
}

int distribute_cmd_task(json_object *msg_json) {
    log_debug("start to distribute cmd task");

    int ret_val = 0;
    int task_id = -1;
    json_object* task_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "task_id", &task_id_json)) {
        task_id = json_object_get_int(task_id_json);
    }

    if (task_id >= 0) {
        char* sql = (char*) malloc(SQL_SIZE_SMALL);
        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "select host_id, command from execute_result where id=%d", task_id);

        if (mysql_query(g_mysql, sql) == 0) {
            MYSQL_RES *result = mysql_store_result(g_mysql);
            if (result) {
                if (mysql_num_rows(result) > 0 ) {
                    MYSQL_ROW row = mysql_fetch_row(result);
                    if (row) {
                        struct json_object *task_data_json = json_object_new_object();
                        json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_COMMAND));

                        struct json_object *data_json = json_object_new_object();
                        json_object_object_add(data_json, "task_id", json_object_new_int(task_id));
                        json_object_object_add(data_json, "command", json_object_new_string(row[1]));
                        json_object_object_add(task_data_json, "data", data_json);

                        send_out_task(row[0], task_data_json);
                        json_object_put(task_data_json);
                    }
                }
                mysql_free_result(result);
            }
            else {
                log_error("failed to read query result.");
                ret_val = -1;
            }

        }
        else {
            log_error("failed to query execute_result info");
            ret_val = -1;
        }

        if (sql) free(sql);
    }
    else {
        ret_val = -1;
    }

    return ret_val;
}

int save_update_task_status(json_object *msg_json) {
    if (!msg_json) return -1;

    const char* id = NULL;
    json_object *id_json = NULL;
    if (json_object_object_get_ex(msg_json, "id", &id_json)) {
        id = json_object_get_string(id_json);
    }
    if (!id) return -1;

    int task_id = -1;
    int transaction_id = -1;
    const char* task_status = NULL;

    json_object *data_json = NULL;
    if (json_object_object_get_ex(msg_json, "data", &data_json)) {
        json_object *temp_json = NULL;
        if (json_object_object_get_ex(data_json, "task_id", &temp_json)) {
            task_id = json_object_get_int(temp_json);
        }
        if (json_object_object_get_ex(data_json, "transaction_id", &temp_json)) {
            transaction_id = json_object_get_int(temp_json);
        }
        if (json_object_object_get_ex(data_json, "status", &temp_json)) {
            task_status = json_object_get_string(temp_json);
        }
    }

    if (task_id > 0 && task_status) {
        char* sql = (char*) malloc(SQL_SIZE_SMALL);
        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "update task_cpe_map set status='%s', transaction_id=%d where cpe_id='%s' and task_id=%d", task_status, transaction_id, id, task_id);

        if (mysql_query(g_mysql, sql)) {
            log_error("faild to change task status[%s][%s][%d][%d][%s]", id, task_status, task_id, transaction_id, mysql_error(g_mysql));
            free(sql);
            sql = NULL;
            return -1;
        }
        free(sql);
    }
    return 0;
}

int save_cmd_task_result(json_object *msg_json) {
    if (!msg_json) return -1;

    const char* id = NULL;
    json_object *id_json = NULL;
    if (json_object_object_get_ex(msg_json, "id", &id_json)) {
        id = json_object_get_string(id_json);
    }
    if (!id) return -1;

    int task_id = -1;
    const char* task_result = NULL;
    json_object *data_json = NULL;
    if (json_object_object_get_ex(msg_json, "data", &data_json)) {
        json_object *temp_json = NULL;
        if (json_object_object_get_ex(data_json, "task_id", &temp_json)) {
            task_id = json_object_get_int(temp_json);
        }
        if (json_object_object_get_ex(data_json, "result", &temp_json)) {
            task_result = json_object_get_string(temp_json);
        }
    }

    if (task_id > 0 && task_result) {
        char* sql = (char*) malloc(SQL_SIZE_SMALL);
        memset(sql, 0, SQL_SIZE_SMALL);
        sprintf(sql, "update execute_result set result='%s' where host_id='%s' and id=%d", task_result, id, task_id);

        if (mysql_query(g_mysql, sql)) {
            log_error("faild to save cmd task result[%s][%s][%d]", id, task_result, task_id);
            free(sql);
            sql = NULL;
            return -1;
        }
        free(sql);
    }
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

        json_object* type_json = NULL;
        const char* type = NULL;
        if (json_object_object_get_ex(msg_body_json, "type", &type_json)) {
            type = json_object_get_string(type_json);
        }

        if (type && strcmp(type, MESSAGE_TYPE_DISTRIBUTE_UPDATE_TASK) == 0) {
            if (distribute_update_task(msg_body_json) < 0) {
                code = ERROR_CODE_TASK_DISTRIBUTE_ERROR;
            }
        }
        else if (type && strcmp(type, MESSAGE_TYPE_DISTRIBUTE_ROLLBACK_TASK) == 0) {
            if (distribute_rollback_task(msg_body_json) < 0) {
                code = ERROR_CODE_TASK_DISTRIBUTE_ERROR;
            }
        }
        else if (type && strcmp(type, MESSAGE_TYPE_DISTRIBUTE_CMD_TASK) == 0) {
            if (distribute_cmd_task(msg_body_json) < 0) {
                code = ERROR_CODE_TASK_DISTRIBUTE_ERROR;
            }  
        }
        else if (type && strcmp(type, MESSAGE_TYPE_UPDATE_TASK_STATUS) == 0) {
            if (save_update_task_status(msg_body_json) < 0) {
                code = ERROR_CODE_SAVE_UPDATE_TASK_STATUS_ERROR;
            }

            const char* service_name = get_service_name(MESSAGE_TYPE_UPDATE_TASK_STATUS_RESPONSE);

            struct json_object *resp_json = json_object_new_object();
            struct json_object *resp_msg_body_json = json_object_new_object();
            json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_UPDATE_TASK_STATUS_RESPONSE));
            json_object_object_add(resp_msg_body_json, "resp_code", json_object_new_int(code));
            json_object_object_add(resp_msg_body_json, "resp_msg", json_object_new_string(get_error_string(code)));
            
            json_object_object_add(resp_json, "identify", json_object_new_string(identify));
            json_object_object_add(resp_json, "msg_body", resp_msg_body_json);
            const char* msg = json_object_to_json_string(resp_json);
            send_msg(task_mgt_srv_producer_rk, service_name, msg, strlen(msg));
            json_object_put(resp_json);
        }
        else if (type && strcmp(type, MESSAGE_TYPE_CMD_TASK_RESULT) == 0) {
            if (save_cmd_task_result(msg_body_json) < 0) {
                code = ERROR_CODE_SAVE_CMD_TASK_RESULT_ERROR;
            }

            const char* service_name = get_service_name(MESSAGE_TYPE_CMD_TASK_RESULT_RESPONSE);


            struct json_object *resp_json = json_object_new_object();
            struct json_object *resp_msg_body_json = json_object_new_object();
            json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_CMD_TASK_RESULT_RESPONSE));
            json_object_object_add(resp_msg_body_json, "resp_code", json_object_new_int(code));
            json_object_object_add(resp_msg_body_json, "resp_msg", json_object_new_string(get_error_string(code)));
            
            json_object_object_add(resp_json, "identify", json_object_new_string(identify));
            json_object_object_add(resp_json, "msg_body", resp_msg_body_json);
            const char* msg = json_object_to_json_string(resp_json);
            send_msg(task_mgt_srv_producer_rk, service_name, msg, strlen(msg));
            json_object_put(resp_json);
        }
        else if (type && strcmp(type, MESSAGE_TYPE_NOTIFY_DEVICE_CONNECTED) == 0) {
            log_debug("receive message[%s]", type);
            if (distribute_task_for_device(msg_body_json) < 0) {
                log_warning("failed to send update task for device[%s]", id);
            }
            if (distribute_config_task_for_device(msg_body_json) < 0) {
                log_warning("failed to send config task for device[%s]", id);
            }
        }
        else if (type && strcmp(type, MESSAGE_TYPE_DISTRIBUTE_CONFIG_TASK) == 0) {
            log_debug("receive message[%s]", type);
            if (distribute_config_task(msg_body_json) < 0) {
                code = ERROR_CODE_TASK_DISTRIBUTE_ERROR;
            }
        }
        else if (type && strcmp(type, MESSAGE_TYPE_CONFIG_TASK_STATUS) == 0) {
            log_debug("receive message[%s]", type);
            code = save_config_task_status(msg_body_json);

            const char* service_name = get_service_name(MESSAGE_TYPE_CONFIG_TASK_STATUS_RESPONSE);

            struct json_object *resp_json = json_object_new_object();
            struct json_object *resp_msg_body_json = json_object_new_object();
            json_object_object_add(resp_msg_body_json, "type", json_object_new_string(MESSAGE_TYPE_CONFIG_TASK_STATUS_RESPONSE));
            json_object_object_add(resp_msg_body_json, "resp_code", json_object_new_int(code));
            json_object_object_add(resp_msg_body_json, "resp_msg", json_object_new_string(get_error_string(code)));
            
            json_object_object_add(resp_json, "identify", json_object_new_string(identify));
            json_object_object_add(resp_json, "msg_body", resp_msg_body_json);
            const char* msg = json_object_to_json_string(resp_json);
            send_msg(task_mgt_srv_producer_rk, service_name, msg, strlen(msg));
            json_object_put(resp_json);
        }
        else {
            log_error("failed to match request type[%s]", type);
            code = ERROR_CODE_UNKNOWN_REQUEST;
        }
    }
    else {
        code = ERROR_CODE_FORMAT_ERROR;
    }

    json_object_put(msg_json);
    free(identify);
}

static const char *group = "task_mgt_srv_group";
static const char* topic = TASK_MANAGEMENT_SERVICE;
void start_message_handler() {
    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *topics = NULL;

    init_consumer_rk(&task_mgt_srv_consumer_rk, group, g_kafka_brokers);

    /* Create topic */
    topics = rd_kafka_topic_partition_list_new(3);
    rd_kafka_topic_partition_list_add(topics, topic, -1);

    if ((err = rd_kafka_subscribe(task_mgt_srv_consumer_rk, topics))) {
        log_error("%% Failed to start consuming topics: %s\n",
                  rd_kafka_err2str(err));
        exit(1);
    }

    while (1) {
        rd_kafka_message_t *rkmessage;

        rkmessage = rd_kafka_consumer_poll(task_mgt_srv_consumer_rk, 1000);

        if (rkmessage) {
            msg_consume(rkmessage, NULL);
            rd_kafka_message_destroy(rkmessage);
        }
    }

    /* Stop consuming */
    err = rd_kafka_consumer_close(task_mgt_srv_consumer_rk);
    if (err)
        log_error("%% Failed to close consumer: %s\n",
                  rd_kafka_err2str(err));
    else
        log_debug("%% Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(topics);

    free_consumer_rk(task_mgt_srv_consumer_rk);
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

    char* sql_create_task = 
        "create table IF NOT EXISTS task("
        "task_id int(11) NOT NULL AUTO_INCREMENT,"
        "update_time varchar(32) DEFAULT NULL,"
        "update_date date DEFAULT NULL,"
        "create_time datetime DEFAULT NULL,"
        "create_by_id int(11) DEFAULT NULL,"
        "last_edit_time datetime DEFAULT NULL,"
        "last_edit_by_id int(11) DEFAULT NULL,"
        "status varchar(32) DEFAULT NULL,"
        "PRIMARY KEY (task_id)"
        ")";

    if (mysql_query(g_mysql, sql_create_task)) {
        log_error("sql:[%s]", sql_create_task);
        log_error("failed to create task table[%s].", mysql_error(g_mysql));
        return -1;
    }

    char* sql_create_task_packages = 
        "create table IF NOT EXISTS task_packages("
        "id int(11) NOT NULL AUTO_INCREMENT,"
        "task_info_id int(11) NOT NULL,"
        "softwarepackageinfo_id int(11) NOT NULL,"
        "foreign key(softwarepackageinfo_id) references packages(id),"
        "foreign key(task_info_id) references task(task_id),"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql_create_task_packages)) {
        log_error("sql:[%s]", sql_create_task_packages);
        log_error("failed to create task cpe map table[%s].", mysql_error(g_mysql));
        return -1;
    }

    char* sql_create_task_cpe_map = 
        "create table IF NOT EXISTS task_cpe_map("
        "id int(11) NOT NULL AUTO_INCREMENT,"
        "cpe_id varchar(20) NOT NULL,"
        "task_id int(11) NOT NULL,"
        "transaction_id int(11) DEFAULT NULL,"
        "status varchar(32) DEFAULT NULL,"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql_create_task_cpe_map))
        {
            log_error("sql:[%s]", sql_create_task_cpe_map);
            log_error("failed to create task cpe map table[%s].", mysql_error(g_mysql));
            return -1;
        }

    char* sql_create_software_repo = 
        "create table IF NOT EXISTS software_repo("
        "id int(11) NOT NULL AUTO_INCREMENT,"
        "name varchar(128) NOT NULL,"
        "remark longtext,"
        "repo_path varchar(128) NOT NULL,"
        "create_time datetime DEFAULT NULL,"
        "create_by_id int(11) DEFAULT NULL,"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql_create_software_repo))
        {
            log_error("sql:[%s]", sql_create_software_repo);
            log_error("failed to create software_repo table[%s].", mysql_error(g_mysql));
            return -1;
        }

    char* sql_create_packages = 
        "create table IF NOT EXISTS packages("
        "id int(11) NOT NULL AUTO_INCREMENT,"
        "name varchar(512) NOT NULL,"
        "soft_name varchar(128) DEFAULT NULL,"
        "soft_version varchar(128) DEFAULT NULL,"
        "soft_release varchar(128) DEFAULT NULL,"
        "soft_vendor varchar(128) DEFAULT NULL,"
        "soft_arch varchar(128) DEFAULT NULL,"
        "repo_id int(11) DEFAULT NULL,"
        "upload_time datetime DEFAULT NULL,"
        "upload_by_id int(11) DEFAULT NULL,"
        "foreign key(repo_id) references software_repo(id),"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql_create_packages))
        {
            log_error("sql:[%s]", sql_create_packages);
            log_error("failed to create packages table[%s].", mysql_error(g_mysql));
            return -1;
        }

    char* sql_create_package_history = 
        "create table IF NOT EXISTS package_history("
        "id int(11) NOT NULL AUTO_INCREMENT,"
        "name varchar(512) NOT NULL,"
        "operation varchar(32) DEFAULT NULL,"
        "operate_time datetime DEFAULT NULL,"
        "operate_by_id int(11) DEFAULT NULL,"
        "PRIMARY KEY (id)"
        ")";

    if (mysql_query(g_mysql, sql_create_package_history))
        {
            log_error("sql:[%s]", sql_create_package_history);
            log_error("failed to create package_history table[%s].", mysql_error(g_mysql));
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
    assert(initlog(NULL, "task_management_service") >= 0);

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

    if (ini_gets("kafka", "brokers", "127.0.0.1:9092", g_kafka_brokers, sizeof(g_kafka_brokers), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get kafka brokers!");
        strncpy(g_kafka_brokers, "127.0.0.1:9092", sizeof(g_kafka_brokers));
    }

    assert(init_db_connection(g_db_host, g_db_uname, g_db_pwd) >= 0);
    init_producer_rk(&task_mgt_srv_producer_rk, g_kafka_brokers);
    start_message_handler();
    free_producer_rk(task_mgt_srv_producer_rk);
    destroy_db_connection();
    releaseLog();
    return 0;
}

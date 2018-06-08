/*
 * file config_task_message_handler.c
 */

#include <assert.h>
#include <mysql/mysql.h>

#include "common/climit.h"
#include "common/message.h"
#include "common/log.h"
#include "common/kafka_util.h"
#include "config_task_message_handler.h"

extern MYSQL *g_mysql;
extern rd_kafka_t *task_mgt_srv_consumer_rk;
extern rd_kafka_t *task_mgt_srv_producer_rk;

static void send_out_task(const char* cpe, struct json_object *task_data_json) {
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

int distribute_config_task(json_object *msg_json) {
    log_debug("start to distribute config task");

    int task_id = -1;
    json_object* task_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "task_id", &task_id_json)) {
        task_id = json_object_get_int(task_id_json);
    }

    if (task_id < 0) {
        log_warning("task id is invalid");
        return -1;
    }

    int ret_val = 0;
    struct json_object *task_data_json = json_object_new_object();
    json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_CONFIG));

    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);
    snprintf(sql, SQL_SIZE_SMALL, "select t.id, c.filename, c.svn_path, c.svn_revision, c.config_abs_path, c.md5sum, c.ftype, t.post_script from configure_task t, configuration_info c where t.configuration_id = c.id and t.id = %d", task_id);
    if (mysql_query(g_mysql, sql)) {
        log_error("failed to query config task info[%s]", mysql_error(g_mysql));
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
        log_debug("no inprocessing task found for config task[%d]...", task_id);
        goto end;
    }

    struct json_object *data_json = json_object_new_object();
    if ((row = mysql_fetch_row(result))) {
        json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[0])));
        json_object_object_add(data_json, "fname", json_object_new_string(row[1] ? row[1] : ""));
        json_object_object_add(data_json, "svn_uri", json_object_new_string(row[2] ? row[2] : ""));
        json_object_object_add(data_json, "svn_revision", json_object_new_int(atoi(row[3])));
        json_object_object_add(data_json, "fpath", json_object_new_string(row[4] ? row[4] : ""));
        json_object_object_add(data_json, "md5sum", json_object_new_string(row[5] ? row[5] : ""));
        // json_object_object_add(data_json, "ftype", json_object_new_int(atoi(row[6])));
        json_object_object_add(data_json, "ftype", json_object_new_string("whole"));
        json_object_object_add(data_json, "post_script", json_object_new_string(row[7] ? row[7] : ""));
    }
    json_object_object_add(task_data_json, "data", data_json);
    mysql_free_result(result);
    result = NULL;

    memset(sql, 0, SQL_SIZE_SMALL);
    sprintf(sql, "select device_id from configure_task_devices where task_id = %d and status in (1,2)", task_id);
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
            send_out_task(row[0], task_data_json);
        }
    }

    mysql_free_result(result);
    result = NULL;
 
 end:
    if (sql) free(sql);
    json_object_put(task_data_json);
    return ret_val;
}

typedef enum {
    TASK_STATUS_UNKNOWN = -1,
    TASK_STATUS_TOBEUPDATE = 1,
    TASK_STATUS_UPDATING,
    TASK_STATUS_UPDATED,
    TASK_STATUS_FAILED,
    TASK_STATUS_CANCELLED,
} TaskStatus;

static int get_task_status(const char* status_str) {
    if (status_str == NULL || status_str[0] == '\0') return TASK_STATUS_UNKNOWN;

    if (strcmp(status_str, "updating") == 0) {
        return TASK_STATUS_UPDATING;
    }
    if (strcmp(status_str, "updated") == 0) {
        return TASK_STATUS_UPDATED;
    }
    if (strcmp(status_str, "failed") == 0) {
        return TASK_STATUS_FAILED;
    }
    if (strcmp(status_str, "cancelled") == 0) {
        return TASK_STATUS_CANCELLED;
    }

    return TASK_STATUS_UNKNOWN;
}

int save_config_task_status(json_object *msg_json) {
    assert(msg_json != NULL);

    const char* id = NULL;
    int task_id = -1;
    int task_status = TASK_STATUS_UNKNOWN;
    const char* msg = NULL;

    json_object *id_json = NULL;
    if (json_object_object_get_ex(msg_json, "id", &id_json)) {
        id = json_object_get_string(id_json);
    }

    json_object *data_json = NULL;
    if (json_object_object_get_ex(msg_json, "data", &data_json)) {
        json_object *temp_json = NULL;
        if (json_object_object_get_ex(data_json, "task_id", &temp_json)) {
            task_id = json_object_get_int(temp_json);
        }
        if (json_object_object_get_ex(data_json, "status", &temp_json)) {
            task_status = get_task_status(json_object_get_string(temp_json));
        }
        if (json_object_object_get_ex(data_json, "message", &temp_json)) {
            msg = json_object_get_string(temp_json);
        }
    }

    if (!id) {
        return ERROR_CODE_NO_DEVICE_FOUND;
    }
    if (task_id <= 0) {
        return ERROR_CODE_REQUEST_INFO_INVALID;
    }

    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);
    sprintf(sql, "update configure_task_devices set status=%d, message='%s' where device_id='%s' and task_id=%d", task_status, msg, id, task_id);
    
    if (mysql_query(g_mysql, sql)) {
        log_error("faild to change conifg task status[%s][%d][%d][%s]", id, task_status, task_id, mysql_error(g_mysql));
        free(sql);
        sql = NULL;
        return ERROR_CODE_SAVE_CONFIG_TASK_RESULT_ERROR;
    }
    free(sql);

    return ERROR_CODE_OK;
}

int distribute_config_task_for_device(json_object *msg_json) {
    log_debug("start to distribute config task to specific device");

    const char* cpe_id = NULL;
    json_object* cpe_id_json = NULL;
    if (json_object_object_get_ex(msg_json, "cpe_id", &cpe_id_json)) {
        cpe_id = json_object_get_string(cpe_id_json);
    }
    if (cpe_id == NULL || cpe_id[0] == '\0') {
        return -1;
    }

    int ret_val = 0;

    char* sql = (char*) malloc(SQL_SIZE_SMALL);
    memset(sql, 0, SQL_SIZE_SMALL);
    MYSQL_ROW row;
    MYSQL_RES *result = NULL;

    snprintf(sql, SQL_SIZE_SMALL, "select t.id, c.filename, c.svn_path, c.svn_revision, c.config_abs_path, c.md5sum, c.ftype, t.post_script from configure_task t, configuration_info c, configure_task_devices m where m.device_id = '%s' and m.task_id = t.id and t.configuration_id = c.id and m.status in (%d, %d) order by t.id", cpe_id,TASK_STATUS_TOBEUPDATE,TASK_STATUS_UPDATING);

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
            json_object_object_add(task_data_json, "type", json_object_new_string(TASK_TYPE_CONFIG));
            json_object_object_add(data_json, "task_id", json_object_new_int(atoi(row[0])));
            json_object_object_add(data_json, "fname", json_object_new_string(row[1] ? row[1] : ""));
            json_object_object_add(data_json, "svn_uri", json_object_new_string(row[2] ? row[2] : ""));
            json_object_object_add(data_json, "svn_revision", json_object_new_int(atoi(row[3])));
            json_object_object_add(data_json, "fpath", json_object_new_string(row[4] ? row[4] : ""));
            json_object_object_add(data_json, "md5sum", json_object_new_string(row[5] ? row[5] : ""));
            // json_object_object_add(data_json, "ftype", json_object_new_int(atoi(row[6])));
            json_object_object_add(data_json, "ftype", json_object_new_string("whole"));
            json_object_object_add(data_json, "post_script", json_object_new_string(row[7] ? row[7] : ""));

            json_object_object_add(task_data_json, "data", data_json);
            send_out_task(cpe_id, task_data_json);
            json_object_put(task_data_json);
        }
    }

 end:
    if (result) {
        mysql_free_result(result);
        result = NULL;
    }

    if (sql) {
        free(sql);
        sql = NULL;
    }
    return ret_val;
}

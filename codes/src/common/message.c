#include "common/message.h"

typedef struct {
  error_code_t code;
  char* err_string;
} error_t;

static error_t errors[] = {
  {ERROR_CODE_OK, ""},
  {ERROR_CODE_REQ_EMPTY, "request empty"},
  {ERROR_CODE_FORMAT_ERROR, "format error"},
  {ERROR_CODE_REQUEST_INFO_INVALID, "invalid request data"},
  {ERROR_CODE_DATA_TOO_LARGE, "data too large"},
  {ERROR_CODE_UNAUTHORIZED_REQUEST, "unauthorized request"},
  {ERROR_CODE_UNKNOWN_REQUEST, "unknown request type"},
  {ERROR_CODE_SAVE_REGISTER_ERROR, "register data error"},
  {ERROR_CODE_SAVE_INFO_ERROR, "inform data error"},
  {ERROR_CODE_SAVE_HEARTBEAT_ERROR, "heartbeat data error"},
  {ERROR_CODE_TASK_DISTRIBUTE_ERROR, "distribute task error"},
  {ERROR_CODE_SAVE_UPDATE_TASK_STATUS_ERROR, "save update task status error"},
  {ERROR_CODE_SAVE_CMD_TASK_RESULT_ERROR, "save cmd task result error"},
  {ERROR_CODE_SAVE_CONFIG_TASK_RESULT_ERROR, "save config task result error"},
  {ERROR_CODE_NO_DEVICE_FOUND, "no device found"}
};

const char* get_error_string(error_code_t err) {

  for (int i = 0; i < sizeof(errors) / sizeof(error_t); i++) {
    if (errors[i].code == err) return errors[i].err_string;
  }
  return NULL;
}

const char* get_service_name(const char* req_type) {
  typedef struct {
    const char* type;
    const char* service;;
  } service_t;

  static service_t service_request_map[] = {
    {MESSAGE_TYPE_REGISTER, REGISTER_SERVICE},
    {MESSAGE_TYPE_INFORM, INFORM_SERVICE},
    {MESSAGE_TYPE_HEARTBEAT, HEARTBEAT_SERVICE},
    {MESSAGE_TYPE_UPDATE_TASK_STATUS, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_CONFIG_TASK_STATUS, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_CMD_TASK_RESULT, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_NOTICE_EVENT, EVENT_SERVICE},
    {MESSAGE_TYPE_SETTING_MONITOR_EVENT, EVENT_SERVICE},
    {MESSAGE_TYPE_INFLUXDB_METRICS,INFLUXDB_METRICS_SERVICE},

    {MESSAGE_TYPE_REGISTER_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_INFORM_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_HEARTBEAT_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_UPDATE_TASK_STATUS_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_CONFIG_TASK_STATUS_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_CMD_TASK_RESULT_RESPONSE, RESPONSE_SENDER_SERVICE},
    {MESSAGE_TYPE_INFLUXDB_METRICS_RESPONSE, RESPONSE_SENDER_SERVICE},

    {MESSAGE_TYPE_PUBLISH_TASK, TASK_PUBLISH_SERVICE},

    {MESSAGE_TYPE_DISTRIBUTE_UPDATE_TASK, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_DISTRIBUTE_ROLLBACK_TASK, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_DISTRIBUTE_CMD_TASK, TASK_MANAGEMENT_SERVICE},
    {MESSAGE_TYPE_DISTRIBUTE_CONFIG_TASK, TASK_MANAGEMENT_SERVICE},

    {MESSAGE_TYPE_NOTIFY_DEVICE_CONNECTED, TASK_MANAGEMENT_SERVICE},
  };

  for (int i = 0; i < sizeof(service_request_map) / sizeof(service_t); i++) {
    if (!strcmp(service_request_map[i].type, req_type)) return service_request_map[i].service;
  }

  return NULL;
}

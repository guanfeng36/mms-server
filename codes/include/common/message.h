#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string.h>

//  error code and error message for response
typedef enum {
  ERROR_CODE_OK = 0,
  ERROR_CODE_REQ_EMPTY,
  ERROR_CODE_FORMAT_ERROR,
  ERROR_CODE_REQUEST_INFO_INVALID,
  ERROR_CODE_DATA_TOO_LARGE,
  ERROR_CODE_UNAUTHORIZED_REQUEST,
  ERROR_CODE_UNKNOWN_REQUEST,

  // for register service
  ERROR_CODE_SAVE_REGISTER_ERROR,
  // for inform service
  ERROR_CODE_SAVE_INFO_ERROR,
  // for heartbeat
  ERROR_CODE_SAVE_HEARTBEAT_ERROR,
  // for task management
  ERROR_CODE_TASK_DISTRIBUTE_ERROR,
  // for update task status
  ERROR_CODE_SAVE_UPDATE_TASK_STATUS_ERROR,
  // for cmd task result
  ERROR_CODE_SAVE_CMD_TASK_RESULT_ERROR,
  // for config task result
  ERROR_CODE_SAVE_CONFIG_TASK_RESULT_ERROR,
  // for metrics service 
  ERROR_CODE_SAVE_INFLUXDB_METRICS_ERROR,

  ERROR_CODE_NO_DEVICE_FOUND = 400
} error_code_t;

const char* get_error_string(error_code_t err);

// define message type
#define MESSAGE_TYPE_BAD_REQUEST "bad_request"
#define MESSAGE_TYPE_REGISTER "register"
#define MESSAGE_TYPE_REGISTER_RESPONSE "register_resp"
#define MESSAGE_TYPE_INFORM "inform"
#define MESSAGE_TYPE_INFORM_RESPONSE "inform_resp"
#define MESSAGE_TYPE_HEARTBEAT "heartbeat"
#define MESSAGE_TYPE_HEARTBEAT_RESPONSE "heartbeat_resp"
#define MESSAGE_TYPE_UPDATE_TASK_STATUS "update_task_status"
#define MESSAGE_TYPE_UPDATE_TASK_STATUS_RESPONSE "update_task_status_resp"
#define MESSAGE_TYPE_NOTICE_EVENT "notice_event"
#define MESSAGE_TYPE_SETTING_MONITOR_EVENT "setting_monitor_event"
#define MESSAGE_TYPE_INFLUXDB_METRICS "metrics"
#define MESSAGE_TYPE_INFLUXDB_METRICS_RESPONSE "metrics_resp"

#define MESSAGE_TYPE_CMD_TASK_RESULT "cmd_task_result"
#define MESSAGE_TYPE_CMD_TASK_RESULT_RESPONSE "cmd_task_result_resp"
#define MESSAGE_TYPE_BLACK_PROCESS_MONITOR "black_process_monitor"

#define MESSAGE_TYPE_CONFIG_TASK_STATUS "config_task_status"
#define MESSAGE_TYPE_CONFIG_TASK_STATUS_RESPONSE "config_task_status_resp"

#define MESSAGE_TYPE_DISTRIBUTE_UPDATE_TASK "distribute_update_task"
#define MESSAGE_TYPE_DISTRIBUTE_ROLLBACK_TASK "distribute_rollback_task"
#define MESSAGE_TYPE_DISTRIBUTE_CMD_TASK "distribute_cmd_task"
#define MESSAGE_TYPE_DISTRIBUTE_CONFIG_TASK "distribute_config_task"

#define MESSAGE_TYPE_PUBLISH_TASK "publish_task"

#define MESSAGE_TYPE_NOTIFY_DEVICE_CONNECTED "notify_device_connected"

#define MESSAGE_TYPE_ABNORMAL_PROCESS "black_process_monitor" 
#define MESSAGE_TYPE_USB_DEVICE_CHANGED "usb_monitor"

// define task type
#define TASK_TYPE_UPDATE "task_update"
#define TASK_TYPE_ROLLBACK "task_rollback"
#define TASK_TYPE_COMMAND "task_command"
#define TASK_TYPE_CONFIG "task_config"


// define service name
#define RESPONSE_SENDER_SERVICE "response_sender_service"
#define REGISTER_SERVICE "register_service"
#define INFORM_SERVICE "inform_service"
#define HEARTBEAT_SERVICE "hb_service"
#define EVENT_SERVICE "event_service"
#define TASK_PUBLISH_SERVICE "task_publish_service"
#define TASK_MANAGEMENT_SERVICE "task_management_service"
#define INFLUXDB_METRICS_SERVICE "influxdb_metrics_service"

const char* get_service_name(const char* req_type);

#endif  // __MESSAGE_H_

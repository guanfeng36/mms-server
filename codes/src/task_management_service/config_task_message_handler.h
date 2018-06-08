
#ifndef __CONFIG_TASK_MESSAGE_HANDLER_H__
#define __CONFIG_TASK_MESSAGE_HANDLER_H__

#include <json-c/json.h>

int distribute_config_task(json_object *msg_json);

int save_config_task_status(json_object *msg_json);

int distribute_config_task_for_device(json_object *msg_json);

#endif // __CONFIG_TASK_MESSAGE_HANDLER_H__

/*
 * common.h
 * Copyright (C) 2016
 */

#ifndef __COMMON_H_
#define __COMMON_H_

#define AUTH_ENCODE_KEY "qWuYhJBMhGbFhLpIhuHgBNMk"

#define DEFAULT_CONFIG_FILE "/etc/jh_mms_conf" 

#define DEFAULT_AMQP_SERVER_HOST "127.0.0.1"
#define DEFAULT_AMQP_SERVER_PORT 5672

#define DEFAULT_DATABASE_HOST "127.0.0.1"
#define DEFAULT_DATABASE_USERNAME "root"
#define DEFAULT_DATABASE_PASSWORD "abc123"

#define DEFAULT_LISTEN_ENDPOINT "tcp://*:9000"
#define DEFAULT_WORKER_NUM 4
#define DEFAULT_PUBLISH_ENDPOINT "tcp://*:9001"

void add_sigsegv_handler();
int get_process_name(char* name, int max_len);

#endif // __COMMON_H_

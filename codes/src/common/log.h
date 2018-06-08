/*********************************************
 * Copyright(C) 2016- JH Co.,Ltd.
 * log.h
 *********************************************/


#ifndef _LOG_H_
#define _LOG_H_
#include <zlog.h>

int initlog(const char* conf_name, const char* app_name);
void releaseLog();

#define log_fatal(...) dzlog_fatal(__VA_ARGS__)
#define log_error(...) dzlog_error(__VA_ARGS__)
#define log_warning(...) dzlog_warn(__VA_ARGS__)
#define log_notice(...) dzlog_notice(__VA_ARGS__)
#define log_info(...) dzlog_info(__VA_ARGS__)
#define log_debug(...) dzlog_debug(__VA_ARGS__)

#endif

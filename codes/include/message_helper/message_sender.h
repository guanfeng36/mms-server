#ifndef __MESSAGE_SENDER_H__
#define __MESSAGE_SENDER_H__

#include <stdlib.h>

int send_str_without_reply(const char* endpoint, const char* msg);
// bool send_bin_data_without_reply();

int send_str_with_reply(const char* endpoint, const char* msg, char** response);
// bool send_bin_data_with_reply();

#endif  // __MESSAGE_SENDER_H__

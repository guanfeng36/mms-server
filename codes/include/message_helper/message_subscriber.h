#ifndef __MESSAGE_SUBSCRIBER_H_
#define __MESSAGE_SUBSCRIBER_H_

typedef int (*sub_cb_func)(const char* msg, int length);

int create_subscriber_and_wait_msg(const char* endpoint, const char* filter, sub_cb_func cb_func);

#endif  // __MESSAGE_SUBSCRIBER_H_

#ifndef __ASYNC_SERVER_H_
#define __ASYNC_SERVER_H_

typedef struct async_server_st async_server_t;

// typedef int (*message_handle_func)(async_server_t* server, void* identity, void* msg, int length);
typedef int (*message_handle_func)(void* identity, int identity_len, void* msg, int length);

int init_server(async_server_t** pp_server, const char* endpoint, int worker_number);
void destroy_server(async_server_t* p_server);

int start_server(async_server_t* p_server);
int stop_servert(async_server_t* p_server);

int set_message_handler(async_server_t* p_server, message_handle_func msg_handler);
int send_response(async_server_t* server, void* identity, int identity_len, void* msg, int length);

#endif  // __ASYNC_SERVER_H_

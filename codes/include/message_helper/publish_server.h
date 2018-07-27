#ifndef __PUBLISH_SERVER_H_
#define __PUBLISH_SERVER_H_

typedef struct publish_server_st publish_server_t;

int init_pub_server(publish_server_t** pp_server, const char* endpoint);
void destroy_pub_server(publish_server_t* p_server);

int start_pub_server(publish_server_t* p_server);
int stop_pub_servert(publish_server_t* p_server);

int publish_message(publish_server_t* p_server, const char* topic, const char* msg);

#endif  // __PUBLISH_SERVER_H_

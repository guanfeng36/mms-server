#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "czmq.h"
#include "message_helper/publish_server.h"

struct publish_server_st {
    char* publish_endpoint;
    zsock_t* pub_sock;
    pthread_t pid;
    int state;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

int init_pub_server(publish_server_t** pp_server, const char* endpoint) {
  assert(pp_server && endpoint);

  publish_server_t* temp = (publish_server_t*) malloc(sizeof(publish_server_t));
  memset(temp, 0, sizeof(publish_server_t));

  temp->publish_endpoint = strdup(endpoint);
  temp->state = 0;

  pthread_mutex_init(&(temp->mutex), NULL);
  pthread_cond_init(&(temp->cond), NULL);

  *pp_server = temp;
  return 0;
}

void destroy_pub_server(publish_server_t* p_server) {
  assert(p_server);

  if (p_server->state) stop_pub_servert(p_server);
  pthread_mutex_destroy(&(p_server->mutex));
  pthread_cond_destroy(&(p_server->cond));
  free(p_server->publish_endpoint);
  free(p_server);
}

void* hb_fn(void* args) {
    assert(args != NULL);
    publish_server_t* server = (publish_server_t*) args;
    struct timeval now;
    struct timespec outtime;

    while (server->state) {
        publish_message(server, "__hb__", "");

        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + 30;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&(server->cond), &(server->mutex), &outtime);
    }
    pthread_mutex_unlock(&(server->mutex));
    return 0;
}

int start_pub_server(publish_server_t* p_server) {
  assert(p_server);
  if (p_server->state) return 1;

  p_server->pub_sock = zsock_new_pub(p_server->publish_endpoint);

  if (pthread_create(&(p_server->pid), NULL, hb_fn, p_server) != 0) {
      zsock_destroy(&(p_server->pub_sock));
      p_server->pub_sock = NULL;
      return -1;
  }
  p_server->state = 1;
  return 0;
}

int stop_pub_servert(publish_server_t* p_server) {
    assert(p_server); 
    if (! p_server->state) return 1;
    p_server->state = 0;
    pthread_mutex_lock(&(p_server->mutex));
    pthread_cond_signal(&(p_server->cond));
    pthread_mutex_unlock(&(p_server->mutex));

    pthread_join(p_server->pid, NULL);
    zsock_destroy(&(p_server->pub_sock));
    return 0;
}

int publish_message(publish_server_t* p_server, const char* topic, const char* msg_str) {
  
  zmsg_t *msg = zmsg_new ();
  zmsg_addstr(msg, topic);
  zmsg_addstr(msg, msg_str);
  zmsg_send(&msg, p_server->pub_sock);
  return 0;
}

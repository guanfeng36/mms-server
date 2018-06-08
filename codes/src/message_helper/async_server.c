#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "czmq.h"
#include "message_helper/async_server.h"

#define DEFAULT_WORKER_NUMBER 1

struct async_server_st{
  // listen endpoint string
  char* listen_endpoint;
  char* transfer_endpoint;
  char* result_endpoint;

  // thread which listen for request
  zactor_t* request_server;

  int worker_num;
  zactor_t** workers;

  message_handle_func msg_handle_func;
};

void worker_func(zsock_t *pipe, void *args) {
  zsock_signal (pipe, 0);

  async_server_t* p_server = (async_server_t*) args;
  assert(p_server);

  zsock_t *req_recv_sock = zsock_new_dealer(p_server->transfer_endpoint);
  zsock_t *res_recv_sock = zsock_new_pull(p_server->result_endpoint);

  zpoller_t *poller = zpoller_new (pipe, req_recv_sock, res_recv_sock, NULL);
  while (true) {
    zsock_t* sock = zpoller_wait (poller, -1);

    if (sock == pipe) {
      zmsg_t *msg = zmsg_recv(sock);
      char *command = zmsg_popstr (msg);
      bool is_teminated = streq (command, "$TERM");
      free (command);
      zmsg_destroy (&msg);
      if (is_teminated) break;
    }
    else if (sock == req_recv_sock) {
      zmsg_t *msg = zmsg_recv(sock);
      zframe_t *identity_f = zmsg_unwrap (msg);
      zframe_t *content_f = zmsg_pop (msg);
      if (p_server->msg_handle_func) {
	p_server->msg_handle_func(zframe_data(identity_f), zframe_size(identity_f), zframe_data(content_f), zframe_size(content_f));
      }
      zframe_destroy(&identity_f);
      zframe_destroy (&content_f);
      zmsg_destroy (&msg);
    }
    else if (sock == res_recv_sock) {
      zmsg_t *msg = zmsg_recv(sock);
      zmsg_send(&msg, req_recv_sock);
      zmsg_destroy (&msg);
    }
    else {
      assert(false);
    }
  }
  zpoller_destroy(&poller);
  zsock_destroy(&req_recv_sock);
  zsock_destroy(&res_recv_sock);
}


int init_server(async_server_t** pp_server, const char* endpoint, int worker_number) {
  assert(pp_server && endpoint);
  async_server_t* temp = (async_server_t*) malloc(sizeof(async_server_t));
  memset(temp, 0, sizeof(async_server_t));

  temp->listen_endpoint = strdup(endpoint);
  temp->request_server = NULL;

  temp->worker_num = worker_number > 0 ? worker_number : DEFAULT_WORKER_NUMBER;
  temp->workers = (zactor_t**) calloc (temp->worker_num, sizeof(zactor_t*));

  /* zuuid_t* uuid = zuuid_new(); */
  /* temp->transfer_endpoint = strdup(zuuid_str(uuid)); */
  /* zuuid_destroy(&uuid); */
  temp->transfer_endpoint = strdup("inproc://backend");
  temp->result_endpoint = strdup("inproc://result");

  *pp_server = temp;
  return 0;
}

void destroy_server(async_server_t* p_server) {
  if (!p_server) return;

  free(p_server->listen_endpoint);
  free(p_server->transfer_endpoint);
  free(p_server->workers);
  free(p_server);
}

int start_server(async_server_t* p_server) {
  assert(p_server && p_server->listen_endpoint &&  p_server->transfer_endpoint);

  for(int i = 0; i < p_server->worker_num; i++) {
    p_server->workers[i] = zactor_new (worker_func, p_server);
  }

  zactor_t *request_listener = zactor_new(zproxy, NULL);
  // zstr_sendx (request_listener, "VERBOSE", NULL);
  // zsock_wait (request_listener);
  zstr_sendx (request_listener, "FRONTEND", "ROUTER", p_server->listen_endpoint, NULL);
  zsock_wait (request_listener);
  zstr_sendx (request_listener, "BACKEND", "DEALER", p_server->transfer_endpoint, NULL);
  zsock_wait (request_listener);
  p_server->request_server = request_listener;
  return 0;
}

int stop_servert(async_server_t* p_server) {
  assert(p_server);

  for(int i = 0; i < p_server->worker_num; i++) {
    if (p_server->workers[i]) {
      zactor_destroy(&(p_server->workers[i]));
      p_server->workers[i] = NULL;
    }
  }
  zactor_destroy(&(p_server->request_server));
  return 0;
}

int set_message_handler(async_server_t* p_server, message_handle_func msg_handler) {
  assert(p_server);

  p_server->msg_handle_func = msg_handler;
  return 0;
}

int send_response(async_server_t* server, void* identity, int identity_len, void* msg, int length) {
  assert(server && identity);

  zsock_t* sock = zsock_new_push(server->result_endpoint);
  zsock_set_linger(sock, -1);

  zmsg_t *reply = zmsg_new ();
  zframe_t* identify_f = zframe_new(identity, identity_len);
  zframe_t* body_f = zframe_new(msg, length);
  zmsg_push (reply, body_f);
  zmsg_wrap (reply, identify_f);
  zmsg_send (&reply, sock);
  zsock_destroy(&sock);
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "message_helper/message_sender.h"
#include "message_helper/async_server.h"

#define ENDPOINT_FOR_TEST "tcp://127.0.0.1:9100"

async_server_t* server = NULL;

int message_handler(void* identity, int identity_len, void* msg, int length) {
  send_response(server, identity, identity_len, msg, length);
  return 0;
}

int main() {

  assert(init_server(&server, ENDPOINT_FOR_TEST, 4) == 0);
  assert(set_message_handler(server, message_handler) == 0);
  assert(start_server(server) == 0);

  sleep(1);

  char* msg = "hello world";
  char* resp = NULL;
  assert(send_str_with_reply(ENDPOINT_FOR_TEST, msg, &resp) == 0);
  assert(strcmp(msg, resp) == 0);
  free(resp);
  printf("[Success] Send string and wait reply.\n");

  assert(stop_servert(server) == 0);
  printf("[Success] Stop server.\n");
  destroy_server(server);
  return 0;
}

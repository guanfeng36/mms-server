#include <stdio.h>
#include <unistd.h>
#include "message_helper/message_sender.h"

#define ENDPOINT_FOR_TEST "tcp://localhost:9100"

int main() {
  char* resp = NULL;
  send_str_with_reply(ENDPOINT_FOR_TEST, "hello woeld", &resp);
  printf("-----------[%s]\n", resp);
  free(resp);
  return 0;
}

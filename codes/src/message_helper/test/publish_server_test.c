#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h> 
#include <signal.h>
#include "message_helper/message_subscriber.h"
#include "message_helper/publish_server.h"

#define ENDPOINT_FOR_TEST "tcp://127.0.0.1:9100"


typedef enum {
    MODE_PUB,
    MODE_SUB,
    MODE_UNKNOWN
} Mode;

int running = 1;

static void s_signal_handler (int signal_value)
{
    running = 0;
}

static void s_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

int message_handler(const char* msg, int length) {
  printf("receive msg [%s]\n", msg);
  return 0;
}

int main(int argc, char* argv[]) {
    struct option long_opts[] = {
        {"mode", required_argument, NULL, 'm'},
        {"topic", required_argument, NULL, 't'},
        {"endpoint", required_argument, NULL, 'e'},
        {NULL, 0, NULL, 0}
    };

    Mode mode = MODE_UNKNOWN;
    char* endpoint = NULL;
    char* topic = NULL;
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "m:t:e:", long_opts, NULL)) != -1) 
    {
        switch (opt) {
        case 't':
            topic = strdup(optarg);
            break;
        case 'm':
            if (strcmp(optarg, "sub") == 0) mode = MODE_SUB;
            else if (strcmp(optarg, "pub") == 0) mode = MODE_PUB;
            break;
        case 'e':
            endpoint = strdup(optarg);
            break;
        default:
            break;
        }
    }

    assert(mode == MODE_SUB || mode == MODE_PUB);
    assert(endpoint != NULL && topic != NULL);

    if (mode == MODE_PUB) {
        publish_server_t* server = NULL;
        assert(init_pub_server(&server, endpoint) == 0);
        assert(start_pub_server(server) == 0);
        s_catch_signals();

        int i = 0;
        char tmp[32] = {0};

        while (running) {
            snprintf(tmp, sizeof(tmp)-1, "%d", i);
            assert(publish_message(server, topic, tmp) == 0);
            printf("[Success] Publish message[%s].\n", tmp);
            i++;
            sleep(1);
        }

        assert(stop_pub_servert(server) == 0);
        printf("[Success] Stop server.\n");
        destroy_pub_server(server);
    }
    else {
        create_subscriber_and_wait_msg(endpoint, topic, message_handler);
    }
  
    free(topic);
    free(endpoint);
    return 0;
}

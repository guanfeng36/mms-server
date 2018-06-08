#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <json-c/json.h>
#include <assert.h>
#include "common/climit.h"
#include "common/message.h"
#include "message_helper/async_server.h"
#include "message_helper/message_sender.h"

#define ENDPOINT_FOR_TEST "tcp://*:10001"

async_server_t* server = NULL;
char** g_argv = NULL;
int g_argc = 0;
int g_index = 1;

int message_handler(void* identity, int identity_len, void* msg, int length) {
    printf("receive message[%d][%.*s]\n", length, length, (char*)msg);
    char* data = NULL;
    FILE *fp;
    fp = fopen(g_argv[g_index] , "r");
    if (!fp) { 
        printf("can not open resp file[%s]\n", g_argv[g_index]);
        const char* resp = "can not find resp file!";
        send_response(server, identity, identity_len, (void*)resp, strlen(resp));
        return 0;
    }

    fseek( fp , 0 , SEEK_END );
    int file_size;
    file_size = ftell( fp );

    fseek( fp , 0 , SEEK_SET);
    data =  (char *)malloc( file_size + 1 );
    memset(data, 0, file_size + 1);
    fread( data , file_size , sizeof(char) , fp);
    printf("read data from resp.txt[%s][%d]\n", data, file_size);
    send_response(server, identity, identity_len, (void*)data, strlen(data));

    g_index++;
    if (g_index >= g_argc) g_index = 1;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        printf("no data file found.\n");
        return 0;
    }

    g_argv = argv;
    g_argc = argc;

    assert(init_server(&server, ENDPOINT_FOR_TEST, 1) == 0);
    assert(set_message_handler(server, message_handler) == 0);
    assert(start_server(server) == 0);
    sleep(1);

    pause();

    assert(stop_servert(server) == 0);
    destroy_server(server);

    return 0;
}

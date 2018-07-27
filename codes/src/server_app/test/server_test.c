#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <json-c/json.h>

#include "common/climit.h"
#include "common/message.h"
#include "message_helper/message_sender.h"

#define ENDPOINT_FOR_TEST "tcp://localhost:9000"

void inform_test() {
  struct json_object *req_object = json_object_new_object(); 
  json_object_object_add(req_object, "type", json_object_new_string(MESSAGE_TYPE_INFORM));
  json_object_object_add(req_object, "id", json_object_new_string("aa:bb:cc:dd:ee:ff"));

  struct json_object *data_object = json_object_new_object();
  json_object_object_add(data_object, "mac_addr", json_object_new_string("aa:bb:cc:dd:ee:ff"));
  json_object_object_add(data_object, "version", json_object_new_string("1.1.1.1"));  
  json_object_object_add(data_object, "current_time", json_object_new_string("2017-01-02 10:10:10"));  
  json_object_object_add(data_object, "retry_count", json_object_new_int(110));  
  json_object_object_add(data_object, "ipv4", json_object_new_string("10.10.10.10"));
  json_object_object_add(data_object, "os_type", json_object_new_string("JHLS 5.1"));
  json_object_object_add(data_object, "hb_interval", json_object_new_int(20));
  json_object_object_add(data_object, "station", json_object_new_string("wuxi 10001"));
  json_object_object_add(data_object, "station_info", json_object_new_string("111111111"));
  json_object_object_add(data_object, "address", json_object_new_string("jiangsuwuxi"));
  json_object_object_add(data_object, "location", json_object_new_string("wuxi"));
  json_object_object_add(data_object, "hw_type", json_object_new_string("pd001"));
  json_object_object_add(data_object, "machine_model", json_object_new_string("asus"));
  json_object_object_add(data_object, "cpu_model", json_object_new_string("test_cpu"));
  json_object_object_add(data_object, "kernel_info", json_object_new_string("test_kernel"));
  json_object_object_add(data_object, "interface", json_object_new_string("test_for_interface"));
  json_object_object_add(data_object, "network", json_object_new_string("test_for_network"));
  json_object_object_add(data_object, "ps2_device", json_object_new_string("test_ps2_device"));
  json_object_object_add(data_object, "usb_device", json_object_new_string("test_usb_device"));
  json_object_object_add(data_object, "com", json_object_new_string("test_com"));
  json_object_object_add(data_object, "graphics", json_object_new_string("test_graphics"));
  json_object_object_add(data_object, "libs", json_object_new_string("test_libs"));
  json_object_object_add(data_object, "software", json_object_new_string("test_software"));
  json_object_object_add(req_object, "data",data_object);

  char* resp = NULL;
  send_str_with_reply(ENDPOINT_FOR_TEST, json_object_to_json_string(req_object), &resp);

  printf("-----------[%s]\n", resp);
  free(resp);

  json_object_put(req_object);
}

void heartbeat_test() {
  struct json_object *req_object = json_object_new_object(); 
  json_object_object_add(req_object, "type", json_object_new_string(MESSAGE_TYPE_HEARTBEAT));
  json_object_object_add(req_object, "id", json_object_new_string("aa:bb:cc:dd:ee:ff"));

  char* resp = NULL;
  send_str_with_reply(ENDPOINT_FOR_TEST, json_object_to_json_string(req_object), &resp);

  printf("-----------[%s]\n", resp);
  free(resp);

  json_object_put(req_object);
}

void update_task_status_test() {
  struct json_object *req_object = json_object_new_object(); 
  json_object_object_add(req_object, "type", json_object_new_string(MESSAGE_TYPE_UPDATE_TASK_STATUS));
  json_object_object_add(req_object, "id", json_object_new_string("aa:bb:cc:dd:ee:ff"));

  char* resp = NULL;
  send_str_with_reply(ENDPOINT_FOR_TEST, json_object_to_json_string(req_object), &resp);

  printf("-----------[%s]\n", resp);
  free(resp);

  json_object_put(req_object);
}

void print_usage() {
  printf("Usage: server_test -d file -h host_ip -p port\n");
  printf("      option -d: json data file\n");
  printf("      option -h: server host ip address\n");
  printf("      option -p: server host listen port\n");
}

int main(int argc, char* argv[]) {
    int ret_val = 0;

    struct option long_opts[] = {
      {"data", required_argument, NULL, 'd'},
      {"host", required_argument, NULL, 'h'},
      {"port", required_argument, NULL, 'p'},
      {NULL, 0, NULL, 0}
    };

    char* data_file = NULL;
    char* host = NULL;
    int port = 0;

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "d:h:p:", long_opts, NULL)) != -1) 
    {
        switch (opt) {
        case 'd':
	    data_file = strdup(optarg);
            break;
        case 'h':
	    host = strdup(optarg);
            break;
        case 'p':
	    port = atoi(optarg);
            break;
        default:
            break;
        }
    }

    char* msg_data = NULL;
    int msg_data_len = 0;
    {
        FILE* infilefd = fopen(data_file, "rb");
        if (infilefd == NULL) {
            fprintf(stderr, "data file[%s] is not existed.\n", data_file);
	    print_usage();
            ret_val = -1;
	    goto end;
        }

        fseek(infilefd, 0, SEEK_END);
        msg_data_len = ftell(infilefd);
        fseek(infilefd, 0, SEEK_SET);

        if (msg_data_len > 0) {
            msg_data = (char *) malloc(msg_data_len);
            memset(msg_data, 0, msg_data_len);

            fread(msg_data, 1, msg_data_len, infilefd);
        }
        fclose(infilefd);
    }

    char end_point[ENDPOINT_LEN_MAX] = {0};
    sprintf(end_point, "tcp://%s:%d", host, port);

    if (!msg_data || msg_data_len <= 0) {
        printf("no data read from file!\n"); 
        printf("try to send empty message to endpoint[%s]\n", end_point);
        char* resp = NULL;
        send_str_with_reply(end_point, "", &resp);
        printf("receive response [%s]\n", resp);
        free(resp);
    }
    else {
        json_object *msg_json = json_tokener_parse((char*)msg_data);
        if (!msg_json) {
            fprintf(stderr, "file content is not valid json data!\n");
            ret_val = -1;
            goto end;
        }

        printf("try to send message to endpoint[%s]\n", end_point);
        char* resp = NULL;
        send_str_with_reply(end_point, json_object_to_json_string(msg_json), &resp);
        printf("receive response [%s]\n", resp);
        free(resp);

        json_object_put(msg_json);
    }
 end:
    free(msg_data);
    free(data_file);
    free(host);

    return ret_val;
}

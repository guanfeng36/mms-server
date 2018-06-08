#include "server_app_main.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <json-c/json.h>

#include "minIni/minIni.h"
#include "common/common.h"
#include "common/kafka_util.h"
#include "common/base64.h"
#include "common/crypt.h"
#include "common/log.h"
#include "common/climit.h"
#include "common/message.h"
#include "message_helper/async_server.h"

#include "resp_sender_service_func.h"

static char g_listen_endpoint[ENDPOINT_LEN_MAX] = {0};
static int g_worker_num = 0;

char g_kafka_brokers[KAFKA_BROKERS_LEN_MAX] = {0};

async_server_t* server = NULL;

static rd_kafka_t *send_rk;

#define AUTHORIZATION_LEN_MAX 128
int check_authorization(const char* authorization) {
    if (!authorization) return -1;

    log_info("authorization:[%s]", authorization);

    // decode base64 encoded authorization                                                                                                                                                                                                   
    char orig_auth[AUTHORIZATION_LEN_MAX] = {0};
    int auth_size = base64_decode(authorization, orig_auth, sizeof(orig_auth));
    if (auth_size <= 0) {
        log_error("failed to decode authorization");
        return -1;
    }
    log_info("auth buffer [%s]  len[%d] len [%d]", orig_auth, auth_size, (int)strlen(orig_auth));

    // get the position of ':'                                                                                                                                                                                                               
    int delimeter_pos = -1;
    for (int i = 0; i < auth_size; i++) {
        if(orig_auth[i] ==':') {
            delimeter_pos = i;
            break;
        }
    }
    if (delimeter_pos <= 0) {
        log_error("cannot find delimeter[:]");
        return -1;
    }
    orig_auth[delimeter_pos] = '\0';

    // get the base64 encoded id on the left of ':'                                                                                                                                                                                          
    char id_left[AUTHORIZATION_LEN_MAX] = {0};
    int id_left_len = base64_decode(orig_auth, id_left, sizeof(id_left));
    if (id_left_len <= 0) {
        log_error("failed to decode id value");
        return -1;
    }

    // get the CBC encoded id on the right of ':'                                                                                                                                                                                            
    char id_right[AUTHORIZATION_LEN_MAX] = {0};
    int id_right_len = decrypt_text(EVP_3DES_CBC, AUTH_ENCODE_KEY, (unsigned char*)(orig_auth + delimeter_pos + 1), auth_size - delimeter_pos - 1, (unsigned char*)id_right, sizeof(id_right));
    if (id_right_len <= 0) {
        log_error("failed to decode id value");
        return -1;
    }

    log_info("id1[%s:%d], id2[%s:%d]", id_left, id_left_len, id_right, id_right_len);

    if ((id_left_len == id_right_len) && (0 == memcmp(id_left, id_right, id_left_len))) {
        return 0;
    }
    log_info("authorization is not valid");
    return -1;
}

int send_errmsg_response(async_server_t* server, void* identity, int identity_len, error_code_t error_code) {
    assert(server && identity);

    struct json_object *resp_object = json_object_new_object();
    json_object_object_add(resp_object, "type", json_object_new_string(MESSAGE_TYPE_BAD_REQUEST));
    json_object_object_add(resp_object, "resp_code", json_object_new_int(error_code));
    json_object_object_add(resp_object, "resp_msg", json_object_new_string(get_error_string(error_code)));

    const char* data = json_object_to_json_string(resp_object);
    int length = strlen(data);
    send_response(server, identity, identity_len, (void*)data, length);

    json_object_put(resp_object);
    return 0;
}

int message_handler(void* identity, int identity_len, void* request, int req_length) {
    assert(identity && identity_len > 0);

    log_debug("receive message[%.*s]", req_length, (char*)request);

    if (!request || req_length <= 0) {
        send_errmsg_response(server, identity, identity_len, ERROR_CODE_REQ_EMPTY);
        return 0;
    }

    if (req_length > REQUEST_BODY_LENGTH_MAX) {
        send_errmsg_response(server, identity, identity_len, ERROR_CODE_DATA_TOO_LARGE);
        return 0;
    }

    json_object *req_json = NULL;
    json_object *msg_object = NULL;

    char* temp = strndup((char*)request, req_length);
    req_json = json_tokener_parse(temp);
    free(temp);

    if (!req_json) {
        send_errmsg_response(server, identity, identity_len, ERROR_CODE_FORMAT_ERROR);
        goto end;
    }

    msg_object = json_object_new_object();

    char base64_str[64] = {0};
    base64_encode((char*)identity, identity_len, base64_str, sizeof(base64_str));
    json_object_object_add(msg_object, "identify", json_object_new_string(base64_str));
    json_object_object_add(msg_object, "msg_body", req_json);

    json_object *type_json = NULL;
    const char* type = NULL;
    if (!json_object_object_get_ex(req_json, "type", &type_json)) {
        log_warning("cannot find value for argument type");
        send_errmsg_response(server, identity, identity_len, ERROR_CODE_UNKNOWN_REQUEST);
        goto end;
    }
    type = json_object_get_string(type_json);

    json_object *id_json = NULL;
    const char* id = NULL;
    if (json_object_object_get_ex(req_json, "id", &id_json)) {
        id = json_object_get_string(id_json);
    }

    const char* authorization = NULL;
    json_object *authorization_json = NULL;
    if (json_object_object_get_ex(req_json, "authorization", &authorization_json)) {
        authorization = json_object_get_string(authorization_json);
    }

    if (strcmp(type, MESSAGE_TYPE_REGISTER) != 0) {
        if (id == NULL || id[0] == '\0') {
            log_warning("failed to do get device id in request");
            send_errmsg_response(server, identity, identity_len, ERROR_CODE_NO_DEVICE_FOUND);
            goto end;
        }
        /* if (!authorization || check_authorization(authorization) != 0) { */
        /*     log_warning("failed to do authorization"); */
        /*     send_errmsg_response(server, identity, identity_len, ERROR_CODE_UNAUTHORIZED_REQUEST); */
        /*     goto end; */
        /* } */
    }

    const char* service_name = get_service_name(type);
    if (!service_name) {
        log_warning("cannot find service for type[%s]", type);
        send_errmsg_response(server, identity, identity_len, ERROR_CODE_UNKNOWN_REQUEST);
        goto end;
    }

    const char* msg_str = json_object_to_json_string(msg_object);

    send_msg(send_rk, service_name, msg_str, strlen(msg_str));

 end:
    if (msg_object) json_object_put(msg_object);
    return 0;
}

int main() {
    assert (initlog(NULL, "server_app") >= 0);

    if (ini_gets("server", "listen_end", "", g_listen_endpoint, sizeof(g_listen_endpoint), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get listen end point!");
        strncpy(g_listen_endpoint, DEFAULT_LISTEN_ENDPOINT, sizeof(g_listen_endpoint));
    }

    if ((g_worker_num = ini_getl("server", "worker_num", -1, DEFAULT_CONFIG_FILE)) <= 0) {
        log_warning("Can not worker num!");
        g_worker_num = DEFAULT_WORKER_NUM;
    }

    if (ini_gets("kafka", "brokers", "localhost:9092", g_kafka_brokers, sizeof(g_kafka_brokers), DEFAULT_CONFIG_FILE) <= 0) {
        log_warning("Can not get kafka brokers!");
        strncpy(g_kafka_brokers, "localhost:9092", sizeof(g_kafka_brokers));
    }

    init_producer_rk(&send_rk, g_kafka_brokers);

    assert(init_server(&server, g_listen_endpoint, g_worker_num) == 0);
    assert(set_message_handler(server, message_handler) == 0);
    assert(start_server(server) == 0);
    sleep(1);

    pthread_t receive_pid = {0};
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&receive_pid, &attr, (void *) resp_sender_service_func, server);
    pthread_attr_destroy(&attr);

    pause();

    assert(stop_servert(server) == 0);
    destroy_server(server);

    releaseLog();

    free_producer_rk(send_rk);
    return 0;
}

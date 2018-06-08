#include "resp_sender_service_func.h"

#include <stdlib.h>
#include <sys/prctl.h>
#include <assert.h>
#include <json-c/json.h>

#include "common/climit.h"
#include "common/log.h"
#include "common/base64.h"
#include "common/message.h"
#include "message_helper/async_server.h"
#include "common/kafka_util.h"

extern char g_kafka_brokers[KAFKA_BROKERS_LEN_MAX];

static async_server_t* server;
static rd_kafka_t *resp_srv_recv_rk;

static void msg_consume (rd_kafka_message_t *rkmessage, void *opaque) {
	if (rkmessage->err) {
		if (rkmessage->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
			log_debug(
                      "%% Consumer reached end of %s [%"PRId32"] "
                      "message queue at offset %"PRId64"\n",
                      rd_kafka_topic_name(rkmessage->rkt),
                      rkmessage->partition, rkmessage->offset);
			return;
		}

		log_error("%% Consume error for topic \"%s\" [%"PRId32"] "
                  "offset %"PRId64": %s\n",
                  rd_kafka_topic_name(rkmessage->rkt),
                  rkmessage->partition,
                  rkmessage->offset,
                  rd_kafka_message_errstr(rkmessage));

        if (rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_PARTITION ||
            rkmessage->err == RD_KAFKA_RESP_ERR__UNKNOWN_TOPIC)
            // run = 0;
            return;
	}

    log_debug("%% Message (offset %"PRId64", %zd bytes):\n",
              rkmessage->offset, rkmessage->len);

	if (rkmessage->key_len) {
        log_debug("Key: %.*s\n",
                  (int)rkmessage->key_len, (char *)rkmessage->key);
	}

    log_debug("%.*s\n",
              (int)rkmessage->len, (char *)rkmessage->payload);

    char* temp = strndup((char*)rkmessage->payload, (int)rkmessage->len);
    json_object *msg_json = json_tokener_parse(temp);
    free(temp);
    temp = NULL;

    char* identify = NULL;
    json_object *identify_json = NULL;
    if (json_object_object_get_ex(msg_json, "identify", &identify_json)) {
        if (json_object_get_string(identify_json))
            identify = strdup(json_object_get_string(identify_json));
    }

    const char* msg_to_send = NULL;
    json_object *msg_body_json = NULL;
    if (!json_object_object_get_ex(msg_json, "msg_body", &msg_body_json)) {
        log_error("cannot get msg_body from message received!");
    }
    msg_to_send = json_object_to_json_string(msg_body_json);
    char ori_identity[64] = {0};
    int len = base64_decode(identify, ori_identity, sizeof(ori_identity));
    if (server) {
        send_response(server, ori_identity, len, (void*)msg_to_send, strlen(msg_to_send));
    }
    json_object_put(msg_json);
    free(identify);
}

static const char* topic = RESPONSE_SENDER_SERVICE;
static const char *group = "response_group";
void resp_sender_service_func(void* arg) {
    prctl(PR_SET_NAME, "resp_srv_thd");

    server = (async_server_t*) arg;
    assert(arg != NULL);

    init_consumer_rk(&resp_srv_recv_rk, group, g_kafka_brokers);
    assert(resp_srv_recv_rk != NULL);

    rd_kafka_resp_err_t err;
    rd_kafka_topic_partition_list_t *topics = NULL;
    topics = rd_kafka_topic_partition_list_new(3);
    rd_kafka_topic_partition_list_add(topics, topic, -1);

    if ((err = rd_kafka_subscribe(resp_srv_recv_rk, topics))) {
        log_error("%% Failed to start consuming topics: %s\n",
                  rd_kafka_err2str(err));
        exit(1);
    }

    while (1) {
        rd_kafka_message_t *rkmessage;

        rkmessage = rd_kafka_consumer_poll(resp_srv_recv_rk, 1000);

        if (rkmessage) {
            msg_consume(rkmessage, NULL);
            rd_kafka_message_destroy(rkmessage);
        }
    }

    /* Stop consuming */
    err = rd_kafka_consumer_close(resp_srv_recv_rk);
    if (err)
        log_error("%% Failed to close consumer: %s\n",
                  rd_kafka_err2str(err));
    else
        log_debug("%% Consumer closed\n");

    rd_kafka_topic_partition_list_destroy(topics);

    /* Destroy handle */
    free_consumer_rk(resp_srv_recv_rk);
}


#include "kafka_util.h"

#include <syslog.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include "common/log.h"

/**
 * Message delivery report callback.
 * Called once for each message.
 * See rdkafka.h for more information.
 */
void msg_delivered (rd_kafka_t *rk,
			   void *payload, size_t len,
			   int error_code,
			   void *opaque, void *msg_opaque) {

	if (error_code)
		log_error("%% Message delivery failed: %s\n",
                  rd_kafka_err2str(error_code));
	else {
		log_debug("%% Message delivered (%zd bytes)\n", len);
    }
}

void init_producer_rk(rd_kafka_t **rk, const char* brokers) {
    assert(rk != NULL);
    rd_kafka_t *temp = NULL;

    // char *brokers = "localhost:9092"; 
    rd_kafka_conf_t *conf = rd_kafka_conf_new();
    rd_kafka_conf_set(conf, "queue.buffering.max.messages", "500000", NULL, 0);
    rd_kafka_conf_set(conf, "message.send.max.retries", "3", NULL, 0);
    rd_kafka_conf_set(conf, "retry.backoff.ms", "500", NULL, 0);

    rd_kafka_conf_set_dr_cb(conf, msg_delivered);

    char errstr[512] = {0};
    if (!(temp = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr)))) {
        log_error("%% Failed to create new producer: %s\n", errstr);
        exit(1);
    }

    rd_kafka_set_log_level(temp, LOG_DEBUG);

    if (rd_kafka_brokers_add(temp, brokers) == 0) {
        log_error("%% No valid brokers specified\n");
        exit(1);
    }

    *rk = temp;
}

void free_producer_rk(rd_kafka_t *rk) {
    rd_kafka_destroy(rk);
}

void send_msg(rd_kafka_t *rk, const char* topic, const char* msg, size_t length) {
    int partition = RD_KAFKA_PARTITION_UA;

    rd_kafka_topic_conf_t *topic_conf;
    topic_conf = rd_kafka_topic_conf_new();

    rd_kafka_topic_t* rkt = rd_kafka_topic_new(rk, topic, topic_conf);
    if (rd_kafka_produce(rkt, partition,
					     RD_KAFKA_MSG_F_COPY,
					     /* Payload and length */
					     (char*)msg, length,
					     /* Optional key and its length */
					     NULL, 0,
					     /* Message opaque, provided in
					      * delivery report callback as
					      * msg_opaque. */
					     NULL) == -1) {
        log_error(
                  "%% Failed to produce to topic %s "
                  "partition %i: %s\n",
                  rd_kafka_topic_name(rkt), partition,
                  rd_kafka_err2str(rd_kafka_errno2err(errno)));
        /* Poll to handle delivery reports */
        rd_kafka_poll(rk, 0);
    }
    else {
        log_debug("%% Sent %zd bytes to topic "
                "%s partition %i\n",
				length, rd_kafka_topic_name(rkt), partition);
        /* Poll to handle delivery reports */
        rd_kafka_poll(rk, 0);
    }

    rd_kafka_poll(rk, 0);
    while (rd_kafka_outq_len(rk) > 0) rd_kafka_poll(rk, 100);
    rd_kafka_topic_destroy(rkt);
}


void rebalance_cb (rd_kafka_t *rk,
                   rd_kafka_resp_err_t err,
                   rd_kafka_topic_partition_list_t *partitions,
                   void *opaque) {

	log_debug("%% Consumer group rebalanced: ");

	switch (err)
	{
	case RD_KAFKA_RESP_ERR__ASSIGN_PARTITIONS:
		log_debug("assigned:\n");
        //		print_partition_list(stderr, 1, partitions);
		rd_kafka_assign(rk, partitions);
		break;

	case RD_KAFKA_RESP_ERR__REVOKE_PARTITIONS:
		log_debug("revoked:\n");
        //		print_partition_list(stderr, 0, partitions);
		rd_kafka_assign(rk, NULL);
		break;

	default:
		log_error("failed: %s\n",
                        rd_kafka_err2str(err));
		break;
	}
}

void init_consumer_rk(rd_kafka_t **rk, const char* group, const char* brokers) {
    assert(rk != NULL);
    rd_kafka_t *temp_rk = NULL;

    char errstr[512] = {0};

    rd_kafka_conf_t *conf = NULL;
    rd_kafka_topic_conf_t *topic_conf;

    conf = rd_kafka_conf_new();
    topic_conf = rd_kafka_topic_conf_new();

    /* Consumer grups require a group id */
    if (rd_kafka_conf_set(conf, "group.id", group,
                          errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK) {
        log_error("%% %s\n", errstr);
        exit(1);
    }

    /* Consumer groups always use broker based offset storage */
    if (rd_kafka_topic_conf_set(topic_conf, "offset.store.method", "broker",
                                errstr, sizeof(errstr)) !=
        RD_KAFKA_CONF_OK) {
        log_error("%% %s\n", errstr);
        exit(1);
    }

    /* Set default topic config for pattern-matched topics. */
    rd_kafka_conf_set_default_topic_conf(conf, topic_conf);

    /* Callback called on partition assignment changes */
    rd_kafka_conf_set_rebalance_cb(conf, rebalance_cb);

    /* Create Kafka handle */
    if (!(temp_rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf,
                                          errstr, sizeof(errstr)))) {
        log_error("%% Failed to create new consumer: %s\n",
                  errstr);
        exit(1);
    }

   /* Add brokers */
    if (rd_kafka_brokers_add(temp_rk, brokers) == 0) {
        log_error("%% No valid brokers specified\n");
        exit(1);
    }

    rd_kafka_poll_set_consumer(temp_rk);

    //    rd_kafka_set_log_level(temp_rk, LOG_DEBUG);

    *rk = temp_rk;
}

void free_consumer_rk(rd_kafka_t *rk) {
    rd_kafka_destroy(rk);
}

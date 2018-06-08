#ifndef __KAFKA_UTIL_H_
#define __KAFKA_UTIL_H_

#include "librdkafka/rdkafka.h" 

void init_producer_rk(rd_kafka_t **rk, const char* brokers);
void free_producer_rk(rd_kafka_t *rk);
void send_msg(rd_kafka_t *rk, const char* topic, const char* msg, size_t length);

void init_consumer_rk(rd_kafka_t **rk, const char* group, const char* brokers);
void free_consumer_rk(rd_kafka_t *rk);

#endif  // __KAFKA_UTIL_H_

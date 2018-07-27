#include "czmq.h"
#include "message_helper/message_subscriber.h"

#define HB_TOPIC "__hb__"
#define HB_TIMEOUT (120*1000)

int create_subscriber_and_wait_msg(const char* endpoint, const char* topic, sub_cb_func cb_func) {
    int retVal = 0;
    zsock_t *sub = zsock_new_sub (endpoint, topic ? topic : "");
    zsock_set_subscribe(sub, HB_TOPIC);
    zpoller_t *poller = zpoller_new (sub, NULL);
    while (true) {
        zsock_t* sock = zpoller_wait (poller, HB_TIMEOUT);

        bool is_expired = zpoller_expired(poller);
        if (sock == NULL && is_expired) {
            zsock_disconnect(sub, endpoint);
            zsock_connect(sub, endpoint);
            continue;
        }
        if (sock == sub) {
            zmsg_t *msg = zmsg_recv(sock);

            zframe_t *topic_f = zmsg_first(msg);
            if (topic_f == NULL) {
                zmsg_destroy (&msg);
                continue;
            }

            if (strncmp((char*)zframe_data(topic_f), HB_TOPIC, (int)zframe_size(topic_f)) == 0) {
                zmsg_destroy (&msg);
                continue;
            }

            zframe_t *content_f = zmsg_next (msg);
            int rc = 0;
            if (cb_func) {
                rc = cb_func((char*)zframe_data(content_f), (int)zframe_size(content_f));
            }
            zmsg_destroy (&msg);

            if (rc < 0) {
                retVal = rc;
                break;
            }
        }
        else {
            retVal = -1;
            break;
        }
    }

    zpoller_destroy(&poller);
    zsock_destroy(&sub);
    return retVal;
}

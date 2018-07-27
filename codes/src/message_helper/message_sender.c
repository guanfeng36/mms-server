#include "czmq.h"
#include "message_helper/message_sender.h"

int send_str_without_reply(const char* endpoint, const char* msg) {
    assert(endpoint != NULL);

    if (!msg) return false;

    zsock_t* sender = zsock_new_req(endpoint);
    zsock_set_linger(sender, -1);
    int rc = zsock_send(sender, "s", msg);
    zsock_destroy(&sender);
    return rc;
}

/* bool send_bin_data_without_reply(uint8_t* msg, size_t length) { */
/*   return true; */
/* } */

int send_str_with_reply(const char* endpoint, const char* msg, char** resp) {
    assert(endpoint != NULL);

    if (!msg) return false;

    int rc = 0;

    zsock_t* sender = zsock_new_req(endpoint);
    zsock_set_linger(sender, -1);
    zsock_set_sndtimeo(sender, 30000);
    zsock_set_rcvtimeo(sender, 30000);

    rc = zsock_send(sender, "s", msg);
    if (rc == 0) {
        rc = zsock_recv(sender, "s", resp);
    }
    zsock_destroy(&sender);

    return rc;
}

/* bool send_bin_data_with_reply() { */
/*   return true; */
/* } */

/*
   rxs_sender
   -----------

*/

#ifndef RXS_SENDER_H
#define RXS_SENDER_H

#include <uv.h>

typedef struct rxs_sender rxs_sender;

struct rxs_sender {
    struct sockaddr_in saddr;
    int sd;
};

int rxs_sender_init(rxs_sender* net, const char* ip, int port);
int rxs_sender_deinit(rxs_sender* net);
int rxs_sender_send(rxs_sender* net, uint8_t* buffer, uint32_t nbytes);
void rxs_sender_update(rxs_sender* net);

#endif

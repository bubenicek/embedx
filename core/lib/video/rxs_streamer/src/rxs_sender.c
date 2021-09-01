
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <rxs_streamer/rxs_sender.h>

#define TRACE_TAG   "rxs_sender"

int rxs_sender_init(rxs_sender* net, const char* ip, int port)
{
    memset(net, 0, sizeof(rxs_sender));

    if ((net->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        TRACE_ERROR("Create UDP socket failed");
        return -1;
    }

    memset((char *) &net->saddr, 0, sizeof(net->saddr));
    net->saddr.sin_family = AF_INET;
    net->saddr.sin_port = htons(port);
    inet_aton(ip, &net->saddr.sin_addr);

    TRACE("RSX sender to %s:%d initialized", ip, port);

    return 0;
}

int rxs_sender_deinit(rxs_sender* net)
{
    if (net->sd > 0)
    {
        close(net->sd);
        net->sd = -1;
    }

    return 0;
}

int rxs_sender_send(rxs_sender *net, uint8_t *buffer, uint32_t nbytes)
{
    return sendto(net->sd, buffer, nbytes, 0, (struct sockaddr*) &net->saddr, sizeof(struct sockaddr_in));
}


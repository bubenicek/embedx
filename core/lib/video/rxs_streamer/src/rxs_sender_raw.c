
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>

#include <rxs_streamer/rxs_sender_raw.h>

#define TRACE_TAG   "rxs_sender"


int rxs_sender_raw_init(rxs_sender_raw* net, struct sockaddr_in *saddr, struct sockaddr_in *daddr)
{
    struct sockaddr_in si_me;

    memset(net, 0, sizeof(rxs_sender_raw));

    // Create socket
    if ((net->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        TRACE_ERROR("Create RAW UDP socket failed");
        return -1;
    }

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = saddr->sin_port;
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(net->sd, (const struct sockaddr *)&si_me, sizeof(struct sockaddr_in)) < 0)
    {
        TRACE_ERROR("Bind UDP socket: errno %d", errno);
        return 1;
    }


    memset((char *) &net->saddr, 0, sizeof(net->saddr));
    net->saddr = *saddr;
    net->daddr = *daddr;

    TRACE_PRINTFF("RSX sender initialized, %s:%d -> ", inet_ntoa(net->saddr.sin_addr), ntohs(net->saddr.sin_port));
    TRACE_PRINTF("%s:%d\n", inet_ntoa(net->daddr.sin_addr), ntohs(net->daddr.sin_port));

    return 0;
}

int rxs_sender_raw_deinit(rxs_sender_raw* net)
{
    if (net->sd > 0)
    {
        close(net->sd);
        net->sd = -1;
    }

    return 0;
}

int rxs_sender_raw_send(rxs_sender_raw *net, uint8_t *buf, uint32_t bufsize)
{
    return sendto(net->sd, buf, bufsize, 0, (struct sockaddr*) &net->daddr, sizeof(struct sockaddr_in));
}

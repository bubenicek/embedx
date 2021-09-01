/*
   rxs_sender
   -----------

*/

#ifndef RXS_SENDER_RAW_H
#define RXS_SENDER_RAW_H

#include <uv.h>
#include <pcap.h>

#ifndef CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE
#define CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE     1500
#endif

typedef struct rxs_sender_raw rxs_sender_raw;

struct rxs_sender_raw
{
    int sd;

    uint16_t id;

    struct sockaddr_in saddr;       // Source address
    struct sockaddr_in daddr;       // Destination address

	struct ifreq if_mac;
	pcap_t *pcap;

    uint8_t datagram[CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE];
    uint8_t pseudogram[CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE];
};

int rxs_sender_raw_init(rxs_sender_raw* net, struct sockaddr_in *saddr, struct sockaddr_in *daddr);
int rxs_sender_raw_deinit(rxs_sender_raw* net);
int rxs_sender_raw_send(rxs_sender_raw* net, uint8_t* buffer, uint32_t nbytes);
void rxs_sender_raw_update(rxs_sender_raw* net);

#endif

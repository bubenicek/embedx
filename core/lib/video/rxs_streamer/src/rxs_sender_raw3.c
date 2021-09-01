
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

#define DEFAULT_IF	    "wlp3s0"

#define SRC_MAC0	0xB4
#define SRC_MAC1	0xE6
#define SRC_MAC2	0x2D
#define SRC_MAC3	0xC1
#define SRC_MAC4	0x59
#define SRC_MAC5	0x95


/** 96 bit (12 bytes) pseudo header needed for udp header checksum calculation */
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};


static unsigned short csum(unsigned short *buf, int nwords)
{
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}


int rxs_sender_raw_init(rxs_sender_raw* net, struct sockaddr_in *saddr, struct sockaddr_in *daddr)
{
	char ifName[IFNAMSIZ] = DEFAULT_IF;
	struct sockaddr_ll socket_address;

    memset(net, 0, sizeof(rxs_sender_raw));

    // Create socket
    if ((net->sd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        TRACE_ERROR("Create RAW UDP socket failed");
        return -1;
    }

	// Get the MAC address of the interface to send on
	memset(&net->if_mac, 0, sizeof(struct ifreq));
	strncpy(net->if_mac.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(net->sd, SIOCGIFHWADDR, &net->if_mac) < 0)
	{
        TRACE_ERROR("SIOCGIFHWADDR");
        goto fail;
	}

	// Bind socket to intreface by name
    memset(&socket_address, 0, sizeof (socket_address));
    socket_address.sll_family = PF_PACKET;
    socket_address.sll_ifindex = if_nametoindex(DEFAULT_IF);
    socket_address.sll_protocol = htons(ETH_P_ALL);

    if (bind(net->sd, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0)
    {
        TRACE_ERROR("Bind socket failed");
        goto fail;
    }

    memset((char *) &net->saddr, 0, sizeof(net->saddr));
    net->saddr = *saddr;
    net->daddr = *daddr;

    TRACE_PRINTFF("RSX sender initialized, %s:%d -> ", inet_ntoa(net->saddr.sin_addr), ntohs(net->saddr.sin_port));
    TRACE_PRINTF("%s:%d\n", inet_ntoa(net->daddr.sin_addr), ntohs(net->daddr.sin_port));

    return 0;

fail:
    close(net->sd);
    return -1;
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
    int res;
    struct pseudo_header psh;
    struct ether_header *eh = (struct ether_header *) net->datagram;
    struct iphdr *iph = (struct iphdr *)(net->datagram + sizeof(struct ether_header));  // IP header
    struct udphdr *udph = (struct udphdr *)(net->datagram + sizeof(struct ether_header) + sizeof(struct ip)); // UDP header
    uint8_t *data = net->datagram + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr);
    int txlen = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr) + bufsize;

    // Data part
    if (sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr) + bufsize > CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE)
    {
        TRACE_ERROR("RAW UDP packet size ovwerflow max size %d", CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE);
        return -1;
    }

    // Zero out the packet buffer
    memset(net->datagram, 0, sizeof(net->datagram));

    // Copy data payload
    memcpy(data, buf, bufsize);

	// Ethernet header
	eh->ether_dhost[0] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[0];
	eh->ether_dhost[1] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[1];
	eh->ether_dhost[2] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[2];
	eh->ether_dhost[3] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[3];
	eh->ether_dhost[4] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[4];
	eh->ether_dhost[5] = ((uint8_t *)&net->if_mac.ifr_hwaddr.sa_data)[5];
	eh->ether_shost[0] = SRC_MAC0;
	eh->ether_shost[1] = SRC_MAC1;
	eh->ether_shost[2] = SRC_MAC2;
	eh->ether_shost[3] = SRC_MAC3;
	eh->ether_shost[4] = SRC_MAC4;
	eh->ether_shost[5] = SRC_MAC5;
	eh->ether_type = htons(ETH_P_IP);

    // Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + bufsize);
    iph->id = htons(net->id++);	                  // Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;		                        // Set to 0 before calculating checksum
    iph->saddr = net->saddr.sin_addr.s_addr; 	// Spoof the source ip address
    iph->daddr = net->daddr.sin_addr.s_addr;

    // Ip checksum
    iph->check = csum((unsigned short *)(net->datagram + sizeof(struct ether_header)), sizeof(struct iphdr)/2);

    // UDP header
    udph->source = net->saddr.sin_port;         // Source port
    udph->dest = net->daddr.sin_port;           // Destination port
    udph->len = htons(8 + bufsize);	            // TCP header size
    udph->check = 0;	                        // Leave checksum 0 now, filled later by pseudo header

    // Now the UDP checksum using the pseudo header
    psh.source_address = net->saddr.sin_addr.s_addr;
    psh.dest_address = net->daddr.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udphdr) + bufsize);

    int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + bufsize;
    memcpy(net->pseudogram, &psh, sizeof (struct pseudo_header));
    memcpy(net->pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + bufsize);
    udph->check = csum( (unsigned short*)net->pseudogram, psize);

    // Send eth frame
    if ((res = send(net->sd, net->datagram, txlen, 0)) == txlen)
    {
        res = bufsize;
    }

    return res;
}


#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/udp.h>
#include <netinet/ip.h>

#include <rxs_streamer/rxs_sender_raw.h>

#define TRACE_TAG   "rxs_sender"


/*
	96 bit (12 bytes) pseudo header needed for udp header checksum calculation
*/
struct pseudo_header
{
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
};


/** Generic checksum calculation function */
static unsigned short csum(unsigned short *ptr,int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while(nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }
    if(nbytes == 1)
    {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum>>16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}



int rxs_sender_raw_init(rxs_sender_raw* net, struct sockaddr_in *saddr, struct sockaddr_in *daddr)
{
    memset(net, 0, sizeof(rxs_sender_raw));

    if ((net->sd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        TRACE_ERROR("Create RAW UDP socket failed");
        return -1;
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
    int res;
    uint8_t *data, *pseudogram;
    struct pseudo_header psh;
    struct iphdr *iph = (struct iphdr *)net->datagram;  // IP header
    struct udphdr *udph = (struct udphdr *)(net->datagram + sizeof(struct ip)); // UDP header

    // Zero out the packet buffer
    memset(net->datagram, 0, sizeof(net->datagram));

    // Data part
    if (sizeof(struct iphdr) + sizeof(struct udphdr) + bufsize > CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE)
    {
        TRACE_ERROR("RAW UDP packet size ovwerflow max size %d", CFG_RXS_SENDER_RAW_DATAGRAM_BUFSIZE);
        return -1;
    }

    // Copy data payload
    data = net->datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(data, buf, bufsize);

    // Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr) + bufsize;
    iph->id = htonl(54321);	                  // Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;		                        // Set to 0 before calculating checksum
    iph->saddr = net->saddr.sin_addr.s_addr; 	// Spoof the source ip address
    iph->daddr = net->daddr.sin_addr.s_addr;

    // Ip checksum
    iph->check = csum ((unsigned short *)net->datagram, iph->tot_len);

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
    pseudogram = malloc(psize);

    memcpy(pseudogram, (char*) &psh, sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header), udph, sizeof(struct udphdr) + bufsize);

    udph->check = csum( (unsigned short*) pseudogram, psize);

    free(pseudogram);

    // Send the packet
    if ((res = sendto(net->sd, net->datagram, iph->tot_len, 0, (struct sockaddr *) &net->daddr, sizeof(net->daddr))) == iph->tot_len)
        res = bufsize;

    return res;
}


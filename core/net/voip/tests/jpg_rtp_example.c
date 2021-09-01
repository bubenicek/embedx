/*
 * RTP data header from RFC1889
 */
typedef struct {
        unsigned int version:2;   /* protocol version */
        unsigned int p:1;         /* padding flag */
        unsigned int x:1;         /* header extension flag */
        unsigned int cc:4;        /* CSRC count */
        unsigned int m:1;         /* marker bit */
        unsigned int pt:7;        /* payload type */
        u_int16 seq;              /* sequence number */
        u_int32 ts;               /* timestamp */
        u_int32 ssrc;             /* synchronization source */
        u_int32 csrc[1];          /* optional CSRC list */
} rtp_hdr_t;

#define RTP_HDR_SZ 12

/* The following definition is from RFC1890 */
#define RTP_PT_JPEG             26

struct jpeghdr {
        unsigned int tspec:8;   /* type-specific field */
        unsigned int off:24;    /* fragment byte offset */
        u_int8 type;            /* id of jpeg decoder params */
        u_int8 q;               /* quantization factor (or table id) */
        u_int8 width;           /* frame width in 8 pixel blocks */
        u_int8 height;          /* frame height in 8 pixel blocks */
};

struct jpeghdr_rst {
        u_int16 dri;
        unsigned int f:1;
        unsigned int l:1;
        unsigned int count:14;
};



struct jpeghdr_qtable {
        u_int8  mbz;
        u_int8  precision;
        u_int16 length;
};

#define RTP_JPEG_RESTART           0x40

/* Procedure SendFrame:
 *
 *  Arguments:
 *    start_seq: The sequence number for the first packet of the current
 *               frame.
 *    ts: RTP timestamp for the current frame
 *    ssrc: RTP SSRC value
 *    jpeg_data: Huffman encoded JPEG scan data
 *    len: Length of the JPEG scan data
 *    type: The value the RTP/JPEG type field should be set to
 *    typespec: The value the RTP/JPEG type-specific field should be set
 *              to
 *    width: The width in pixels of the JPEG image
 *    height: The height in pixels of the JPEG image
 *    dri: The number of MCUs between restart markers (or 0 if there
 *         are no restart markers in the data
 *    q: The Q factor of the data, to be specified using the Independent
 *       JPEG group's algorithm if 1 <= q <= 99, specified explicitly
 *       with lqt and cqt if q >= 128, or undefined otherwise.
 *    lqt: The quantization table for the luminance channel if q >= 128
 *    cqt: The quantization table for the chrominance channels if
 *         q >= 128
 *
 *  Return value:
 *    the sequence number to be sent for the first packet of the next
 *    frame.
 *
 * The following are assumed to be defined:
 *
 * PACKET_SIZE                         - The size of the outgoing packet
 * send_packet(u_int8 *data, int len)  - Sends the packet to the network
 */

u_int16 SendFrame(u_int16 start_seq, u_int32 ts, u_int32 ssrc,
                   u_int8 *jpeg_data, int len, u_int8 type,
                   u_int8 typespec, int width, int height, int dri,
                   u_int8 q, u_int8 *lqt, u_int8 *cqt) {
        rtp_hdr_t rtphdr;
        struct jpeghdr jpghdr;
        struct jpeghdr_rst rsthdr;
        struct jpeghdr_qtable qtblhdr;
        u_int8 packet_buf[PACKET_SIZE];
        u_int8 *ptr;
        int bytes_left = len;
        int seq = start_seq;
        int pkt_len, data_len;

        /* Initialize RTP header
         */
        rtphdr.version = 2;
        rtphdr.p = 0;
        rtphdr.x = 0;
        rtphdr.cc = 0;
        rtphdr.m = 0;
        rtphdr.pt = RTP_PT_JPEG;
        rtphdr.seq = start_seq;
        rtphdr.ts = ts;
        rtphdr.ssrc = ssrc;

        /* Initialize JPEG header
         */
        jpghdr.tspec = typespec;
        jpghdr.off = 0;
        jpghdr.type = type | ((dri != 0) ? RTP_JPEG_RESTART : 0);
        jpghdr.q = q;
        jpghdr.width = width / 8;
        jpghdr.height = height / 8;

        /* Initialize DRI header
         */
        if (dri != 0) {
                rsthdr.dri = dri;
                rsthdr.f = 1;        /* This code does not align RIs */
                rsthdr.l = 1;
                rsthdr.count = 0x3fff;
        }

        /* Initialize quantization table header
         */
        if (q >= 128) {
                qtblhdr.mbz = 0;
                qtblhdr.precision = 0; /* This code uses 8 bit tables only */
                qtblhdr.length = 128;  /* 2 64-byte tables */
        }

        while (bytes_left > 0) {
                ptr = packet_buf + RTP_HDR_SZ;
                memcpy(ptr, &jpghdr, sizeof(jpghdr));

                ptr += sizeof(jpghdr);

                if (dri != 0) {
                        memcpy(ptr, &rsthdr, sizeof(rsthdr));
                        ptr += sizeof(rsthdr);
                }

                if (q >= 128 && jpghdr.off == 0) {
                        memcpy(ptr, &qtblhdr, sizeof(qtblhdr));
                        ptr += sizeof(qtblhdr);
                        memcpy(ptr, lqt, 64);
                        ptr += 64;
                        memcpy(ptr, cqt, 64);
                        ptr += 64;
                }

                data_len = PACKET_SIZE - (ptr - packet_buf);
                if (data_len >= bytes_left) {
                        data_len = bytes_left;
                        rtphdr.m = 1;
                }

                memcpy(packet_buf, &rtphdr, RTP_HDR_SZ);
                memcpy(ptr, jpeg_data + jpghdr.off, data_len);

                send_packet(packet_buf, (ptr - packet_buf) + data_len);

                jpghdr.off += data_len;
                bytes_left -= data_len;
                rtphdr.seq++;
        }
        return rtphdr.seq;
}




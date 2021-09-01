
#ifndef __VIDEO_TRANSCODER_H
#define __VIDEO_TRANSCODER_H

#define VIDEO_TRANSCODER_PORT       1974

/** Packet flags */
typedef enum
{
    VIDEO_TRANSCODING_FLAG_START_FRAME = 0x1,           // Start of image frame

} video_transcoding_flag_t;


/** Prenosovy packet pro prekodovani obrazu */
typedef struct
{
   uint16_t seqn;               // Sekvencni cislo
   uint32_t sip_uid;            // User ID, SIP uid cislo volaneho
   uint8_t flags;               // priznaky jestli se jedna o zacatek frame
   uint32_t dst_ip;             // Cilova adresa kam preposlat po prekodovani
   uint16_t dst_port;           // Cilovy port kam preposlat po prekodovani
   uint16_t payload_length;     // Velikost dat kodovanych v src_formatu
   uint8_t payload[0];          // Data kodovana v src formatu

} __PACKED__ video_transcoding_packet_t;


#endif   // __VIDEO_TRANSCODER_H

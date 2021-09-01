
#ifndef __APP_HEADER_H
#define __APP_HEADER_H


#define APP_HEADER_MAGIC               0x4341575A     // ZWAC ascii
#define APP_VERSION(_major, _minor)    ((_major << 8) | _minor)
#define APP_VERSION_MAJOR(_version)    ((_version >> 8) & 0xFF)
#define APP_VERSION_MINOR(_version)    (_version & 0xFF)


/** Application header */
typedef struct app_header
{
   uint32_t magic;
   uint16_t hw_version;
   uint16_t fw_version;
   uint32_t fw_size;
   uint32_t fw_crc;       // FW CRC included fw_crc = 0x0

}  __attribute__ ((packed)) app_header_t;


#endif // __APP_HEADER_H
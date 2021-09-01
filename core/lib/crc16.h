#ifndef __CRC16_H
#define __CRC16_H

#include <stdint.h>

#if __AVR__
 #include <avr/pgmspace.h>
#else
 #define PROGMEM
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
extern const uint16_t PROGMEM crc16_table[256];

static inline uint16_t crc16_byte(uint16_t crc, const uint8_t data)
{
#if __AVR__
   return (crc >> 8) ^ pgm_read_word(&crc16_table[(crc ^ data) & 0xff]);
#else
   return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
#endif
}


/**
 * crc16 - compute the CRC-16 for the data buffer
 * @crc:	previous CRC value
 * @buffer:	data pointer
 * @len:	number of bytes in the buffer
 *
 * Returns the updated CRC value.
 */
uint16_t crc16(uint16_t crc, uint8_t *buffer, int len);


#ifdef __cplusplus
}
#endif


#endif /* __CRC16_H */


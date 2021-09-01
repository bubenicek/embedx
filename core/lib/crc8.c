
#include "system.h"

uint8_t crc8(uint8_t data, uint8_t crc)
{
   int i;

   for (i = 0; i < 8; i++)
   {
      const uint8_t polynom = (crc & 0x80) ? 0x9B : 0;

      crc <<= 1;
      if (data & 0x80)
         crc |= 0x01;

      crc ^= polynom;
      data <<= 1;
   }
   
   return crc;
}

uint8_t crc8_buf(uint8_t *buf, int len)
{
  uint8_t crc = 0;
  
  while(len > 0)
  {
    crc = crc8(*buf++, crc);
    len--;
  }
    
  return crc;
}

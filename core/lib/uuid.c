#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "system.h"
#include "uuid.h"


//                                           12345678 9012 3456 7890 123456789012
// Returns a 36-character string in the form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
// where "X" is an "upper-case" hexadecimal digit [0-9A-F].
// Use the LCase function if you want lower-case letters.
char *uuid128_make(uint8_t *data, int datalen, char *uuid, int uuidlen)
{
   int ix;
   int len = 0;
   char strHex[2*UUID_NBYTES+1];

   if (datalen < UUID_NBYTES || uuidlen < UUID_STRLEN)
      return NULL;

   // 2. Adjust certain bits according to RFC 4122 section 4.4.
   // This just means do the following
   // (a) set the high nibble of the 7th byte equal to 4 and
   // (b) set the two most significant bits of the 9th byte to 10'B,
   //     so the high nibble will be one of {8,9,A,B}.
   //data[6] = 0x40 | (data[6] & 0xf);
   //data[8] = 0x80 | (data[8] & 0x3f);

   // 3. Convert the adjusted bytes to hex values
   for (ix = 0; ix < UUID_NBYTES; ix++)
      len += snprintf(&strHex[len], sizeof(strHex) - len, "%2.2X", data[ix]);

   // 4. Add four hyphen '-' characters
   memset(uuid, 0, uuidlen);
   strncpy(uuid, strHex, 8);
   strcat(uuid, "-");
   strncat(uuid, &strHex[8], 4);
   strcat(uuid, "-");
   strncat(uuid, &strHex[12], 4);
   strcat(uuid, "-");
   strncat(uuid, &strHex[16], 4);
   strcat(uuid, "-");
   strncat(uuid, &strHex[20], 12);

   // Return the UUID string
   return uuid;
}

/** Generate random uuid */
char *uuid128_make_random(char *uuid, int uuidlen)
{
   int ix;
   uint8_t data[UUID_NBYTES];

   // 1. Generate 16 random bytes = 128 bits
   for (ix = 0; ix < UUID_NBYTES; ix++)
      data[ix] = rand() % 256;

   return uuid128_make(data, UUID_NBYTES, uuid, uuidlen);
}

char *uuid128_make_u32(uint32_t u32, char *uuid, int uuidlen)
{
   uint8_t data[UUID_NBYTES];

   memset(data, 0, sizeof(data));
   data[0] = u32 >> 24;
   data[1] = u32 >> 16;
   data[2] = u32 >> 8;
   data[3] = u32 & 0xFF;

   return uuid128_make(data, UUID_NBYTES, uuid, uuidlen);
}

int uuid128_parse(const char *in, uint8_t *data, int datalen)
{
   int i, j = 0;
   const char *cp;
   char buf[3];

   if (strlen(in) != 36 || datalen < UUID_NBYTES)
      return -1;

   for (i = 0, cp = in; i <= 36; i++, cp++)
   {
      if ((i == 8) || (i == 13) || (i == 18) || (i == 23))
      {
         if (*cp == '-')
            continue;
         else
            return -1;
      }
      if (i == 36)
         if (*cp == 0)
            continue;

      if (!isxdigit((int)*cp))
         return -1;

      buf[j++] = *cp;
      if (j == 2)
      {
         buf[j] = '\0';
         *data++ = strtoul(buf, NULL, 16);
         j = 0;
      }
   }

   return 0;
}

int uuid128_parse_u32(const char *in, uint32_t *u32)
{
   uint8_t data[UUID_NBYTES];

   if (uuid128_parse(in, data, UUID_NBYTES) != 0)
      return -1;

   *u32 = ((uint32_t)data[0]) << 24;
   *u32 |= ((uint32_t)data[1]) << 16;
   *u32 |= data[2] << 8;
   *u32 |= data[3] & 0xFF;

   return 0;
}

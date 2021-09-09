
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/uuid.h"

/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint32_t *)0x1FFFF7E8)


/** Get unique board ID */
uint32_t hal_get_board_uuid32(void)
{
   return (STM32_UUID[2] & 0xFFFF) << 16 | (STM32_UUID[1] & 0xFFFF);
}

/** Generate uuid board string */
char *hal_get_board_uuid(char *uuid, int bufsize)
{
   uint8_t data[UUID_NBYTES];

   memcpy(data, &STM32_UUID[0], 4);
   memcpy(&data[4], &STM32_UUID[1], 4);
   memcpy(&data[8], &STM32_UUID[2], 4);
   memcpy(&data[12], &STM32_UUID[0], 4);

   return uuid128_make(data, UUID_NBYTES, uuid, bufsize);
}
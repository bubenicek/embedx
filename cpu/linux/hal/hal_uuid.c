
#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/uuid.h"
#include "lib/sha3.h"


uint32_t STM32_UUID[3];   // 96 bit unique ID


/** Get unique board ID */
uint32_t hal_get_board_uuid32(void)
{
    uint32_t hash;

    sha3(STM32_UUID, 12, &hash, 4);

    return hash;
}

/** Generate uuid board string */
char *hal_get_board_uuid(char *uuid, int bufsize)
{
   uint8_t data[UUID_NBYTES];

   memcpy(data, &STM32_UUID[0], 4);
   memcpy(&data[4], &STM32_UUID[1], 4);
   memcpy(&data[8], &STM32_UUID[2], 4);
   memcpy(&data[12], &STM32_UUID[3], 4);

   return uuid128_make(data, UUID_NBYTES, uuid, bufsize);
}
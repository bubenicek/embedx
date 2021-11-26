#include "system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/uuid.h"
#include "lib/sha3.h"

/**
 * The STM32 factory-programmed UUID memory.
 * Three values of 32 bits each starting at this address
 * Use like this: STM32_UUID[0], STM32_UUID[1], STM32_UUID[2]
 */
#define STM32_UUID ((uint32_t *)0x1FFF7A10)


/** Get unique board ID */
uint32_t hal_get_board_uuid32(void)
{
    uint32_t hash;

    // Compute sha3 32bit hash from 96bit uuid
    sha3(STM32_UUID, 12, &hash, 4);

    return hash;
}

/** Generate uuid board string */
char *hal_get_board_uuid(char *uuid, int bufsize)
{
    uint8_t data[UUID_NBYTES];

    // Compute sha3 hash from 96bit uuid
    sha3(STM32_UUID, 12, data, sizeof(data));

   return uuid128_make(data, UUID_NBYTES, uuid, bufsize);
}
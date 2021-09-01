
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "zw_config.h"

#include "dev/mem/m95256.h"
#include "dev/mem/m25pe10.h"

/** Erase ZW external eeprom */
int zw_eeprom_erase(void)
{
   // Hold Zwave chip in reset
   hal_gpio_set(GPIO_ZW_RESET, 0);

   // Erase Zwave external eeprom
#if defined(ZW_EEPROM_TYPE) && (ZW_EEPROM_TYPE == ZW_EEPROM_TYPE_M95256)
   if (m95256_init() != 0)
      return -1;

   return m95256_erase_chip();
#elif defined(ZW_EEPROM_TYPE) && (ZW_EEPROM_TYPE == ZW_EEPROM_TYPE_M25PE10)
   if (m25pe10_init() != 0)
      return -1;

   return m25pe10_erase_chip();
#else
#error Not defined ZW_EEPROM_TYPE
#endif
}

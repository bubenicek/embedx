
#ifndef __STM32F4XX_SWO_H
#define __STM32F4XX_SWO_H

void swo_init(uint32_t SWOSpeed);
uint32_t swo_putchar(uint8_t ch);


#endif // __STM32F4XX_SWO_H
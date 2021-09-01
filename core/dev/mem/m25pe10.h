

#ifndef __M25P10_H
#define __M25P10_H

int m25pe10_init(void);
int m25pe10_read(uint16_t addr, uint8_t *buf, int count);
int m25pe10_write(uint16_t addr, uint8_t *buf, int count);
int m25pe10_erase_chip(void);

#endif 
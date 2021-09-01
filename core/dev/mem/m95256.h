

#ifndef __M95256_H
#define __M95256_H

int m95256_init(void);
int m95256_read(uint16_t addr, uint8_t *buf, int count);
int m95256_write(uint16_t addr, uint8_t *buf, int count);
int m95256_erase_chip(void);

#endif // __M95256_H
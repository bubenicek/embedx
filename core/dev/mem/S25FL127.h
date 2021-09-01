
#ifndef __S25FL127_H
#define __S25FL127_H

#define S25FL127_PAGE_SIZE                256
#define S25FL127_SECTOR_SIZE              65536
#define S25FL127_NUM_SECTORS              128 //256

int s25fl127_init(void);

int s25fl127_write_page(uint32_t addr, uint8_t *pagebuf);
int s25fl127_read_page(uint32_t addr, uint8_t *pagebuf);

int s25fl127_read(uint32_t addr, void *buf, int nbytes);
int s25fl127_write(uint32_t addr, void *buf, int nbytes);

int s25fl127_erase_sector(uint32_t sector);
int s25fl127_erase_chip(void);


#endif   // _S25FL127_H

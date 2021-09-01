
#ifndef __at45db_h
#define __at45db_h

#define AT45DB_NUM_BLOCKS           256
#define AT45DB_PAGE_SIZE		      264
#define AT45DB_PAGES_PER_BLOCK      8
#define AT45DB_NUM_PAGES            (AT45DB_NUM_BLOCKS * AT45DB_PAGES_PER_BLOCK)

int at45db_init(void);

int at45db_write_page(uint16_t page, uint8_t *buffer);
int at45db_write_page_offset(uint16_t p_addr, uint16_t b_addr, uint8_t *buffer, uint16_t bytes);

int at45db_read_page(uint16_t page, uint8_t *buffer);
int at45db_read_page_offset(uint16_t p_addr, uint16_t b_addr, uint8_t *buffer, uint16_t bytes);

int at45db_erase_chip(void);

int at45db_erase_block(uint16_t addr);

int at45db_erase_page(uint16_t addr);

#endif

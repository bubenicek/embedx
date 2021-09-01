
#ifndef __HAL_FLASH_H
#define __HAL_FLASH_H

int hal_flash_init(void);
int hal_flash_deinit(void);
int hal_flash_erase(uint32_t start_addr, int length);
int hal_flash_read(uint32_t addr, void *buf, int bufsize);
int hal_flash_write(uint32_t addr, const void *buf, int bufsize);

#endif // __HAL_FLASH_H
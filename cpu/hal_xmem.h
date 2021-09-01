
#ifndef HAL_XMEM_H
#define HAL_XMEM_H

int hal_xmem_init(void);
int hal_xmem_read(uint32_t offset, void *buf, int nbytes);
int hal_xmem_write(uint32_t offset, const void *buf, int nbytes);
int hal_xmem_erase(uint32_t offset, uint32_t nbytes);

#endif /* HAL_XMEM_H */

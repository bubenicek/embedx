
#ifndef __HAL_DMX_H
#define __HAL_DMX_H

typedef enum
{
   HAL_DMX0,
   HAL_DMX1,
   HAL_DMX2,
   HAL_DMX3,

} hal_dmx_t;


/** Initialize DMX device */
int hal_dmx_init(hal_dmx_t dmx, uint32_t dmx_break_delay);

/** Deinitialize DMX device */
int hal_dmx_deinit(hal_dmx_t dmx);

/** Write buffer */
int hal_dmx_write(hal_dmx_t dmx, uint8_t *buf, int count);

#endif // __HAL_DMX_H


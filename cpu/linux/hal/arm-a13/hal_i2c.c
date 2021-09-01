
#include "system.h"

TRACE_TAG(hal_i2c);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif


int hal_i2c_init(hal_i2c_t dev)
{
    TRACE("I2C initialized");
    return 0;
}

int hal_i2c_deinit(hal_i2c_t dev)
{
    return 0;
}

int hal_i2c_write_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg, uint8_t data)
{
    return 0;
}

int hal_i2c_read_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg, uint8_t *data)
{
    return 0;
}

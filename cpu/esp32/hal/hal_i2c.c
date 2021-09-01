
#include "system.h"

#include "twi.h"
#include "wiring.h"

TRACE_TAG(hal_i2c);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_HAL_I2C_DEF
#define CFG_HAL_I2C_DEF {}
#endif

static const hal_i2c_def_t i2c_def[] = CFG_HAL_I2C_DEF;
#define NUM_I2C  (sizeof(hal_i2c_def_t) / sizeof(hal_i2c_def_t))


int hal_i2c_init(hal_i2c_t dev)
{
    ASSERT(dev < NUM_I2C);

    twi_init(i2c_def[dev].sda_pin, i2c_def[dev].scl_pin);

    return 0;
}

int hal_i2c_deinit(hal_i2c_t dev)
{
    ASSERT(dev < NUM_I2C);

    return 0;
}

int hal_i2c_write_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg, uint8_t data)
{
    int ret;
    uint8_t buf[] = {reg, data};

    ASSERT(dev < NUM_I2C);

    __disable_irq();
    ret = twi_writeTo(slave_addr, buf, 2, true);
    __enable_irq();

    if (ret != 0)
    {
        TRACE_ERROR("i2c write [%02x]=%02x failed\n", reg, data);
        return -1;
    }

    return 0;
}

int hal_i2c_read_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg, uint8_t *data)
{
    int ret;

    ASSERT(dev < NUM_I2C);

    __disable_irq();
    if (twi_writeTo(slave_addr, &reg, 1, true) == 0)
    {
        ret = twi_readFrom(slave_addr, data, 1, true);
    }   
    __enable_irq();

    if (ret != 0)
    {
        TRACE_ERROR("I2C read [%02x] failed rc=%d\n", reg, ret);
        return -1;
    }

    return 0;
}

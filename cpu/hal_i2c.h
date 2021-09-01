
#ifndef __HAL_I2C_H
#define __HAL_I2C_H

typedef enum
{
   HAL_I2C0,
   HAL_I2C1,
   HAL_I2C2,
   HAL_I2C3,

} hal_i2c_t;


int hal_i2c_init(hal_i2c_t dev);

int hal_i2c_deinit(hal_i2c_t dev);

int hal_i2c_write_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg_addr, uint8_t value);

int hal_i2c_read_register(hal_i2c_t dev, uint8_t slave_addr, uint8_t reg_addr, uint8_t *value);


#endif // __HAL_I2C_H

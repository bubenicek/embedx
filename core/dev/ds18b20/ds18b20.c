
#include "system.h"
#include "ds18b20.h"

TRACE_TAG(ds18b20);

int ds_init(ds_t *ds, hal_gpio_t gpio)
{   
    ds->gpio = gpio;
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_OUTPUT);
    TRACE("Init, gpio: %d", ds->gpio);

    return 0;    
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesnt then it is broken or shorted
// and we return;
static int ds_reset(ds_t *ds)
{   
    //    IO_REG_TYPE mask = bitmask;
    //    volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
    uint8_t retries = 125;

    // noInterrupts();
    // DIRECT_MODE_INPUT(reg, mask);
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_INPUT);
    // interrupts();
    // wait until the wire is high just in case
    do {
        if (retries == 0) 
            return -1;

        hal_delay_us(2);

    } while (!hal_gpio_get(ds->gpio));

    // noInterrupts();
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set(ds->gpio, 0);
    // DIRECT_WRITE_LOW(reg, mask);
    // DIRECT_MODE_OUTPUT(reg, mask);    // drive output low
    // interrupts();
    hal_delay_us(480);

    // noInterrupts();
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_INPUT);
    // DIRECT_MODE_INPUT(reg, mask);    // allow it to float
    hal_delay_us(70);
    // r = !DIRECT_READ(reg, mask);
    //r = !GPIO_INPUT_GET( gpioPin );
    // interrupts();
    hal_delay_us(410);

    return 0;
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline void write_bit(ds_t *ds, int v)
{   
    // IO_REG_TYPE mask=bitmask;
    //    volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
    hal_gpio_set(ds->gpio, 0);

    if (v) 
    {
        // noInterrupts();
        //    DIRECT_WRITE_LOW(reg, mask);
        //    DIRECT_MODE_OUTPUT(reg, mask);    // drive output low
        hal_delay_us(10);
        hal_gpio_set(ds->gpio, 1);
        // DIRECT_WRITE_HIGH(reg, mask);    // drive output high
        // interrupts();
        hal_delay_us(55);
    } 
    else 
    {
        // noInterrupts();
        //    DIRECT_WRITE_LOW(reg, mask);
        //    DIRECT_MODE_OUTPUT(reg, mask);    // drive output low
        hal_delay_us(65);
        hal_gpio_set(ds->gpio, 1);
        //    DIRECT_WRITE_HIGH(reg, mask);    // drive output high
        //        interrupts();
        hal_delay_us(5);
    }
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline int read_bit(ds_t *ds)
{
    //IO_REG_TYPE mask=bitmask;
    //volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
    int r = 0;
 
    // noInterrupts();
    // DIRECT_MODE_OUTPUT(reg, mask);
    // DIRECT_WRITE_LOW(reg, mask);
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set(ds->gpio, 0);    
    hal_delay_us(3);

    // DIRECT_MODE_INPUT(reg, mask);    // let pin float, pull up will raise
    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_INPUT);
    hal_delay_us(10);
    
    // r = DIRECT_READ(reg, mask);
    r = hal_gpio_get(ds->gpio);
    // interrupts();
    hal_delay_us(53);
    
    return r;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set power to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
static int ds_write(ds_t *ds, uint8_t v, int power ) 
{
    uint8_t bitMask;

    DISABLE_INTERRUPTS();

    hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_OUTPUT);
 
    for (bitMask = 0x01; bitMask; bitMask <<= 1) 
    {
        write_bit(ds, (bitMask & v) ? 1:0);
    }

    if (!power) 
    {
        // noInterrupts();
        // DIRECT_MODE_INPUT(baseReg, bitmask);
        // DIRECT_WRITE_LOW(baseReg, bitmask);
        // interrupts();
        hal_gpio_set(ds->gpio, 0);
        hal_gpio_configure(ds->gpio, HAL_GPIO_MODE_INPUT);
    }

    ENABLE_INTERRUPTS();

    return 0;
}

//
// Read a byte
//
static uint8_t ds_read(ds_t *ds) 
{
    uint8_t bitMask;
    uint8_t r = 0;

    DISABLE_INTERRUPTS();

    for (bitMask = 0x01; bitMask; bitMask <<= 1) 
    {
        if (read_bit(ds)) 
            r |= bitMask;
    }

    ENABLE_INTERRUPTS();

    return r;
}

// Start measurement
int ds_start(ds_t *ds)
{
    ds_reset(ds);
    ds_write(ds, 0xcc, 1);
    ds_write(ds, 0x44, 1);

    return 0;
}


int ds_read_temp(ds_t *ds, float *value)
{
	ds_reset(ds);
    ds_write(ds, 0xcc,1);
    ds_write(ds, 0xbe,1);

	uint8_t lsb = (int)ds_read(ds);
    uint8_t msb = (int)ds_read(ds);

    // Check available temp sensor
    if (lsb == 0xFF && msb == 0xFF)
        return -1;

    uint8_t sign = msb & 0x80;
    int16_t temp = (msb << 8) + lsb;

    if (sign)
        temp = ((temp ^ 0xffff) + 1) * -1;

    *value = temp / 16.0;

    return 0;
}

int ds_read_temp_str(ds_t *ds, char *buf, int bufsize)
{
    int HighByte, LowByte, TReading, SignBit, Whole, Fract;

	ds_reset(ds);
    ds_write(ds, 0xcc,1);
    ds_write(ds, 0xbe,1);

	LowByte = (int)ds_read(ds);
    HighByte = (int)ds_read(ds);

    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if (SignBit) // negative
  	    TReading = (TReading ^ 0xffff) + 1; // 2's comp

	Whole = TReading >> 4;  // separate off the whole and fractional portions
    Fract = (TReading & 0xf) * 100 / 16;

	if (SignBit)
	    sprintf(buf, "-%d.%d", Whole, Fract < 10 ? 0 : Fract);
	else
	    sprintf(buf, "%d.%d", Whole, Fract < 10 ? 0 : Fract);

    return 0;
}

#ifndef __DS18B20_H__
#define __DS18B20_H__

#define DS1820_WRITE_SCRATCHPAD 		0x4E
#define DS1820_READ_SCRATCHPAD      0xBE
#define DS1820_COPY_SCRATCHPAD 		0x48
#define DS1820_READ_EEPROM 			0xB8
#define DS1820_READ_PWRSUPPLY 		0xB4
#define DS1820_SEARCHROM 				0xF0
#define DS1820_SKIP_ROM             0xCC
#define DS1820_READROM 					0x33
#define DS1820_MATCHROM 				0x55
#define DS1820_ALARMSEARCH 			0xEC
#define DS1820_CONVERT_T            0x44

typedef struct
{
   hal_gpio_t gpio;

} ds_t;

int ds_init(ds_t *ds, hal_gpio_t gpio);
int ds_start(ds_t *ds);
int ds_read_temp(ds_t *ds, float *value);
int ds_read_temp_str(ds_t *ds, char *buf, int bufsize);

#endif   // __DS18B20_H__

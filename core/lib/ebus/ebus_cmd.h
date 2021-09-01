
#ifndef __EBUS_CMD_H
#define __EBUS_CMD_H


#define EBUS_CMD_MAX_DATALEN      16

/** EBUS command value type */
typedef enum
{
    EBUS_VALUE_NULL,
    EBUS_VALUE_BYTE,
    EBUS_VALUE_DATA1C,
    EBUS_VALUE_DATA2C,

} ebus_value_type_t;


typedef struct
{
    ebus_value_type_t type;
    union
    {
        uint8_t b;
        float f;

    } v;

} ebus_value_t;


/** EBUS command definition */
typedef struct
{
    const char *name;
    uint16_t cmd[2];
    uint8_t datalen;
    uint8_t data[EBUS_CMD_MAX_DATALEN];
    uint8_t dst;

    struct
    {
        ebus_value_type_t type;
        uint8_t pos;

    } param;

    struct
    {
        ebus_value_type_t type;
        uint8_t pos;

    } value;

} ebus_cmd_def_t;


#define ebus_value_set_null(_obj) (_obj)->type = EBUS_VALUE_NULL
#define ebus_value_set_byte(_obj, _value) {(_obj)->type = EBUS_VALUE_BYTE;(_obj)->v.b = _value;}
#define ebus_value_set_data1c(_obj, _value) {(_obj)->type = EBUS_VALUE_DATA1C;(_obj)->v.f = _value;}
#define ebus_value_set_data2c(_obj, _value) {(_obj)->type = EBUS_VALUE_DATA2C;(_obj)->v.f = _value;}

const char *ebus_value_str(const ebus_value_t *value);
uint8_t ebus_value_byte(const ebus_value_t *value);
float ebus_value_data2c(const ebus_value_t *value);
float ebus_value_data1c(const ebus_value_t *value);


#endif   // __EBUS_CMD_H

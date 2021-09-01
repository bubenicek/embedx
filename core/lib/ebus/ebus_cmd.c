
#include "system.h"
#include "ebus.h"

#define TRACE_TAG "ebus"
#if !ENABLE_TRACE_EBUS
#include "trace_undef.h"
#endif

#define ECMD(c1,c2) {c1,c2}
#define EDATA(_num, ...) _num, { __VA_ARGS__ }

#define EPARAM_NULL          {0}
#define EPARAM_BYTE(pos)     {EBUS_VALUE_BYTE,pos}
#define EPARAM_DATA1C(pos)   {EBUS_VALUE_DATA1C,pos}
#define EPARAM_DATA2C(pos)   {EBUS_VALUE_DATA2C,pos}

#define EVALUE_NULL          {0}
#define EVALUE_BYTE(pos)     {EBUS_VALUE_BYTE,pos}
#define EVALUE_DATA1C(pos)   {EBUS_VALUE_DATA1C,pos}
#define EVALUE_DATA2C(pos)   {EBUS_VALUE_DATA2C,pos}

// FF 15 B5 09 03 0D 44 00 80 00 01 5C C7 00 AA

const ebus_cmd_def_t ebus_cmds[] =
{
// Vaillant VRC430
    {"vrc430.temp_room_disp",          ECMD(0xB5,0x09), EDATA(3,0x0D,0x80,0x00),      0x15, EPARAM_NULL, EVALUE_DATA2C(0)},  // Displayed room temperature (°C)
    {"vrc430.temp_outside",            ECMD(0xB5,0x09), EDATA(3,0x0D,0x62,0x00),      0x15, EPARAM_NULL, EVALUE_DATA2C(0)},  // Outside temperature (°C)
    // Tepla voda
    {"vrc430.program_dhw_circuit",     ECMD(0xB5,0x09), EDATA(3,0x0D,0x42,0x00),      0x15, EPARAM_NULL, EVALUE_BYTE(0)},    // DHW Operation mode ["0":"Off", "1":"Manual", "2":"Auto"]
    {"vrc430.set_program_dhw_circuit", ECMD(0xB5,0x09), EDATA(4,0x0E,0x42,0x00,0x00), 0x15, EPARAM_BYTE(3), EVALUE_NULL},    // Set DHW Operation mode
    {"vrc430.temp_d_dhw",              ECMD(0xB5,0x09), EDATA(3,0x0D,0x44,0x00),      0x15, EPARAM_NULL, EVALUE_DATA1C(0)},  // Domestic Hot Water Setpoint
    {"vrc430.temp_d_actual_dhw",       ECMD(0xB5,0x09), EDATA(3,0x0D,0x86,0x00),      0x15, EPARAM_NULL, EVALUE_DATA1C(0)},  // Domestic Hot Water actual desired temperature

    {NULL}
};


const char *ebus_value_str(const ebus_value_t *value)
{
    static char txt[32];

    switch(value->type)
    {
        case EBUS_VALUE_BYTE:
            snprintf(txt, sizeof(txt), "%d", value->v.b);
            break;

        case EBUS_VALUE_DATA1C:
        case EBUS_VALUE_DATA2C:
            snprintf(txt, sizeof(txt), "%2.2f", value->v.f);
            break;

        default:
            strcpy(txt, "null");
    }

    return txt;
}

uint8_t ebus_value_byte(const ebus_value_t *value)
{
    uint8_t b;

    switch(value->type)
    {
        case EBUS_VALUE_BYTE:
            b = value->v.b;
            break;

        case EBUS_VALUE_DATA1C:
        case EBUS_VALUE_DATA2C:
            b = value->v.f;
            break;

        default:
            b = 0;
    }

    return b;
}

float ebus_value_data2c(const ebus_value_t *value)
{
    float v;

    switch(value->type)
    {
        case EBUS_VALUE_BYTE:
            v = value->v.b;
            break;

        case EBUS_VALUE_DATA1C:
        case EBUS_VALUE_DATA2C:
            v = value->v.f;
            break;

        default:
            v = 0;
    }

    return v;
}

float ebus_value_data1c(const ebus_value_t *value)
{
    float v;

    switch(value->type)
    {
        case EBUS_VALUE_BYTE:
            v = value->v.b;
            break;

        case EBUS_VALUE_DATA1C:
        case EBUS_VALUE_DATA2C:
            v = value->v.f;
            break;

        default:
            v = 0;
    }

    return v;
}

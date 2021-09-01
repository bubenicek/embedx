
#include "system.h"
#include "axp20x.h"

TRACE_TAG(axp20x);
#if !ENABLE_TRACE_AXP20X
#undef TRACE
#define TRACE(...)
#endif

// Locals:
static const uint8_t startupParams[] = {
    0b00000000,
    0b01000000,
    0b10000000,
    0b11000000
};

static const uint8_t longPressParams[] = {
    0b00000000,
    0b00010000,
    0b00100000,
    0b00110000
};

static const uint8_t shutdownParams[] = {
    0b00000000,
    0b00000001,
    0b00000010,
    0b00000011
};

static const uint8_t targetVolParams[] = {
    0b00000000,
    0b00100000,
    0b01000000,
    0b01100000
};

// Power Output Control register
static uint8_t _outputReg;

static bool init = false;
static uint8_t i2c_addr;
static hal_i2c_t i2c_dev;
static uint8_t irq[5];
static uint8_t chip_id;
static uint8_t gpio[4];


int _readByte(uint8_t reg, uint8_t nbytes, uint8_t *data)
{   
    return hal_i2c_read_register(i2c_dev, i2c_addr, reg, data);
}

int _writeByte(uint8_t reg, uint8_t nbytes, uint8_t *data)
{
    return hal_i2c_write_register(i2c_dev, i2c_addr, reg, *data);
}



int AXP20X_init(hal_i2c_t dev, uint8_t addr)
{
    if (hal_i2c_init(dev) != 0)
    {
        TRACE_ERROR("I2c device init failed");
        return -1;
    }

    i2c_dev = dev;
    i2c_addr = addr;
    
    _readByte(AXP202_IC_TYPE, 1, &chip_id);
    TRACE("Chip id detect 0x%X", chip_id);
    
    if (chip_id == AXP202_CHIP_ID || chip_id == AXP192_CHIP_ID) 
    {
        TRACE("Detect CHIP :%s", chip_id == AXP202_CHIP_ID ? "AXP202" : "AXP192");

        _readByte(AXP202_LDO234_DC23_CTL, 1, &_outputReg);
        TRACE("OUTPUT Register 0x%x", _outputReg);

        init = true;
    }
    else
    {
        TRACE_ERROR("Can't detect AXP20X device");
        return -1;
    }

    TRACE("Init");

    return 0;
}

int AXP20X_deinit(void)
{
    TRACE("Deinit");
    return 0;
}

int AXP20X_setPowerOutPut(uint8_t ch, bool en)
{
    uint8_t data;
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;

    _readByte(AXP202_LDO234_DC23_CTL, 1, &data);
    if (en) {
        data |= (1 << ch);
    } else {
        data &= (~(1 << ch));
    }

    FORCED_OPEN_DCDC3(data);    //! Must be forced open in T-Watch

    _writeByte(AXP202_LDO234_DC23_CTL, 1, &data);
    hal_delay_ms(1);
    _readByte(AXP202_LDO234_DC23_CTL, 1, &val);
    if (data == val) {
        _outputReg = val;
        return AXP_PASS;
    }
    return AXP_FAIL;
}

uint16_t AXP20X_getRegistH8L5(uint8_t regh8, uint8_t regl4)
{
    uint8_t hv, lv;
    _readByte(regh8, 1, &hv);
    _readByte(regl4, 1, &lv);
    return (hv << 5) | (lv & 0x1F);
}

uint16_t AXP20X_getRegistResult(uint8_t regh8, uint8_t regl4)
{
    uint8_t hv, lv;
    _readByte(regh8, 1, &hv);
    _readByte(regl4, 1, &lv);
    return (hv << 4) | (lv & 0xF);
}


//Only axp192 chip
bool AXP20X_isDCDC1Enable()
{
    return IS_OPEN(_outputReg, AXP192_DCDC1);
}
//Only axp192 chip
bool AXP20X_isExtenEnable()
{
    return IS_OPEN(_outputReg, AXP192_EXTEN);
}

bool AXP20X_isLDO2Enable()
{
    return IS_OPEN(_outputReg, AXP202_LDO2);
}

bool AXP20X_isLDO3Enable()
{
    return IS_OPEN(_outputReg, AXP202_LDO3);
}

bool AXP20X_isLDO4Enable()
{
    return IS_OPEN(_outputReg, AXP202_LDO4);
}

bool AXP20X_isDCDC2Enable()
{
    return IS_OPEN(_outputReg, AXP202_DCDC2);
}

bool AXP20X_isDCDC3Enable()
{
    return IS_OPEN(_outputReg, AXP202_DCDC3);
}

bool AXP20X_isChargeing()
{
    uint8_t reg;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_MODE_CHGSTATUS, 1, &reg);
    return IS_OPEN(reg, 6);
}

bool AXP20X_isBatteryConnect()
{
    uint8_t reg;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_MODE_CHGSTATUS, 1, &reg);
    return IS_OPEN(reg, 5);
}

float AXP20X_getAcinVoltage()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_ACIN_VOL_H8, AXP202_ACIN_VOL_L4) * AXP202_ACIN_VOLTAGE_STEP;
}

float AXP20X_getAcinCurrent()
{
    if (!init)return AXP_NOT_INIT;
    float rslt;
    uint8_t hv, lv;
    return AXP20X_getRegistResult(AXP202_ACIN_CUR_H8, AXP202_ACIN_CUR_L4) * AXP202_ACIN_CUR_STEP;
}

float AXP20X_getVbusVoltage()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_VBUS_VOL_H8, AXP202_VBUS_VOL_L4) * AXP202_VBUS_VOLTAGE_STEP;
}

float AXP20X_getVbusCurrent()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_VBUS_CUR_H8, AXP202_VBUS_CUR_L4) * AXP202_VBUS_CUR_STEP;
}

float AXP20X_getTemp()
{
    if (!init)return AXP_NOT_INIT;
    uint8_t hv, lv;
    _readByte(AXP202_INTERNAL_TEMP_H8, 1, &hv);
    _readByte(AXP202_INTERNAL_TEMP_L4, 1, &lv);
    float rslt =  hv << 8 | (lv & 0xF);
    return rslt / 1000;
}

float AXP20X_getTSTemp()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_TS_IN_H8, AXP202_TS_IN_L4) * AXP202_TS_PIN_OUT_STEP;
}

float AXP20X_getGPIO0Voltage()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_GPIO0_VOL_ADC_H8, AXP202_GPIO0_VOL_ADC_L4) * AXP202_GPIO0_STEP;
}

float AXP20X_getGPIO1Voltage()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_GPIO1_VOL_ADC_H8, AXP202_GPIO1_VOL_ADC_L4) * AXP202_GPIO1_STEP;
}

/*
Note: the battery power formula:
Pbat =2* register value * Voltage LSB * Current LSB / 1000.
(Voltage LSB is 1.1mV; Current LSB is 0.5mA, and unit of calculation result is mW.)
*/
float AXP20X_getBattInpower()
{
    float rslt;
    uint8_t hv, mv, lv;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_BAT_POWERH8, 1, &hv);
    _readByte(AXP202_BAT_POWERM8, 1, &mv);
    _readByte(AXP202_BAT_POWERL8, 1, &lv);
    rslt =  (hv << 16) | (mv << 8) | lv;
    rslt = 2 * rslt * 1.1 * 0.5 / 1000;
    return rslt;
}

float AXP20X_getBattVoltage()
{
    if (!init)return AXP_NOT_INIT;
    return AXP20X_getRegistResult(AXP202_BAT_AVERVOL_H8, AXP202_BAT_AVERVOL_L4) * AXP202_BATT_VOLTAGE_STEP;

}

float AXP20X_getBattChargeCurrent()
{
    if (!init)return AXP_NOT_INIT;
    switch (chip_id) {
    case AXP202_CHIP_ID:
        return AXP20X_getRegistResult(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L4) * AXP202_BATT_CHARGE_CUR_STEP;
    case AXP192_CHIP_ID:
        return AXP20X_getRegistH8L5(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L4) * AXP202_BATT_CHARGE_CUR_STEP;
    default:
        break;
    }
}

float AXP20X_getBattDischargeCurrent()
{
    float rslt;
    uint8_t hv, lv;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_BAT_AVERDISCHGCUR_H8, 1, &hv);
    _readByte(AXP202_BAT_AVERDISCHGCUR_L5, 1, &lv);
    rslt = (hv << 5) | (lv & 0x1F);
    return rslt * AXP202_BATT_DISCHARGE_CUR_STEP;
}

float AXP20X_getSysIPSOUTVoltage()
{
    float rslt;
    uint8_t hv, lv;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_APS_AVERVOL_H8, 1, &hv);
    _readByte(AXP202_APS_AVERVOL_L4, 1, &lv);
    rslt = (hv << 4) | (lv & 0xF);
    return rslt;
}

/*
Coulomb calculation formula:
C= 65536 * current LSB *（charge coulomb counter value - discharge coulomb counter value） /
3600 / ADC sample rate. Refer to REG84H setting for ADC sample rate；the current LSB is
0.5mA；unit of the calculation result is mAh. ）
*/
float AXP20X_getBattChargeCoulomb()
{
    float rslt;
    if (!init)return AXP_NOT_INIT;
    //TODO
}

float AXP20X_getBattDischargeCoulomb()
{
    float rslt;
    if (!init)return AXP_NOT_INIT;
    //TODO
}

int AXP20X_adc1Enable(uint16_t params, bool en)
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val;
    _readByte(AXP202_ADC_EN1, 1, &val);
    if (en)
        val |= params;
    else
        val &= ~(params);
    _writeByte(AXP202_ADC_EN1, 1, &val);

    _readByte(AXP202_ADC_EN1, 1, &val);
    return AXP_PASS;
}

int AXP20X_adc2Enable(uint16_t params, bool en)
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val;
    _readByte(AXP202_ADC_EN2, 1, &val);
    if (en)
        val |= params;
    else
        val &= ~(params);
    _writeByte(AXP202_ADC_EN2, 1, &val);
    return AXP_PASS;
}

int AXP20X_enableIRQ(uint32_t params, bool en)
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val, val1;
    if (params & 0xFF) {
        val1 = params & 0xFF;
        _readByte(AXP202_INTEN1, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        TRACE("%s [0x%x]val:0x%x", en ? "enable" : "disable", AXP202_INTEN1, val);
        _writeByte(AXP202_INTEN1, 1, &val);
    }
    if (params & 0xFF00) {
        val1 = params >> 8;
        _readByte(AXP202_INTEN2, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        TRACE("%s [0x%x]val:0x%x", en ? "enable" : "disable", AXP202_INTEN2, val);
        _writeByte(AXP202_INTEN2, 1, &val);
    }

    if (params & 0xFF0000) {
        val1 = params >> 16;
        _readByte(AXP202_INTEN3, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        TRACE("%s [0x%x]val:0x%x", en ? "enable" : "disable", AXP202_INTEN3, val);
        _writeByte(AXP202_INTEN3, 1, &val);
    }

    if (params & 0xFF000000) {
        val1 = params >> 24;
        _readByte(AXP202_INTEN4, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        TRACE("%s [0x%x]val:0x%x", en ? "enable" : "disable", AXP202_INTEN4, val);
        _writeByte(AXP202_INTEN4, 1, &val);
    }
    return AXP_PASS;
}

int AXP20X_readIRQ()
{
    if (!init)return AXP_NOT_INIT;
    switch (chip_id) {
    case AXP192_CHIP_ID:
        for (int i = 0; i < 4; ++i) {
            _readByte(AXP192_INTSTS1 + i, 1, &irq[i]);
        }
        _readByte(AXP192_INTSTS5, 1, &irq[4]);
        return AXP_PASS;

    case AXP202_CHIP_ID:
        for (int i = 0; i < 5; ++i) {
            _readByte(AXP202_INTSTS1 + i, 1, &irq[i]);
        }
        return AXP_PASS;
    default:
        return AXP_FAIL;
    }
}

void AXP20X_clearIRQ()
{
    uint8_t val = 0xFF;
    switch (chip_id) {
    case AXP192_CHIP_ID:
        for (int i = 0; i < 3; i++) {
            _writeByte(AXP192_INTSTS1 + i, 1, &val);
        }
        _writeByte(AXP192_INTSTS5, 1, &val);
        break;
    case AXP202_CHIP_ID:
        for (int i = 0; i < 5; i++) {
            _writeByte(AXP202_INTSTS1 + i, 1, &val);
        }
        break;
    default:
        break;
    }
    memset(irq, 0, sizeof(irq));
}

bool AXP20X_isAcinOverVoltageIRQ()
{
    return (bool)(irq[0] & BIT_MASK(7));
}

bool AXP20X_isAcinPlugInIRQ()
{
    return (bool)(irq[0] & BIT_MASK(6));
}

bool AXP20X_isAcinRemoveIRQ()
{
    return (bool)(irq[0] & BIT_MASK(5));
}

bool AXP20X_isVbusOverVoltageIRQ()
{
    return (bool)(irq[0] & BIT_MASK(4));
}

bool AXP20X_isVbusPlugInIRQ()
{
    return (bool)(irq[0] & BIT_MASK(3));
}

bool AXP20X_isVbusRemoveIRQ()
{
    return (bool)(irq[0] & BIT_MASK(2));
}

bool AXP20X_isVbusLowVHOLDIRQ()
{
    return (bool)(irq[0] & BIT_MASK(1));
}

bool AXP20X_isBattPlugInIRQ()
{
    return (bool)(irq[1] & BIT_MASK(7));
}
bool AXP20X_isBattRemoveIRQ()
{
    return (bool)(irq[1] & BIT_MASK(6));
}
bool AXP20X_isBattEnterActivateIRQ()
{
    return (bool)(irq[1] & BIT_MASK(5));
}
bool AXP20X_isBattExitActivateIRQ()
{
    return (bool)(irq[1] & BIT_MASK(4));
}
bool AXP20X_isChargingIRQ()
{
    return (bool)(irq[1] & BIT_MASK(3));
}
bool AXP20X_isChargingDoneIRQ()
{
    return (bool)(irq[1] & BIT_MASK(2));
}
bool AXP20X_isBattTempLowIRQ()
{
    return (bool)(irq[1] & BIT_MASK(1));
}
bool AXP20X_isBattTempHighIRQ()
{
    return (bool)(irq[1] & BIT_MASK(0));
}

bool AXP20X_isPEKShortPressIRQ()
{
    return (bool)(irq[2] & BIT_MASK(1));
}

bool AXP20X_isPEKLongtPressIRQ()
{
    return (bool)(irq[2] & BIT_MASK(0));
}

bool AXP20X_isVBUSPlug()
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val;
    _readByte(AXP202_STATUS, 1, &val);
    return (bool)(irq[2] & BIT_MASK(5));
}

int AXP20X_setDCDC2Voltage(uint16_t mv)
{
    if (!init)return AXP_NOT_INIT;
    if (mv < 700) {
        TRACE("DCDC2:Below settable voltage:700mV~2275mV");
        mv = 700;
    }
    if (mv > 2275) {
        TRACE("DCDC2:Above settable voltage:700mV~2275mV");
        mv = 2275;
    }
    uint8_t val = (mv - 700) / 25;
    _writeByte(AXP202_DC2OUT_VOL, 1, &val);
    return AXP_PASS;
}

int AXP20X_setDCDC3Voltage(uint16_t mv)
{
    if (!init)return AXP_NOT_INIT;
    if (mv < 700) {
        TRACE("DCDC3:Below settable voltage:700mV~3500mV");
        mv = 700;
    }
    if (mv > 3500) {
        TRACE("DCDC3:Above settable voltage:700mV~3500mV");
        mv = 3500;
    }
    uint8_t val = (mv - 700) / 25;
    _writeByte(AXP202_DC3OUT_VOL, 1, &val);
    return AXP_PASS;
}

int AXP20X_setLDO2Voltage(uint16_t mv)
{
    if (!init)return AXP_NOT_INIT;
    if (mv < 1800) {
        TRACE("LDO2:Below settable voltage:1800mV~3300mV");
        mv = 1800;
    }
    if (mv > 3300) {
        TRACE("LDO2:Above settable voltage:1800mV~3300mV");
        mv = 3300;
    }
    uint8_t val = (mv - 1800) / 100;
    _writeByte(AXP202_LDO24OUT_VOL, 1, &val);
    return AXP_PASS;
}

//!29H LDO3 Output Voltage Setting
// LDO3 Mode select：
// Bit 7    0：LDO mode，voltage can be set by [6:0]
//          1：enable/disable control mode，and voltage is determined by LDO3IN
// 6-0      LDO3 output voltage setting Bit6-Bit0
// 0.7-2.275V，25mV/step
// Vout=[0.7+(Bit6-0)*0.025]V
int AXP20X_setLDO3Voltage(uint16_t mv)
{
    if (!init)return AXP_NOT_INIT;
    if (mv < 700) {
        TRACE("LDO3:Below settable voltage:700mV~2275mV");
        mv = 700;
    }
    if (mv > 2275) {
        TRACE("LDO3:Above settable voltage:700mV~2275mV");
        mv = 2275;
    }
    uint8_t val = (mv - 700) / 25;
    // val |= BIT_MASK(7);
    _writeByte(AXP202_LDO3OUT_VOL, 1, &val);
    return AXP_PASS;
}

int AXP20X_setLDO4Voltage(axp_ldo4_table_t param)
{
    if (!init)return AXP_NOT_INIT;
    if (param >= AXP202_LDO4_MAX)return AXP_INVALID;
    uint8_t val;
    _readByte(AXP202_LDO24OUT_VOL, 1, &val);
    val &= 0xF0;
    val |= param;
    _writeByte(AXP202_LDO24OUT_VOL, 1, &val);
    return AXP_PASS;
}

// 0 : LDO  1 : DCIN
int AXP20X_setLDO3Mode(uint8_t mode)
{
    uint8_t val;
    _readByte(AXP202_LDO3OUT_VOL, 1, &val);
    if (mode) {
        val |= BIT_MASK(7);
    } else {
        val &= (~BIT_MASK(7));
    }
    _writeByte(AXP202_LDO3OUT_VOL, 1, &val);
    return AXP_PASS;

}

int AXP20X_setStartupTime(uint8_t param)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    if (param > sizeof(startupParams) / sizeof(startupParams[0]))return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b11000000);
    val |= startupParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
}

int AXP20X_setlongPressTime(uint8_t param)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    if (param > sizeof(longPressParams) / sizeof(longPressParams[0]))return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b00110000);
    val |= longPressParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
}

int AXP20X_setShutdownTime(uint8_t param)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    if (param > sizeof(shutdownParams) / sizeof(shutdownParams[0]))return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b00000011);
    val |= shutdownParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
}

int AXP20X_setTimeOutShutdown(bool en)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_POK_SET, 1, &val);
    if (en)
        val |= (1 << 3);
    else
        val &= (~(1 << 3));
    _writeByte(AXP202_POK_SET, 1, &val);
}


int AXP20X_shutdown()
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_OFF_CTL, 1, &val);
    val |= (1 << 7);
    _writeByte(AXP202_OFF_CTL, 1, &val);
}


float AXP20X_getSettingChargeCurrent()
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_CHARGE1, 1, &val);
    val &= 0b00000111;
    float cur = 300.0 + val * 100.0;
    TRACE("Setting Charge current : %.2f mA", cur);
    return cur;
}

bool AXP20X_isChargeingEnable()
{
    uint8_t val;
    if (!init)return false;
    _readByte(AXP202_CHARGE1, 1, &val);
    if (val & (1 << 7)) {
        TRACE("Charging enable is enable");
        val = true;
    } else {
        TRACE("Charging enable is disable");
        val = false;
    }
    return val;
}

int AXP20X_enableChargeing(bool en)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_CHARGE1, 1, &val);
    val |= (1 << 7);
    _writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
}

int AXP20X_setChargingTargetVoltage(axp_chargeing_vol_t param)
{
    uint8_t val;
    if (!init)return AXP_NOT_INIT;
    if (param > sizeof(targetVolParams) / sizeof(targetVolParams[0]))return AXP_INVALID;
    _readByte(AXP202_CHARGE1, 1, &val);
    val &= ~(0b01100000);
    val |= targetVolParams[param];
    _writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
}

int AXP20X_getBattPercentage()
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val;
    _readByte(AXP202_BATT_PERCENTAGE, 1, &val);
    if (!(val & BIT_MASK(7))) {
        return val & (~BIT_MASK(7));
    }
    return 0;
}

int AXP20X_setChgLEDMode(uint8_t mode)
{
    uint8_t val;
    _readByte(AXP202_OFF_CTL, 1, &val);
    val |= BIT_MASK(3);
    switch (mode) {
    case AXP20X_LED_OFF:
        val &= 0b11001111;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_BLINK_1HZ:
        val &= 0b11001111;
        val |= 0b00010000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_BLINK_4HZ:
        val &= 0b11001111;
        val |= 0b00100000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_LOW_LEVEL:
        val &= 0b11001111;
        val |= 0b00110000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    default:
        break;
    }
    return AXP_PASS;
}

int AXP20X_debugCharging()
{
    uint8_t val;
    _readByte(AXP202_CHARGE1, 1, &val);
    TRACE("SRC REG:0x%x", val);
    if (val & (1 << 7)) {
        TRACE("Charging enable is enable");
    } else {
        TRACE("Charging enable is disable");
    }
    TRACE("Charging target-voltage : 0x%x", ((val & 0b01100000) >> 5 ) & 0b11);
    if (val & (1 << 4)) {
        TRACE("end when the charge current is lower than 15%% of the set value");
    } else {
        TRACE(" end when the charge current is lower than 10%% of the set value");
    }
    val &= 0b00000111;
    float cur = 300.0 + val * 100.0;
    TRACE("Charge current : %.2f mA", cur);
}


int AXP20X_debugStatus()
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val, val1, val2;
    _readByte(AXP202_STATUS, 1, &val);
    _readByte(AXP202_MODE_CHGSTATUS, 1, &val1);
    _readByte(AXP202_IPS_SET, 1, &val2);
    TRACE("AXP202_STATUS:   AXP202_MODE_CHGSTATUS   AXP202_IPS_SET");
    TRACE("0x%x\t\t\t 0x%x\t\t\t 0x%x", val, val1, val2);
}


int AXP20X_limitingOff()
{
    if (!init)return AXP_NOT_INIT;
    uint8_t val;
    _readByte(AXP202_IPS_SET, 1, &val);
    if (chip_id == AXP202_CHIP_ID) {
        // val &= ~(1 << 1);
        val |= 0x03;
    } else {
        val &= ~(1 << 1);
    }
    _writeByte(AXP202_IPS_SET, 1, &val);
    return AXP_PASS;
}

// Only AXP129 chip
int AXP20X_setDCDC1Voltage(uint16_t mv)
{
    if (!init)return AXP_NOT_INIT;
    if (mv < 700) {
        TRACE("DCDC1:Below settable voltage:700mV~3500mV");
        mv = 700;
    }
    if (mv > 3500) {
        TRACE("DCDC1:Above settable voltage:700mV~3500mV");
        mv = 3500;
    }
    uint8_t val = (mv - 700) / 25;
    _writeByte(AXP192_DC1_VLOTAGE, 1, &val);
    return AXP_PASS;
}

int AXP20X_setGPIO0Voltage(uint8_t param)
{
    uint8_t params[] = {
        0b11111000,
        0b11111001,
        0b11111010,
        0b11111011,
        0b11111100,
        0b11111101,
        0b11111110,
        0b11111111,
    };
    if (!init)return AXP_NOT_INIT;
    if (param > sizeof(params) / sizeof(params[0]))return AXP_INVALID;
    uint8_t val = 0;
    _readByte(AXP202_GPIO0_VOL, 1, &val);
    val &= 0b11111000;
    val |= params[param];
    _writeByte(AXP202_GPIO0_VOL, 1, &val);
    return AXP_PASS;
}

int AXP20X_setGPIO0Level(uint8_t level)
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_GPIO0_CTL, 1, &val);
    val = level ? val & 0b11111000 : (val & 0b11111000) | 0b00000001;
    _writeByte(AXP202_GPIO0_CTL, 1, &val);
    return AXP_PASS;
}
int AXP20X_setGPIO1Level(uint8_t level)
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_GPIO1_CTL, 1, &val);
    val = level ? val & 0b11111000 : (val & 0b11111000) | 0b00000001;
    _writeByte(AXP202_GPIO1_CTL, 1, &val);
    return AXP_PASS;
}

int AXP20X_readGpioStatus()
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_GPIO012_SIGNAL, 1, &val);
    gpio[0] = val & BIT_MASK(4);
    gpio[1] = val & BIT_MASK(5);
    gpio[2] = val & BIT_MASK(6);
    _readByte(AXP202_GPIO3_CTL, 1, &val);
    gpio[3] = val & 1;
    return AXP_PASS;
}

int AXP20X_readGpio0Level()
{
    return gpio[0];
}

int AXP20X_readGpio1Level()
{
    return gpio[1];
}

int AXP20X_readGpio2Level()
{
    return gpio[2];
}


int AXP20X_setGpio2Mode(uint8_t mode)
{
    uint8_t params[] = {
        0b11111000,
        0b11111001,
        0b11111010,
    };
    if (!init)return AXP_NOT_INIT;
    if (mode > sizeof(params) / sizeof(params[0]))return AXP_INVALID;
    uint8_t val = 0;
    _readByte(AXP202_GPIO2_CTL, 1, &val);
    val &= params[0];
    val |= params[mode];
    _writeByte(AXP202_GPIO2_CTL, 1, &val);
    return AXP_PASS;
}

int AXP20X_setGpio3Mode(uint8_t mode)
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_GPIO3_CTL, 1, &val);
    if (mode == AXP202_GPIO3_DIGITAL_INPUT) {
        val |= BIT_MASK(2);
    } else if (mode == AXP202_GPIO3_OPEN_DRAIN_OUTPUT) {
        val &= ~BIT_MASK(2);
    } else {
        return AXP_INVALID;
    }
    return AXP_PASS;
}

int AXP20X_setGpio3Level(uint8_t level)
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    _readByte(AXP202_GPIO3_CTL, 1, &val);
    if (!(val & BIT_MASK(2))) {
        return AXP_FAIL;
    }
    val = level ? val & (~BIT_MASK(1)) : val |  BIT_MASK(1);
    _writeByte(AXP202_GPIO3_CTL, 1, &val);
}

int AXP20X_setGpioInterrupt(uint8_t *val, int mode, bool en)
{
    switch (mode) {
    case RISING:
        *val = en ? *val | BIT_MASK(7) : *val & (~BIT_MASK(7));
        break;
    case FALLING:
        *val = en ? *val | BIT_MASK(6) : *val & (~BIT_MASK(6));
        break;
    default:
        break;
    }
}

int AXP20X_setGpioInterruptMode(uint8_t gpio, int mode, bool en)
{
    uint8_t val = 0;
    if (!init)return AXP_NOT_INIT;
    switch (gpio) {
    case AXP202_GPIO0:
        _readByte(AXP202_GPIO0_CTL, 1, &val);
        AXP20X_setGpioInterrupt(&val, mode, en);
        break;
    case AXP202_GPIO1:
        _readByte(AXP202_GPIO1_CTL, 1, &val);
        AXP20X_setGpioInterrupt(&val, mode, en);
        break;
    case AXP202_GPIO2:
        _readByte(AXP202_GPIO2_CTL, 1, &val);
        AXP20X_setGpioInterrupt(&val, mode, en);
        break;
    case AXP202_GPIO3:
        _readByte(AXP202_GPIO3_CTL, 1, &val);
        AXP20X_setGpioInterrupt(&val, mode, en);
        break;
    }
}


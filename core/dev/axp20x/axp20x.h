
#ifndef __AXP20X_H
#define __AXP20X_H

#ifndef RISING
#define RISING 0x01
#endif

#ifndef FALLING
#define FALLING 0x02
#endif

//! Error Code
#define AXP_PASS 0
#define AXP_FAIL -1
#define AXP_INVALID -2
#define AXP_NOT_INIT -3

//! Chip Address
#define AXP202_SLAVE_ADDRESS (0x35)
#define AXP192_SLAVE_ADDRESS (0x34)

//! Chip ID
#define AXP202_CHIP_ID 0x41
#define AXP192_CHIP_ID 0x03

//! REG MAP
#define AXP202_STATUS (0x00)
#define AXP202_MODE_CHGSTATUS (0x01)
#define AXP202_OTG_STATUS (0x02)
#define AXP202_IC_TYPE (0x03)
#define AXP202_DATA_BUFFER1 (0x04)
#define AXP202_DATA_BUFFER2 (0x05)
#define AXP202_DATA_BUFFER3 (0x06)
#define AXP202_DATA_BUFFER4 (0x07)
#define AXP202_DATA_BUFFER5 (0x08)
#define AXP202_DATA_BUFFER6 (0x09)
#define AXP202_DATA_BUFFER7 (0x0A)
#define AXP202_DATA_BUFFER8 (0x0B)
#define AXP202_DATA_BUFFER9 (0x0C)
#define AXP202_DATA_BUFFERA (0x0D)
#define AXP202_DATA_BUFFERB (0x0E)
#define AXP202_DATA_BUFFERC (0x0F)
#define AXP202_LDO234_DC23_CTL (0x12)
#define AXP202_DC2OUT_VOL (0x23)
#define AXP202_LDO3_DC2_DVM (0x25)
#define AXP202_DC3OUT_VOL (0x27)
#define AXP202_LDO24OUT_VOL (0x28)
#define AXP202_LDO3OUT_VOL (0x29)
#define AXP202_IPS_SET (0x30)
#define AXP202_VOFF_SET (0x31)
#define AXP202_OFF_CTL (0x32)
#define AXP202_CHARGE1 (0x33)
#define AXP202_CHARGE2 (0x34)
#define AXP202_BACKUP_CHG (0x35)
#define AXP202_POK_SET (0x36)
#define AXP202_DCDC_FREQSET (0x37)
#define AXP202_VLTF_CHGSET (0x38)
#define AXP202_VHTF_CHGSET (0x39)
#define AXP202_APS_WARNING1 (0x3A)
#define AXP202_APS_WARNING2 (0x3B)
#define AXP202_TLTF_DISCHGSET (0x3C)
#define AXP202_THTF_DISCHGSET (0x3D)
#define AXP202_DCDC_MODESET (0x80)
#define AXP202_ADC_EN1 (0x82)
#define AXP202_ADC_EN2 (0x83)
#define AXP202_ADC_SPEED (0x84)
#define AXP202_ADC_INPUTRANGE (0x85)
#define AXP202_ADC_IRQ_RETFSET (0x86)
#define AXP202_ADC_IRQ_FETFSET (0x87)
#define AXP202_TIMER_CTL (0x8A)
#define AXP202_VBUS_DET_SRP (0x8B)
#define AXP202_HOTOVER_CTL (0x8F)
#define AXP202_GPIO0_CTL (0x90)
#define AXP202_GPIO0_VOL (0x91)
#define AXP202_GPIO1_CTL (0x92)
#define AXP202_GPIO2_CTL (0x93)
#define AXP202_GPIO012_SIGNAL (0x94)
#define AXP202_GPIO3_CTL (0x95)
#define AXP202_INTEN1 (0x40)
#define AXP202_INTEN2 (0x41)
#define AXP202_INTEN3 (0x42)
#define AXP202_INTEN4 (0x43)
#define AXP202_INTEN5 (0x44)
#define AXP202_INTSTS1 (0x48)
#define AXP202_INTSTS2 (0x49)
#define AXP202_INTSTS3 (0x4A)
#define AXP202_INTSTS4 (0x4B)
#define AXP202_INTSTS5 (0x4C)

//Irq control register
#define AXP192_INTEN1 (0x40)
#define AXP192_INTEN2 (0x41)
#define AXP192_INTEN3 (0x42)
#define AXP192_INTEN4 (0x43)
#define AXP192_INTEN5 (0x4A)
//Irq status register
#define AXP192_INTSTS1 (0x44)
#define AXP192_INTSTS2 (0x45)
#define AXP192_INTSTS3 (0x46)
#define AXP192_INTSTS4 (0x47)
#define AXP192_INTSTS5 (0x4D)

#define AXP192_DC1_VLOTAGE (0x26)

/* axp 20 adc data register */
#define AXP202_BAT_AVERVOL_H8 (0x78)
#define AXP202_BAT_AVERVOL_L4 (0x79)
#define AXP202_BAT_AVERCHGCUR_H8 (0x7A)
#define AXP202_BAT_AVERCHGCUR_L4 (0x7B)
#define AXP202_BAT_VOL_H8 (0x50)
#define AXP202_BAT_VOL_L4 (0x51)
#define AXP202_ACIN_VOL_H8 (0x56)
#define AXP202_ACIN_VOL_L4 (0x57)
#define AXP202_ACIN_CUR_H8 (0x58)
#define AXP202_ACIN_CUR_L4 (0x59)
#define AXP202_VBUS_VOL_H8 (0x5A)
#define AXP202_VBUS_VOL_L4 (0x5B)
#define AXP202_VBUS_CUR_H8 (0x5C)
#define AXP202_VBUS_CUR_L4 (0x5D)
#define AXP202_INTERNAL_TEMP_H8 (0x5E)
#define AXP202_INTERNAL_TEMP_L4 (0x5F)
#define AXP202_TS_IN_H8 (0x62)
#define AXP202_TS_IN_L4 (0x63)
#define AXP202_GPIO0_VOL_ADC_H8 (0x64)
#define AXP202_GPIO0_VOL_ADC_L4 (0x65)
#define AXP202_GPIO1_VOL_ADC_H8 (0x66)
#define AXP202_GPIO1_VOL_ADC_L4 (0x67)

#define AXP202_BAT_AVERDISCHGCUR_H8 (0x7C)
#define AXP202_BAT_AVERDISCHGCUR_L5 (0x7D)
#define AXP202_APS_AVERVOL_H8 (0x7E)
#define AXP202_APS_AVERVOL_L4 (0x7F)
#define AXP202_INT_BAT_CHGCUR_H8 (0xA0)
#define AXP202_INT_BAT_CHGCUR_L4 (0xA1)
#define AXP202_EXT_BAT_CHGCUR_H8 (0xA2)
#define AXP202_EXT_BAT_CHGCUR_L4 (0xA3)
#define AXP202_INT_BAT_DISCHGCUR_H8 (0xA4)
#define AXP202_INT_BAT_DISCHGCUR_L4 (0xA5)
#define AXP202_EXT_BAT_DISCHGCUR_H8 (0xA6)
#define AXP202_EXT_BAT_DISCHGCUR_L4 (0xA7)
#define AXP202_BAT_CHGCOULOMB3 (0xB0)
#define AXP202_BAT_CHGCOULOMB2 (0xB1)
#define AXP202_BAT_CHGCOULOMB1 (0xB2)
#define AXP202_BAT_CHGCOULOMB0 (0xB3)
#define AXP202_BAT_DISCHGCOULOMB3 (0xB4)
#define AXP202_BAT_DISCHGCOULOMB2 (0xB5)
#define AXP202_BAT_DISCHGCOULOMB1 (0xB6)
#define AXP202_BAT_DISCHGCOULOMB0 (0xB7)
#define AXP202_COULOMB_CTL (0xB8)
#define AXP202_BAT_POWERH8 (0x70)
#define AXP202_BAT_POWERM8 (0x71)
#define AXP202_BAT_POWERL8 (0x72)

#define AXP202_VREF_TEM_CTRL (0xF3)
#define AXP202_BATT_PERCENTAGE (0xB9)

/* bit definitions for AXP events ,irq event */
/*  AXP202  */
#define AXP202_IRQ_USBLO (1)
#define AXP202_IRQ_USBRE (2)
#define AXP202_IRQ_USBIN (3)
#define AXP202_IRQ_USBOV (4)
#define AXP202_IRQ_ACRE (5)
#define AXP202_IRQ_ACIN (6)
#define AXP202_IRQ_ACOV (7)

#define AXP202_IRQ_TEMLO (8)
#define AXP202_IRQ_TEMOV (9)
#define AXP202_IRQ_CHAOV (10)
#define AXP202_IRQ_CHAST (11)
#define AXP202_IRQ_BATATOU (12)
#define AXP202_IRQ_BATATIN (13)
#define AXP202_IRQ_BATRE (14)
#define AXP202_IRQ_BATIN (15)

#define AXP202_IRQ_POKLO (16)
#define AXP202_IRQ_POKSH (17)
#define AXP202_IRQ_LDO3LO (18)
#define AXP202_IRQ_DCDC3LO (19)
#define AXP202_IRQ_DCDC2LO (20)
#define AXP202_IRQ_CHACURLO (22)
#define AXP202_IRQ_ICTEMOV (23)

#define AXP202_IRQ_EXTLOWARN2 (24)
#define AXP202_IRQ_EXTLOWARN1 (25)
#define AXP202_IRQ_SESSION_END (26)
#define AXP202_IRQ_SESS_AB_VALID (27)
#define AXP202_IRQ_VBUS_UN_VALID (28)
#define AXP202_IRQ_VBUS_VALID (29)
#define AXP202_IRQ_PDOWN_BY_NOE (30)
#define AXP202_IRQ_PUP_BY_NOE (31)

#define AXP202_IRQ_GPIO0TG (32)
#define AXP202_IRQ_GPIO1TG (33)
#define AXP202_IRQ_GPIO2TG (34)
#define AXP202_IRQ_GPIO3TG (35)
#define AXP202_IRQ_PEKFE (37)
#define AXP202_IRQ_PEKRE (38)
#define AXP202_IRQ_TIMER (39)

//Signal Capture
#define AXP202_BATT_VOLTAGE_STEP (1.1F)
#define AXP202_BATT_DISCHARGE_CUR_STEP (0.5F)
#define AXP202_BATT_CHARGE_CUR_STEP (0.5F)
#define AXP202_ACIN_VOLTAGE_STEP (1.7F)
#define AXP202_ACIN_CUR_STEP (0.625F)
#define AXP202_VBUS_VOLTAGE_STEP (1.7F)
#define AXP202_VBUS_CUR_STEP (0.375F)
#define AXP202_INTENAL_TEMP_STEP (0.1F)
#define AXP202_APS_VOLTAGE_STEP (1.4F)
#define AXP202_TS_PIN_OUT_STEP (0.8F)
#define AXP202_GPIO0_STEP (0.5F)
#define AXP202_GPIO1_STEP (0.5F)

#define FORCED_OPEN_DCDC3(x) (x |= AXP202_DCDC3)
#define BIT_MASK(x) (1 << x)
#define IS_OPEN(reg, channel) (bool)(reg & BIT_MASK(channel))

#define AXP202_ON 1
#define AXP202_OFF 0

enum
{
    AXP202_EXTEN = 0,
    AXP202_DCDC3 = 1,
    AXP202_LDO2 = 2,
    AXP202_LDO4 = 3,
    AXP202_DCDC2 = 4,
    AXP202_LDO3 = 6,
    AXP202_OUTPUT_MAX,
};

enum
{
    AXP192_DCDC1 = 0,
    AXP192_DCDC3 = 1,
    AXP192_LDO2 = 2,
    AXP192_LDO3 = 3,
    AXP192_DCDC2 = 4,
    AXP192_EXTEN = 6,
    AXP192_OUTPUT_MAX,
};

enum
{
    AXP202_STARTUP_TIME,
    AXP202_LONGPRESS_TIME,
    AXP202_SHUTDOWN_EXCEEDS_TIME,
    AXP202_PWROK_SIGNAL_DELAY,
    AXP202_SHUTDOWN_TIME,
};

enum
{
    AXP202_STARTUP_TIME_128MS,
    AXP202_STARTUP_TIME_3S,
    AXP202_STARTUP_TIME_1S,
    AXP202_STARTUP_TIME_2S,
};

//REG 33H: Charging control 1 Charging target-voltage setting
typedef enum
{
    AXP202_TARGET_VOL_4_1V,
    AXP202_TARGET_VOL_4_15V,
    AXP202_TARGET_VOL_4_2V,
    AXP202_TARGET_VOL_4_36V
} axp_chargeing_vol_t;

//REG 82H: ADC Enable 1 register Parameter
typedef enum
{
    AXP202_BATT_VOL_ADC1 = 1 << 7,
    AXP202_BATT_CUR_ADC1 = 1 << 6,
    AXP202_ACIN_VOL_ADC1 = 1 << 5,
    AXP202_ACIN_CUR_ADC1 = 1 << 4,
    AXP202_VBUS_VOL_ADC1 = 1 << 3,
    AXP202_VBUS_CUR_ADC1 = 1 << 2,
    AXP202_APS_VOL_ADC1 = 1 << 1,
    AXP202_TS_PIN_ADC1 = 1 << 0
} axp_adc1_func_t;

// REG 83H: ADC Enable 2 register Parameter
typedef enum
{
    AXP202_TEMP_MONITORING_ADC2 = 1 << 7,
    AXP202_GPIO1_FUNC_ADC2 = 1 << 3,
    AXP202_GPIO0_FUNC_ADC2 = 1 << 2
} axp_adc2_func_t;

enum
{
    AXP202_LDO3_MODE_LDO,
    AXP202_LDO3_MODE_DCIN
};

typedef enum
{
    //IRQ1
    AXP202_VBUS_VHOLD_LOW_IRQ = 1 << 1,
    AXP202_VBUS_REMOVED_IRQ = 1 << 2,
    AXP202_VBUS_CONNECT_IRQ = 1 << 3,
    AXP202_VBUS_OVER_VOL_IRQ = 1 << 4,
    AXP202_ACIN_REMOVED_IRQ = 1 << 5,
    AXP202_ACIN_CONNECT_IRQ = 1 << 6,
    AXP202_ACIN_OVER_VOL_IRQ = 1 << 7,
    //IRQ2
    AXP202_BATT_LOW_TEMP_IRQ = 1 << 8,
    AXP202_BATT_OVER_TEMP_IRQ = 1 << 9,
    AXP202_CHARGING_FINISHED_IRQ = 1 << 10,
    AXP202_CHARGING_IRQ = 1 << 11,
    AXP202_BATT_EXIT_ACTIVATE_IRQ = 1 << 12,
    AXP202_BATT_ACTIVATE_IRQ = 1 << 13,
    AXP202_BATT_REMOVED_IRQ = 1 << 14,
    AXP202_BATT_CONNECT_IRQ = 1 << 15,
    //IRQ3
    AXP202_PEK_LONGPRESS_IRQ = 1 << 16,
    AXP202_PEK_SHORTPRESS_IRQ = 1 << 17,
    AXP202_LDO3_LOW_VOL_IRQ = 1 << 18,
    AXP202_DC3_LOW_VOL_IRQ = 1 << 19,
    AXP202_DC2_LOW_VOL_IRQ = 1 << 20,
    AXP202_CHARGE_LOW_CUR_IRQ = 1 << 21,
    AXP202_CHIP_TEMP_HIGH_IRQ = 1 << 22,

    //IRQ4
    AXP202_APS_LOW_VOL_LEVEL2_IRQ = 1 << 24,
    APX202_APS_LOW_VOL_LEVEL1_IRQ = 1 << 25,
    AXP202_VBUS_SESSION_END_IRQ = 1 << 26,
    AXP202_VBUS_SESSION_AB_IRQ = 1 << 27,
    AXP202_VBUS_INVALID_IRQ = 1 << 28,
    AXP202_VBUS_VAILD_IRQ = 1 << 29,
    AXP202_NOE_OFF_IRQ = 1 << 30,
    AXP202_NOE_ON_IRQ = 1 << 31,
    AXP202_ALL_IRQ = 0xFFFF

} axp_irq_t;

typedef enum
{
    AXP202_LDO4_1250MV,
    AXP202_LDO4_1300MV,
    AXP202_LDO4_1400MV,
    AXP202_LDO4_1500MV,
    AXP202_LDO4_1600MV,
    AXP202_LDO4_1700MV,
    AXP202_LDO4_1800MV,
    AXP202_LDO4_1900MV,
    AXP202_LDO4_2000MV,
    AXP202_LDO4_2500MV,
    AXP202_LDO4_2700MV,
    AXP202_LDO4_2800MV,
    AXP202_LDO4_3000MV,
    AXP202_LDO4_3100MV,
    AXP202_LDO4_3200MV,
    AXP202_LDO4_3300MV,
    AXP202_LDO4_MAX,
} axp_ldo4_table_t;

typedef enum
{
    AXP20X_LED_OFF,
    AXP20X_LED_BLINK_1HZ,
    AXP20X_LED_BLINK_4HZ,
    AXP20X_LED_LOW_LEVEL,
} axp_chgled_mode_t;

typedef enum
{
    AXP202_GPIO_1V8,
    AXP202_GPIO_2V5,
    AXP202_GPIO_2V8,
    AXP202_GPIO_3V0,
    AXP202_GPIO_3V1,
    AXP202_GPIO_3V3,
    AXP202_GPIO_3V4,
    AXP202_GPIO_3V5,
} axp_gpio_voltage_t;

typedef enum
{
    AXP202_GPIO2_OUTPUT_LOW,
    AXP202_GPIO2_FLOATING,
    AXP202_GPIO3_INPUT,
} axp202_gpio2_mode_t;

typedef enum
{
    AXP202_GPIO3_DIGITAL_INPUT,
    AXP202_GPIO3_OPEN_DRAIN_OUTPUT,
} axp202_gpio3_mode_t;

typedef enum
{
    AXP202_GPIO3_OUTPUT_LOW,
    AXP202_GPIO3_FLOATING,
} axp202_gpio3_output_t;

typedef enum
{
    AXP202_GPIO0,
    AXP202_GPIO1,
    AXP202_GPIO2,
    AXP202_GPIO3,
} axp202_gpio_t;


int AXP20X_init(hal_i2c_t i2c, uint8_t address);
int AXP20X_deinit(void);

int AXP20X_setPowerOutPut(uint8_t ch, bool en);


bool AXP20X_isDCDC1Enable();
bool AXP20X_isExtenEnable();
bool AXP20X_isLDO2Enable();
bool AXP20X_isLDO3Enable();
bool AXP20X_isLDO4Enable();
bool AXP20X_isDCDC2Enable();
bool AXP20X_isDCDC3Enable();

bool AXP20X_isChargeing();
bool AXP20X_isBatteryConnect();
float AXP20X_getAcinVoltage();
float AXP20X_getAcinCurrent();
float AXP20X_getVbusVoltage();
float AXP20X_getVbusCurrent();
float AXP20X_getTemp();
float AXP20X_getTSTemp();
float AXP20X_getGPIO0Voltage();
float AXP20X_getGPIO1Voltage();
float AXP20X_getBattInpower();
float AXP20X_getBattVoltage();
float AXP20X_getBattChargeCurrent();
float AXP20X_getBattDischargeCurrent();
float AXP20X_getSysIPSOUTVoltage();
float AXP20X_getBattChargeCoulomb();








#endif   // __AXP20X_H
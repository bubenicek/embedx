/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Driver for the MAX9860 Mono Audio Voice Codec
 *
 * Author: Peter Rosin <peda@axentia.s>
 *         Copyright 2016 Axentia Technologies
 */

#ifndef _SND_SOC_MAX9860
#define _SND_SOC_MAX9860

//
// MAX 9860 HW configuration
//
#define CFG_MAX9860_PIN_BCK            32
#define CFG_MAX9860_PIN_LRCLK          4
#define CFG_MAX9860_PIN_DOUT           0
#define CFG_MAX9860_PIN_DIN            14

#define CFG_MAX9860_I2S_NUM            1

#define CFG_MAX9860_SAMPLERATE         16000
#define CFG_MAX9860_BITSPERSAMPLE      16

//#define CFG_MAX9860_MCLK_FREQ          11289600
#define CFG_MAX9860_MCLK_FREQ          12000000
//#define CFG_MAX9860_MCLK_FREQ          16000000
#define CFG_MAX9860_MCLK_PIN           27

#define CFG_MAX9860_DMA_BUFLEN         160
#define CFG_MAX9860_DMA_BUFCOUNT       4


//
// Registers
//
#define MAX9860_ADDR         (0x20 >> 1)

#define MAX9860_INTRSTATUS   0x00
#define MAX9860_MICREADBACK  0x01
#define MAX9860_INTEN        0x02
#define MAX9860_SYSCLK       0x03
#define MAX9860_AUDIOCLKHIGH 0x04
#define MAX9860_AUDIOCLKLOW  0x05
#define MAX9860_IFC1A        0x06
#define MAX9860_IFC1B        0x07
#define MAX9860_VOICEFLTR    0x08
#define MAX9860_DACATTN      0x09
#define MAX9860_ADCLEVEL     0x0a
#define MAX9860_DACGAIN      0x0b
#define MAX9860_MICGAIN      0x0c
#define MAX9860_RESERVED     0x0d
#define MAX9860_MICADC       0x0e
#define MAX9860_NOISEGATE    0x0f
#define MAX9860_PWRMAN       0x10
#define MAX9860_REVISION     0xff

#define MAX9860_MAX_REGISTER 0xff

/* INTRSTATUS */
#define MAX9860_CLD          0x80
#define MAX9860_SLD          0x40
#define MAX9860_ULK          0x20

/* MICREADBACK */
#define MAX9860_NG           0xe0
#define MAX9860_AGC          0x1f

/* INTEN */
#define MAX9860_ICLD         0x80
#define MAX9860_ISLD         0x40
#define MAX9860_IULK         0x20

/* SYSCLK */
#define MAX9860_PSCLK        0x30
#define MAX9860_PSCLK_OFF    0x00
#define MAX9860_PSCLK_SHIFT  4
#define MAX9860_FREQ         0x06
#define MAX9860_FREQ_NORMAL  0x00
#define MAX9860_FREQ_12MHZ   0x02
#define MAX9860_FREQ_13MHZ   0x04
#define MAX9860_FREQ_19_2MHZ 0x06
#define MAX9860_16KHZ        0x01

/* AUDIOCLKHIGH */
#define MAX9860_PLL          0x80
#define MAX9860_NHI          0x7f

/* AUDIOCLKLOW */
#define MAX9860_NLO          0xff

/* IFC1A */
#define MAX9860_MASTER       0x80
#define MAX9860_WCI          0x40
#define MAX9860_DBCI         0x20
#define MAX9860_DDLY         0x10
#define MAX9860_HIZ          0x08
#define MAX9860_TDM          0x04

/* IFC1B */
#define MAX9860_ABCI         0x20
#define MAX9860_ADLY         0x10
#define MAX9860_ST           0x08
#define MAX9860_BSEL         0x07
#define MAX9860_BSEL_OFF     0x00
#define MAX9860_BSEL_64X     0x01
#define MAX9860_BSEL_48X     0x02
#define MAX9860_BSEL_PCLK_2  0x04
#define MAX9860_BSEL_PCLK_4  0x05
#define MAX9860_BSEL_PCLK_8  0x06
#define MAX9860_BSEL_PCLK_16 0x07

/* VOICEFLTR */
#define MAX9860_AVFLT        0xf0
#define MAX9860_AVFLT_SHIFT  4
#define MAX9860_AVFLT_COUNT  6
#define MAX9860_DVFLT        0x0f
#define MAX9860_DVFLT_SHIFT  0
#define MAX9860_DVFLT_COUNT  6

/* DACATTN */
#define MAX9860_DVA          0xfe
#define MAX9860_DVA_SHIFT    1
#define MAX9860_DVA_MUTE     0x5e

/* ADCLEVEL */
#define MAX9860_ADCRL        0xf0
#define MAX9860_ADCRL_SHIFT  4
#define MAX9860_ADCLL        0x0f
#define MAX9860_ADCLL_SHIFT  0
#define MAX9860_ADCxL_MIN    15

/* DACGAIN */
#define MAX9860_DVG          0x60
#define MAX9860_DVG_SHIFT    5
#define MAX9860_DVG_MAX      3                    }

#define MAX9860_DVST         0x1f
#define MAX9860_DVST_SHIFT   0
#define MAX9860_DVST_MIN     31

/* MICGAIN */
#define MAX9860_PAM          0x60
#define MAX9860_PAM_SHIFT    5
#define MAX9860_PAM_MAX      3
#define MAX9860_PGAM         0x1f
#define MAX9860_PGAM_SHIFT   0
#define MAX9860_PGAM_MIN     20

/* MICADC */
#define MAX9860_AGCSRC       0x80
#define MAX9860_AGCSRC_SHIFT 7
#define MAX9860_AGCSRC_COUNT 2
#define MAX9860_AGCRLS       0x70
#define MAX9860_AGCRLS_SHIFT 4
#define MAX9860_AGCRLS_COUNT 8
#define MAX9860_AGCATK       0x0c
#define MAX9860_AGCATK_SHIFT 2
#define MAX9860_AGCATK_COUNT 4
#define MAX9860_AGCHLD       0x03
#define MAX9860_AGCHLD_OFF   0x00
#define MAX9860_AGCHLD_SHIFT 0
#define MAX9860_AGCHLD_COUNT 4

/* NOISEGATE */
#define MAX9860_ANTH         0xf0
#define MAX9860_ANTH_SHIFT   4
#define MAX9860_ANTH_MAX     15
#define MAX9860_AGCTH        0x0f
#define MAX9860_AGCTH_SHIFT  0
#define MAX9860_AGCTH_MIN    15

/* PWRMAN */
#define MAX9860_SHDN         0x80
#define MAX9860_DACEN        0x08
#define MAX9860_DACEN_SHIFT  3
#define MAX9860_ADCLEN       0x02
#define MAX9860_ADCLEN_SHIFT 1
#define MAX9860_ADCREN       0x01
#define MAX9860_ADCREN_SHIFT 0



//
// I2C bus configuration
//
#ifndef CFG_MAX9860_SDA_PIN
#define CFG_MAX9860_SDA_PIN      25
#endif

#ifndef CFG_MAX9860_SCL_PIN
#define CFG_MAX9860_SCL_PIN      23
#endif


/** Initialize audiocodec */
int max9860_init(void);

/** Deinitialize audiocodec */
int max9860_deinit(void);

/** Write output samples */
int max9860_write_output(uint8_t *buf, int bufsize);

/** Read input samples */
int max9860_read_input(uint8_t *buf, int bufsize);


#endif /* _SND_SOC_MAX9860 */


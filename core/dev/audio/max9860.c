
#include "system.h"

#include "driver/i2c.h"
#include "driver/i2s.h"
#include "driver/ledc.h"
#include "esp_log.h"

#include "twi.h"
#include "max9860.h"

#define TRACE_TAG "voip-audio"
#if !ENABLE_TRACE_MAX9860
#undef TRACE
#define TRACE(...)
#endif


#define N      (((unsigned long)65536*96*CFG_MAX9860_SAMPLERATE)/CFG_MAX9860_MCLK_FREQ)	   // N = (65536*96*F_lrclk)/F_pclk
#define NHI    (N >> 8)					       // high 7 bits of N
#define NLO    (N & 0xFF)		               // NLO is lower 8 bits of N

// Prototypes:
static void mclk_disable(void);
static esp_err_t mclk_enable(uint32_t xclk_freq_hz, uint32_t pin_xclk);
static uint8_t max9860_read_reg(uint8_t reg);
static uint8_t max9860_write_reg(uint8_t reg, uint8_t data);

// Locals:
static uint8_t max9860_ready = 0;

static i2s_config_t max9860_i2s_config =
{
    .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
    .sample_rate = CFG_MAX9860_SAMPLERATE,
    .bits_per_sample = CFG_MAX9860_BITSPERSAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,
    .dma_buf_count = CFG_MAX9860_DMA_BUFCOUNT,
    .dma_buf_len = CFG_MAX9860_DMA_BUFLEN,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,          //Interrupt level 1
    .use_apll = 1,
    .fixed_mclk = CFG_MAX9860_MCLK_FREQ,
};

static i2s_pin_config_t max9860_pin_config =
{
    .bck_io_num = CFG_MAX9860_PIN_BCK,
    .ws_io_num = CFG_MAX9860_PIN_LRCLK,
    .data_out_num = CFG_MAX9860_PIN_DOUT,
    .data_in_num = CFG_MAX9860_PIN_DIN
};


int max9860_init(void)
{
   uint8_t reg;

   // Initialize I2C bus
   twi_init(CFG_MAX9860_SDA_PIN, CFG_MAX9860_SCL_PIN);

   // Read chip revision
   reg = max9860_read_reg(MAX9860_REVISION);
   if (reg != 0x40)
   {
      TRACE_ERROR("Read revision failed, reg=0x%X", reg);
      return -1;
   }

   TRACE("MAX9860 revision: 0x%x", reg);

   //
   // Initialize the MAX9860 by setting registers
   //

	// Shutdown
	max9860_write_reg(MAX9860_PWRMAN, 0x00);	   // powered all off

	// Clock control (MCLK = 10MHz)
	//max9860_write_reg(MAX9860_SYSCLK, 0x10);	      //PSCLK = 01, FREQ = 01, 16KHz = 1 (LRCLK = 16KHz)
	//max9860_write_reg(MAX9860_AUDIOCLKHIGH, NHI);	  // NHI
	//max9860_write_reg(MAX9860_AUDIOCLKLOW, NLO);    // NLO

	max9860_write_reg(MAX9860_SYSCLK, 0x10);	      //PSCLK = 01, FREQ = 01, 16KHz = 1 (LRCLK = 16KHz)
	max9860_write_reg(MAX9860_AUDIOCLKLOW, 0x01);    // NLO
	max9860_write_reg(MAX9860_AUDIOCLKHIGH, 0x80);	  //PLL = 1, NHI = 0

	// Registers at 0x06 and 0x07 are digital audio interface registers
	max9860_write_reg(MAX9860_IFC1A, 0x00);	   //MAS = 0, WCI = 0 (really not sure), DBCI = 0, DDLY = 0, HIZ = 0, TDM = 0,
	max9860_write_reg(MAX9860_IFC1B, 0x00);	   //ABCI = 0, ADLY = 0, ST = 0, BSEL = 000

	// Digital filters
	max9860_write_reg(MAX9860_VOICEFLTR, 0x00);	//AVFLT = 0, DVFLT = 0 (not sure which filter would be best for this, choosing no filter)

	// Digital level control registers
	max9860_write_reg(MAX9860_DACATTN, 0x06);	   //0 DAC adjustment, this would require testing and/or a better understanding of the overall system
	max9860_write_reg(MAX9860_ADCLEVEL, 0x30);
	max9860_write_reg(MAX9860_DACGAIN, 0x00);	   //no gain on DAC, unsure of DVST bits safer to disable

	// Microphone input register
	max9860_write_reg(MAX9860_MICGAIN, (1 << 6)| 0xA);

	// AGC and Noise gate registers
	max9860_write_reg(MAX9860_MICADC, 0x80);	   //sum of left & right noise gates for AGC & noise gate, AGCRLS shortest time, but AGC disabled since I am unsure of its need
	max9860_write_reg(MAX9860_NOISEGATE, 0x00);	   //Noise gate threshhold disabled, AGC signal threshhold -3bBFS

	// Power management register
	max9860_write_reg(MAX9860_PWRMAN, 0x8A);	   //powered  DAC on and leftADC on

    max9860_write_reg(MAX9860_DACATTN, 0x2E);       // Set volume

   // Initialize I2SCtA2013!
   if (i2s_driver_install(CFG_MAX9860_I2S_NUM, &max9860_i2s_config, 0, NULL) != ESP_OK)
   {
      TRACE_ERROR("I2s driver init failed");
      return -1;
   }

   if (i2s_set_pin(CFG_MAX9860_I2S_NUM, &max9860_pin_config) != ESP_OK)
   {
      TRACE_ERROR("I2s config pins failed");
      return -1;
   }

   // Enable MCLK output
   if (mclk_enable(CFG_MAX9860_MCLK_FREQ, CFG_MAX9860_MCLK_PIN) != ESP_OK)
   {
      TRACE_ERROR("mclk enable failed");
      return -1;
   }

   max9860_ready = 1;

   TRACE("Audiocodec init");

   return 0;
}

/** Deinitialize audiocodec */
int max9860_deinit(void)
{
    max9860_ready = 0;
    mclk_disable();
    i2s_stop(CFG_MAX9860_I2S_NUM);
    i2s_driver_uninstall(CFG_MAX9860_I2S_NUM);

    TRACE("Audiocodec deinit");

    return 0;
}

int max9860_write_output(uint8_t *buf, int bufsize)
{
    size_t bytes_written = 0;

    if (max9860_ready)
    {
        if (i2s_write(CFG_MAX9860_I2S_NUM, buf, bufsize, &bytes_written, portMAX_DELAY) != ESP_OK)
        {
            TRACE_ERROR("Write i2s output failed");
            return -1;
        }
    }

    return bytes_written;
}

int max9860_read_input(uint8_t *buf, int bufsize)
{
    size_t bytes_read = 0;

    if (max9860_ready)
    {
        if (i2s_read(CFG_MAX9860_I2S_NUM, buf, bufsize, &bytes_read, portMAX_DELAY) != ESP_OK)
        {
            TRACE_ERROR("Read i2S input failed");
            return -1;
        }
    }

    return bytes_read;
}

static void mclk_disable(void)
{
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    periph_module_disable(PERIPH_LEDC_MODULE);
}

static esp_err_t mclk_enable(uint32_t xclk_freq_hz, uint32_t pin_xclk)
{
    esp_err_t res;

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num  = LEDC_TIMER_0,
        .bit_num    = 1,
        .freq_hz    = xclk_freq_hz
    };

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .gpio_num   = pin_xclk,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 1
    };

    res = ledc_timer_config(&ledc_timer);
    if (res != ESP_OK)
    {
      TRACE_ERROR("ledc_timer_config failed, res=%x", res);
      return res;
    }

    res = ledc_channel_config(&ledc_channel);
    if (res != ESP_OK)
    {
      TRACE_ERROR("ledc_channel_config failed, res=%x", res);
      return res;
    }

    TRACE("Enable MCLK=%d Hz", xclk_freq_hz);

    return res;
}



//
// I2C interface for MAX9860
//

static uint8_t max9860_read_reg(uint8_t reg)
{
   uint8_t data = 0;

   int rc = twi_writeTo(MAX9860_ADDR, &reg, 1, false);
   if (rc != 0)
   {
      data = 0xff;
   }
   else
   {
      rc = twi_readFrom(MAX9860_ADDR, &data, 1, true);
      if (rc != 0)
      {
         data = 0xFF;
      }
   }


   if (rc != 0)
   {
      TRACE_ERROR("SCCB_Read [%02x] failed rc=%d", reg, rc);
   }

   return data;
}

static uint8_t max9860_write_reg(uint8_t reg, uint8_t data)
{
   uint8_t ret=0;
   uint8_t buf[] = {reg, data};

   if(twi_writeTo(MAX9860_ADDR, buf, 2, true) != 0)
   {
      ret = 0xFF;
   }

   if (ret != 0)
   {
      TRACE_ERROR("SCCB_Write [%02x]=%02x failed\n", reg, data);
   }

   return ret;
}

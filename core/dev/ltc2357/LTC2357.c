
#include <math.h>

#include "system.h"
#include "LTC2357.h"

TRACE_TAG(LTC2357);

#if defined(LTC2357_SPI)

#ifndef CFG_LTC2357_CONFIG_NUMBER
#define CFG_LTC2357_CONFIG_NUMBER       0x2
#endif


// Prototypes:
static void spi_recv_callback(hal_spi_t spi, const uint8_t *buf, int bufsize);
static void LTC2357_set_config_word(uint8_t channel, uint8_t config_number);

// Locals:
static uint32_t config_word = 0;
static __attribute__((section(".data"))) uint8_t chanbuf[LTC2357_NUM_CHANNELS * 3];


/** Initialize LTC2357 */
int LTC2357_init(void)
{
   int ic;

   hal_gpio_set(LTC2357_GPIO_PD, 0);    // Power ON
   hal_gpio_set(LTC2357_GPIO_CS, 1);    // SPI CS idle
   hal_gpio_set(LTC2357_GPIO_CNV, 0);   // Conversion disabled

   // Initialize SPI
   if (hal_spi_init(LTC2357_SPI) != 0)
   {
      TRACE_ERROR("Init SPI");
      return -1;
   }

   // Set config word for all channels
   for (ic = 0; ic < LTC2357_NUM_CHANNELS; ic++)
        LTC2357_set_config_word(ic, CFG_LTC2357_CONFIG_NUMBER);

   TRACE("Init, config number: 0x%X  word: 0x%X", CFG_LTC2357_CONFIG_NUMBER, config_word);

   return 0;
}

/** Read one sample */
int LTC2357_read_samples(LTC2357_samples_t *psamples)
{
   int ix;
   uint8_t c;
   uint16_t sample;
   uint8_t chan, span;
  
   // Set config word to output buffer
   chanbuf[0] = (uint8_t)(config_word >> 16);
   chanbuf[1] = (uint8_t)(config_word >> 8);
   chanbuf[2] = (uint8_t)(config_word);
   for (ix = 3; ix < sizeof(chanbuf); ix++)
      chanbuf[ix] = 0;

   // Start conversion
   hal_gpio_set(LTC2357_GPIO_CNV, 1);
   hal_gpio_set(LTC2357_GPIO_CNV, 0);

   // Wait for conversion finished
   while(hal_gpio_get(LTC2357_GPIO_BUSY) == 1);

   hal_spi_select(LTC2357_SPI, LTC2357_GPIO_CS);

   // Read four channels samples
   hal_spi_read(LTC2357_SPI, chanbuf, sizeof(chanbuf));

   hal_spi_deselect(LTC2357_SPI, LTC2357_GPIO_CS);

   // Parse samples buffer
   for (ix = 0; ix < sizeof(chanbuf); ix++)
   {
      // Read 16 bit sample
      sample = chanbuf[ix++] << 8;
      sample |= chanbuf[ix++];

      // Read 8bit - SOFTSPAN + CHANID
      c = chanbuf[ix];
      span = c & 0x7;
      chan = (c >> 3) & 0x03;
      //TRACE("%2.2X  span: %d  chan: %d   sample: %d", c, span, chan, sample);

      if (chan < LTC2357_NUM_CHANNELS)
         psamples->channel[chan] = sample;
   }

   return 0;
}


static void LTC2357_set_config_word(uint8_t channel, uint8_t config_number)
{
  config_word = config_word | ((uint32_t)(config_number & 0x07) << (channel * 3));
}

#else

#define PI                    3.14159265358979323846
#define SIGNAL_SAMPLERATE     48000
#define SIGNAL_FREQ           16000
#define SIGNAL_AMP            0.8


int LTC2357_init(void)
{
   TRACE("ADC dummy initialized");
   return 0;
}

int LTC2357_read_samples(LTC2357_samples_t *ps)
{
   static float time_f = 0;
   int chan, sample;

   // Generate 16bit sin wave
   sample = (SIGNAL_AMP * 0x8000 * sin(2 * PI * SIGNAL_FREQ * time_f)) + 0x8000;
   time_f += 1.0 / SIGNAL_SAMPLERATE;  

   for (chan = 0; chan < LTC2357_NUM_CHANNELS; chan++) 
      ps->channel[chan] = sample;

   return 0;
}

#endif   // LTC2357_SPI





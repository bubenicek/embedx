
#include <math.h>
#include "system.h"
#include "emon.h"

TRACE_TAG(emon);

#define ADC_COUNTS                (1 << CFG_EMON_ADC_BITS)

static double ICAL;              // calibration factor
static double offsetI;           

/** Initialize energy monitor */
int emon_init(double icalib)
{
   ICAL = icalib;
   offsetI = ADC_COUNTS >> 1;

   return 0;
}

/** Read and calculate current RMS */
int emon_calc_irms(hal_adc_t adc, int nsamples, double *irms)
{
   int n;
   int sample;
   double filteredI;
   double sqI, sumI = 0;

   for (n = 0; n < nsamples; n++)
   {
      hal_adc_read(adc, &sample);
    
      // Digital low pass filter extracts the 2.5 V or 1.65 V dc offset,
      // then subtract this - signal is now centered on 0 counts.
      offsetI = (offsetI + (sample - offsetI) / 1024);
      filteredI = sample - offsetI;

      // Root-mean-square method current
      // 1) square current values
      sqI = filteredI * filteredI;
      
      // 2) sum
      sumI += sqI;
   }

   double I_RATIO = ICAL *((CFG_EMON_SUPPLY_VOLTAGE / 1000.0) / (ADC_COUNTS));
   *irms = I_RATIO * sqrt(sumI / nsamples);

   return 0;
}

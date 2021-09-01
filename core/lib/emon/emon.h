/**
 * \file emon.h      \brief Enery monitor
 */

#ifndef __EMON_H
#define __EMON_H

#ifndef CFG_EMON_SUPPLY_VOLTAGE
#define CFG_EMON_SUPPLY_VOLTAGE        3300
#endif

#ifndef CFG_EMON_ADC_BITS
#define CFG_EMON_ADC_BITS              12
#endif

/** Initialize energy monitor */
int emon_init(double icalib);

/** Read and calculate current RMS */
int emon_calc_irms(hal_adc_t adc, int nsamples, double *irms);

#endif   // __EMON_H
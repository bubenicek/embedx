
#ifndef __HAL_ADC_H
#define __HAL_ADC_H

typedef enum
{
   HAL_ADC0,
   HAL_ADC1,
   HAL_ADC2,
   HAL_ADC3,
   HAL_ADC4,
   HAL_ADC5

} hal_adc_t;

/** Initialaze ADC */
int hal_adc_init(hal_adc_t dev);

/** Deinitialize ADC */
int hal_adc_deinit(hal_adc_t dev);

/** Read raw value from ADC */
int hal_adc_read(hal_adc_t dev, int *value);

#endif // __HAL_ADC_H

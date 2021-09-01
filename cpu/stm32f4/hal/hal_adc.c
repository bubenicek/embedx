
#include "system.h"
#include "tm_stm32f4_adc.h"

TRACE_TAG(hal_adc);
#if ! ENABLE_TRACE_HAL
#include "trace_undef.h"
#endif

// Locals:
static const hal_adc_def_t hal_adc_def[] = CFG_HAL_ADC_DEF;
#define NUM_ADC   (sizeof(hal_adc_def) / sizeof(hal_adc_def_t))


/** Initialaze ADC */
int hal_adc_init(hal_adc_t adc)
{
   ASSERT(adc < NUM_ADC);

   TM_ADC_Init(hal_adc_def[adc].dev, hal_adc_def[adc].channel);

   TRACE("ADC[%d] init, channel: %d", adc, hal_adc_def[adc].channel);

   return 0;
}

/** Deinitialize ADC */
int hal_adc_deinit(hal_adc_t adc)
{
   ASSERT(adc < NUM_ADC);

   return 0;
}

/** Read raw value from ADC */
int hal_adc_read(hal_adc_t adc, int *value)
{
   ASSERT(adc < NUM_ADC);

   *value = TM_ADC_Read(hal_adc_def[adc].dev, hal_adc_def[adc].channel);

   return 0;
}

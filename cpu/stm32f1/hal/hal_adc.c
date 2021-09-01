
#include "system.h"

TRACE_TAG(hal_adc);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

ADC_HandleTypeDef hadc1;

/** Initialaze ADC */
int hal_adc_init(hal_adc_t dev)
{
   GPIO_InitTypeDef  GPIO_InitStruct;
   ADC_ChannelConfTypeDef sConfig;

   // Peripheral clock enable
   __HAL_RCC_ADC1_CLK_ENABLE();

   // ADC1 GPIO Configuration    
   // PA0-WKUP     ------> ADC1_IN0
   // PA1          ------> ADC1_IN1 
   GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
   GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


   // Common config 
   hadc1.Instance = ADC1;
   hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
   hadc1.Init.ContinuousConvMode = DISABLE;
   hadc1.Init.DiscontinuousConvMode = DISABLE;
   hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
   hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
   hadc1.Init.NbrOfConversion = 1;
   HAL_ADC_Init(&hadc1);

   // Configure Regular Channel 
   sConfig.Channel = ADC_CHANNEL_0;
   sConfig.Rank = 1;
   sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5; //ADC_SAMPLETIME_1CYCLE_5;
   HAL_ADC_ConfigChannel(&hadc1, &sConfig);
   
   return 0;
}

/** Deinitialize ADC */
int hal_adc_deinit(hal_adc_t dev)
{
   // Peripheral clock disable
   __HAL_RCC_ADC1_CLK_DISABLE();

   HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

   return 0;
}

/** Read raw value from ADC */
int hal_adc_read(hal_adc_t dev, int *value)
{
   int res = -1;

   HAL_ADC_Start(&hadc1);
   
   if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
   {
      *value = HAL_ADC_GetValue(&hadc1);
      res = 0;
   }
   
   HAL_ADC_Stop(&hadc1);

   return res;
}

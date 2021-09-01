/**
 * \file hal_can.c     \brief CAN hal driver
 */

#include "system.h"

TRACE_TAG(hal_can);
#if !ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

#ifndef CFG_HAL_CAN_PRIORITY
#define CFG_HAL_CAN_PRIORITY     0
#endif

/** CAN device */
typedef struct
{
   CAN_HandleTypeDef hcan;
   const hal_can_def_t *def;
   hal_can_t can;
   hal_can_recv_cb_t recv_cb;
   
} hal_can_device_t;

// Prototypes:
static hal_can_device_t *hal_can_get_device(CAN_TypeDef *instance);

// Locals:
static const hal_can_def_t hal_can_def[] = CFG_HAL_CAN_DEF;
#define NUM_CANS   (sizeof(hal_can_def) / sizeof(hal_can_def_t))

static hal_can_device_t hal_can_dev[NUM_CANS];

/** Open CAN */
int hal_can_init(hal_can_t can)
{
   int ix;
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   CAN_FilterTypeDef sFilterConfig;
   hal_can_device_t *dev;

   ASSERT(can < NUM_CANS);   
   dev = &hal_can_dev[can];
   dev->def = &hal_can_def[can];
   dev->can = can;
   dev->recv_cb = NULL;

   if (dev->def->instance == CAN1)      
      __HAL_RCC_CAN1_CLK_ENABLE();
   else if (dev->def->instance == CAN2)      
      __HAL_RCC_CAN2_CLK_ENABLE();
   else
   {
      TRACE_ERROR("Not supported CAN instance");
      return -1;
   }  

   // CAN GPIO Configuration    
   GPIO_InitStruct.Pin = dev->def->gpio_rx.pin;
   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
   GPIO_InitStruct.Pull = GPIO_NOPULL;
   HAL_GPIO_Init(dev->def->gpio_rx.port, &GPIO_InitStruct);

   GPIO_InitStruct.Pin = dev->def->gpio_tx.pin;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
   HAL_GPIO_Init(dev->def->gpio_tx.port, &GPIO_InitStruct);

   if (dev->def->remap_can1_2)
      __HAL_AFIO_REMAP_CAN1_2();

   dev->hcan.Instance = dev->def->instance;
   dev->hcan.Init.Prescaler = dev->def->prescaler;
   dev->hcan.Init.Mode = CAN_MODE_NORMAL;
   dev->hcan.Init.SyncJumpWidth = dev->def->sync_jump_width; 
   dev->hcan.Init.TimeSeg1 = dev->def->time_seg1;
   dev->hcan.Init.TimeSeg2 = dev->def->time_seg2;
   dev->hcan.Init.TimeTriggeredMode = DISABLE;
   dev->hcan.Init.AutoBusOff = DISABLE;
   dev->hcan.Init.AutoWakeUp = DISABLE;
   dev->hcan.Init.AutoRetransmission = DISABLE;
   dev->hcan.Init.ReceiveFifoLocked = DISABLE;
   dev->hcan.Init.TransmitFifoPriority = DISABLE;
   if (HAL_CAN_Init(&dev->hcan) != HAL_OK)
   {
      TRACE_ERROR("Can init failed");
      return -1;
   }

   // CAN1 interrupt Init 
   HAL_NVIC_SetPriority(CAN1_TX_IRQn, CFG_HAL_CAN_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
   HAL_NVIC_SetPriority(CAN1_RX0_IRQn, CFG_HAL_CAN_PRIORITY, 0);
   HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

   // Configure filter for receiving all packets
   for (ix = 0; ix < CFG_HAL_CAN_NUM_FILTERS; ix++)
   {
      sFilterConfig.FilterBank = ix;
      sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
      sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
      sFilterConfig.FilterIdHigh = (uint16_t) (dev->def->filter[ix].id << 5);
      sFilterConfig.FilterIdLow = 0x0000;
      sFilterConfig.FilterMaskIdHigh = (uint16_t) (dev->def->filter[ix].mask << 5);
      sFilterConfig.FilterMaskIdLow = 0x0000;
      sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
      sFilterConfig.FilterActivation = ENABLE;
      sFilterConfig.SlaveStartFilterBank = 14;
      if (HAL_CAN_ConfigFilter(&dev->hcan, &sFilterConfig) != HAL_OK)
      {
         TRACE_ERROR("Configure filter failed");
         return -1;
      }
   }

   if (HAL_CAN_Start(&dev->hcan) != HAL_OK)
   {
      TRACE_ERROR("Can start failed");
      return -1;
   }

   if (HAL_CAN_ActivateNotification(&dev->hcan, CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
   {
      TRACE_ERROR("HAL_CAN_ActivateNotification failed");
      return -1;
   }

   TRACE("Init CAN[%d] device", can);

   return 0;
}

/** Close CAN */
int hal_can_deinit(hal_can_t can)
{
   ASSERT(can < NUM_CANS);   

   return 0;
}

/** Write buffer */
int hal_can_write(hal_can_t can, uint32_t id, void *buf, uint16_t count)
{
   CAN_TxHeaderTypeDef hdr;
   uint32_t mailbox;

   ASSERT(can < NUM_CANS);   

   hdr.StdId = id;
   hdr.ExtId = 0x01;
   hdr.RTR = CAN_RTR_DATA;
   hdr.IDE = CAN_ID_STD;
   hdr.DLC = count; 
   hdr.TransmitGlobalTime = DISABLE;

   if (HAL_CAN_AddTxMessage(&hal_can_dev[can].hcan, &hdr, buf, &mailbox) != HAL_OK)
   {
      TRACE_ERROR("Can add tx message failed");
      return -1;
   }

   return count;
}

/** Register receive callback */
int hal_can_recv(hal_can_t can, hal_can_recv_cb_t _recv_cb)
{
   ASSERT(can < NUM_CANS);   

   DISABLE_INTERRUPTS();
   hal_can_dev[can].recv_cb = _recv_cb;
   ENABLE_INTERRUPTS();

   return 0;
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
   int ix;
   CAN_RxHeaderTypeDef hdr;
   uint8_t data[8];
   hal_can_device_t *dev;

   if ((dev = hal_can_get_device(hcan->Instance)) != NULL)
   {  
      HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hdr, data);

      if (dev->recv_cb != NULL)
         dev->recv_cb(dev->can, hdr.StdId, data, hdr.DLC);
   }
}

static hal_can_device_t *hal_can_get_device(CAN_TypeDef *instance)
{
   for (int ix = 0; ix < NUM_CANS; ix++)
   {
      if (hal_can_dev[ix].def->instance == instance)
         return &hal_can_dev[ix];
   }
   
   return NULL;
}

#define CAN_IRQ_HANDLER(can)              \
{                                         \
   static hal_can_device_t *dev = NULL;   \
   if (dev == NULL)                       \
      dev = hal_can_get_device(can);      \
   if (dev != NULL)                       \
      HAL_CAN_IRQHandler(&dev->hcan);     \
}

/** This function handles CAN1 TX interrupt. */
void CAN1_TX_IRQHandler(void)
{
   CAN_IRQ_HANDLER(CAN1);
}

/** This function handles CAN1 RX0 interrupt. */
void CAN1_RX0_IRQHandler(void)
{
   CAN_IRQ_HANDLER(CAN1);
}

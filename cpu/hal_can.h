
#ifndef __HAL_CAN_H
#define __HAL_CAN_H

#define HAL_CAN_MSGSIZE    8

typedef enum
{
   HAL_CAN0,
   HAL_CAN1

} hal_can_t;

/** Recv char callback */
typedef void (*hal_can_recv_cb_t)(hal_can_t can, uint32_t std_id, uint8_t *buf, int bufsize);


/** Open CAN */
int hal_can_init(hal_can_t can);

/** Close CAN */
int hal_can_deinit(hal_can_t can);

/** Write CAN message */
int hal_can_write(hal_can_t can, uint32_t std_id, void *buf, uint16_t count);

/** Register receive CAN message callback */
int hal_can_recv(hal_can_t can, hal_can_recv_cb_t recv_cb);

#endif  // __HAL_CAN_H
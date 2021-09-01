
#ifndef __HAL_UUID_H
#define __HAL_UUID_H

/** Get unique board ID */
uint32_t hal_get_board_uuid32(void);

/** Generate uuid board string */
char *hal_get_board_uuid(char *uuid, int bufsize);

#endif // __HAL_UUID_H
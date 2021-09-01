
#ifndef _MODBUS_H_
#define _MODBUS_H_

#define MODBUS_TCP_PORT                      502
#define MODBUS_TCP_HEADER_SIZE               7

#define MODBUS_TCP_ADDR_IDX                  6
#define MODBUS_TCP_FUNC_IDX                  7
#define MODBUS_TCP_DATA_IDX                  8

#define MODBUS_RTU_ADDR_IDX                  0
#define MODBUS_RTU_FUNC_IDX                  1
#define MODBUS_RTU_DATA_IDX                  2

#define MODBUS_FUNC_READ_COILS               0x01
#define MODBUS_READ_DISCRETE_INPUTS          0x02
#define MODBUS_READ_HOLDING_REGISTER         0x03
#define MODBUS_FUNC_WRITE_COIL               0x05
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER    0x06

#define CFG_MODBUS_MAX_HOLDING_REGS_COUNT    125


/** RTU recv request callback */
typedef int (*modbus_rtu_recv_cb_t)(hal_uart_t uart, uint8_t addr, uint8_t func, uint16_t regaddr, uint16_t regdata);

/** Initialize MODBUS RTU */
int modbus_rtu_init(hal_uart_t uart, uint32_t baudrate, uint32_t uart_settings, uint16_t recv_timeout);

/** Register RTU recv request callback */
int modbus_rtu_recv(hal_uart_t uart, uint8_t addr, modbus_rtu_recv_cb_t cb);

/** Write RTU request */
int modbus_rtu_write_request(hal_uart_t uart, int addr, uint8_t func, uint16_t regaddr, uint16_t regdata);

/** Write RTU response */
int modbus_rtu_write_response(hal_uart_t uart, int addr, uint8_t func, uint8_t *data, int datalen);

/** Read RTU response */
int modbus_rtu_read_response(hal_uart_t uart, int addr, int rspsize, uint8_t *buf, int bufsize);

/** Read RTU coils state (0x1) */
int modbus_rtu_read_coils_state(hal_uart_t uart, int addr, int start_coil, int num_coils, uint16_t *state);

/** Write RTU coil state (0x5)*/
int modbus_rtu_write_coil(hal_uart_t uart, int addr, int coil, int state);

/** Read RTU discrete inputs state (0x2) */
int modbus_rtu_read_inputs(hal_uart_t uart, int addr, int start_input, int count, uint16_t *state);

/** Write single register (0x6) */
int modbus_rtu_write_sigle_register(hal_uart_t uart, int addr, int regaddr, int value);

/** Read holding registers (0x3) */
int modbus_rtu_read_holding_register(hal_uart_t uart, int addr, int start, int count, uint16_t *regs);


#endif /* _MODBUS_RTU_H_ */

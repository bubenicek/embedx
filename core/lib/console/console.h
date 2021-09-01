/**
 * \file console.h          \brief Serial port control console
 */

#ifndef __console_h
#define __console_h


/** Size of console input buffer */
#ifndef CFG_CONSOLE_BUFIN_SIZE
#define CFG_CONSOLE_BUFIN_SIZE          64
#endif


/** Max number of argc paramas */
#ifndef CFG_CONSOLE_MAX_ARGC
#define CFG_CONSOLE_MAX_ARGC            16
#endif


#ifndef CFG_CONSOLE_PROMPT
#define CFG_CONSOLE_PROMPT             "> "
#endif


/** Console process states */
typedef enum
{
    CONSOLE_STATE_INIT,
    CONSOLE_STATE_RCV_CHAR,
    CONSOLE_STATE_PROCESS_CMD,

} console_state_t;


/** Console process context */
typedef struct
{
    /** Console process state */
    console_state_t state;

    /** Console input buffer */
    uint8_t bufin[CFG_CONSOLE_BUFIN_SIZE];

    /** Index to bufin buffer */
    uint16_t bufin_index;

    uint16_t prev_bufin_index;
    int prev_argc;

    struct
    {
        int argc;
        char *argv[CFG_CONSOLE_MAX_ARGC];

    } params;

} console_t;


/** Console command */
typedef struct
{
    char *name;
    void (*cmd_func)(console_t *con, const char *cmd_name, int argc, char *argv[]);

} console_cmd_t;


/** Console commands definition */
extern const console_cmd_t console_cmds[];


/** Initialize console process */
int console_init(console_t *s);

/** Console processing task */
void console_task(console_t *s);

/** output formated text */
void console_printf(console_t *s, const char *fmt, ...);

#define console_debug(_con, _format, ...) console_printf(_con, _format "\r\n", ## __VA_ARGS__)

#define console_error(_con, _format, ...) do {\
    console_printf(_con, "ERROR - "_format "\r\n", ## __VA_ARGS__); \
} while(0)


//
// Console driver funcs
//

#ifndef console_driver_init
#error "Not defined console_driver"
#endif

#ifndef console_putchar
#error "Not defined console driver"
#endif

#ifndef console_getchar
#error "Not defined console driver"
#endif

#endif   // console.h

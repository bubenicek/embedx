/**
 * \file ln_console.c          \brief Serial port control console
 */

#include <string.h>
#include <stdarg.h>  
#include "system.h"
#include "console.h"

#define KEY_ENTER       13
#define KEY_ESC         27
#define KEY_SPACE       32
#define KEY_BKSPACE     8


// Prototypes:
static void console_send_int(console_t *s, int32_t num, uint8_t signed_value);
static void console_send_hex(console_t *s, int32_t value, uint8_t upper_case);
static void console_send_text(console_t *s, const unsigned char *pucBuffer);

//--------------------------------------------------------------------------------------------

/** initialize console process */
int console_init(console_t *s)
{
    memset(s, 0, sizeof(console_t));

    // Intialize console input/output
    console_driver_init(s);
    s->state = CONSOLE_STATE_INIT;

#if defined(CFG_OPENOS_OS_API) && (CFG_OPENOS_OS_API == 1)
    os_scheduler_push_task((os_task_cbt)console_task, OS_TASKPRIO_MAX, s);
#endif
    
    return 0;
}

/** Console processing task */
void console_task(console_t *s)
{
    int c;

    switch(s->state)
    {
         // Initialize reading of line
        case CONSOLE_STATE_INIT:            
        
            // Send prompt
            console_printf(s, CFG_CONSOLE_PROMPT);
        
            s->bufin_index = 0;
            s->params.argc = 0;
            s->params.argv[0] = (char *)s->bufin;
            s->state = CONSOLE_STATE_RCV_CHAR;
            break;

         // Receive chars until CR or ESC received
        case CONSOLE_STATE_RCV_CHAR:        
            if ((c = console_getchar(s)) != -1)
            {
               // echo char
               console_putchar(s, c);

               if (c == KEY_BKSPACE)
                {
                    if (s->bufin_index > 0)
                        s->bufin_index--;

                    console_putchar(s, KEY_SPACE);
                    console_putchar(s, KEY_BKSPACE);
                }
                else if (c == KEY_ESC)
                {
                    // clear command buffer
                    s->params.argc = 1;
                    *s->bufin = 0;
                    s->state = CONSOLE_STATE_PROCESS_CMD;
                }
                else if (c == KEY_ENTER)
                {
                    // confirm command
                    if (++s->params.argc > CFG_CONSOLE_MAX_ARGC)
                    {
                        console_error(s, "params buffer overflow");
                        s->state = CONSOLE_STATE_INIT;
                    }
                    else
                    {
                        if (s->bufin_index == 0)
                        {
                            s->bufin_index = s->prev_bufin_index;
                            s->params.argc = s->prev_argc;
                        }

                        s->bufin[s->bufin_index] = 0;
                        s->prev_bufin_index = s->bufin_index;
                        s->prev_argc = s->params.argc;

                        s->state = CONSOLE_STATE_PROCESS_CMD;
                    }
                }
                else
                {
                    if (s->bufin_index < CFG_CONSOLE_BUFIN_SIZE-1)
                    {
                        if (c == KEY_SPACE)
                        {
                            s->bufin[s->bufin_index++] = 0;
                            if (++s->params.argc > CFG_CONSOLE_MAX_ARGC)
                            {
                                console_error(s, "params buffer overflow");
                                s->state = CONSOLE_STATE_INIT;
                            }
                            else
                            {
                                s->params.argv[s->params.argc] = (char *)&s->bufin[s->bufin_index];
                            }
                        }
                        else
                        {
                            s->bufin[s->bufin_index++] = c;
                        }
                    }
                    else
                    {
                        console_error(s, "line buffer overflow");
                        s->state = CONSOLE_STATE_INIT;
                    }
                }
            }
            break;

        case CONSOLE_STATE_PROCESS_CMD:         // process received command
        {
            const console_cmd_t *cmd = NULL;

            // find defined command
            for (cmd = console_cmds; cmd->name != NULL; cmd++)
            {
                if (strcmp(s->params.argv[0], cmd->name) == 0)
                    break;
            }

            console_printf(s, "\r\n");

            // execute command handler
            if (cmd->name != NULL)
            {
                cmd->cmd_func(s, cmd->name, s->params.argc, s->params.argv);
            }
            else
            {
                console_error(s, "Command not found");
            }

            s->state = CONSOLE_STATE_INIT;
        }
        break;
    }

#if defined(CFG_OPENOS_OS_API) && (CFG_OPENOS_OS_API == 1)
    os_scheduler_push_task((os_task_cbt)console_task, OS_TASKPRIO_MAX, s);
#endif
}

/** output formated text */
void console_printf(console_t *s, const char *string, ...)
{
	char arg_char;
	int arg_int;
	int32_t arg_long;
	char* arg_string;
	va_list p;

	va_start(p, string);

	while (*string != '\0')
	{
		if (*string == '%')
		{
			switch(*(++string))
			{
				case 'd':
					arg_int = va_arg(p, int);
					console_send_int(s, (int64_t)arg_int, 1);
					string++;
					break;

				case 'l':
					arg_long = va_arg(p, int64_t);
					console_send_int(s, arg_long, 1);
					string++;
					break;

				case 'u':
					arg_long = va_arg(p, int64_t);
					console_send_int(s, arg_long, 0);
					string++;
					break;

				case 's':
					arg_string = va_arg(p, char*);
					console_send_text(s, (const unsigned char*) arg_string);
					string++;
					break;

				case 'x':
				case 'X':
					arg_int = va_arg(p, int);
					console_send_hex(s, arg_int, 'x' - (*string));
					string++;
					break;

				case 'c':
					arg_char = va_arg(p, int);
					console_putchar(s, arg_char);
					string++;
					break;

				default:
					console_putchar(s, *string++);
					break;
			}
		}
		else
		{
         console_putchar(s, *string++);
		}
	}

	va_end(p);
}

static void console_send_int(console_t *s, int32_t num, uint8_t signed_value)
{
	uint8_t temp[25];
	uint8_t i = 0;
	uint8_t neg_flag = 0;
	uint32_t num_positive = num;
	uint16_t data_to_send;


	if (num < 0 && signed_value == 1)
	{
		neg_flag = 1;
		num_positive = -num;
	}

	do
	{
		temp[i] = '0' + num_positive % 10;
		i++;
		num_positive = num_positive / 10;
	}
	while (num_positive > 0);

	if (neg_flag)
	{
		data_to_send = '-';
		console_putchar(s, data_to_send); //send "minus" character
	}

	do
	{
		console_putchar(s, temp[i - 1]);	//send a numeric character
		i--;
	}
	while (i > 0);
}

static void console_send_hex(console_t *s, int32_t value, uint8_t upper_case)
{
	uint8_t non_zero = 0;		//signalizes that a non-zero digit has already been obtained
	uint32_t mask = 0x0000000F;	//masks value to get appropriate bits for conversion
	int8_t  shift;				//shift the "value" bits
	uint8_t offset = 0;			//ofset for the non-numeric characters (Lower/Upper case)
	uint8_t bits;				//currently read bits of "value"
	uint16_t data_to_send;

	if (!upper_case)
	{
		offset = 'a' - 'A';
	}

	for (shift = 7; shift >= 0; shift--)
	{
		bits = (uint8_t)((value>>(4*shift)) & mask);

		if (bits != 0x00) non_zero = 1;

		if (non_zero)
		{
			if (bits < 10)
			{
				data_to_send = bits + '0';
				console_putchar(s, data_to_send); //send a numeric character
			}
			else {
				data_to_send = bits - 10 + 'A' + offset;
				console_putchar(s, data_to_send); //send a non-numeric character
			}
		}
	}

	if (non_zero == 0)
	{
		data_to_send = '0';
		console_putchar(s, data_to_send); //send zero
	}
}

static void console_send_text(console_t *s, const unsigned char *pucBuffer)
{
	// Loop while there are more characters to send.
	while (*pucBuffer != '\0')
	{
		// Write the next character to the UART queue.
		console_putchar(s, *pucBuffer);
		pucBuffer++;
	}
}


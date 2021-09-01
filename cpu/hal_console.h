/**
 * \file hal_console.h
 *
 * \brief Real Time Locating System - HAL console driver
 *
 * Copyright (c) 2015-2016 Robert Bubenicek, MacTechCity
 *
 */

#ifndef __HAL_CONSOLE_H
#define __HAL_CONSOLE_H

/** Initialize console */
int hal_console_init(void);

/** Put char to console */
void hal_console_putchar(char c);

/** Get char from console */
int hal_console_getchar(void);

/** Flush console output */
void hal_console_flush(void);

#endif // __HAL_CONSOLE_H

/**
 * \file hal_time.h
 *
 * \brief Real Time Locating System - HAL SPI driver
 *
 * Copyright (c) 2015-2016 Robert Bubenicek, MacTechCity
 *
 */

#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#include "hal_gpio.h"

/** SPI device types */
typedef enum
{
	HAL_SPI0,
	HAL_SPI1,
	HAL_SPI2,
	HAL_SPI3

} hal_spi_t;

/** Nonblocking recv data callback */
typedef void (*hal_spi_recv_cb_t)(hal_spi_t spi, const uint8_t *buf, int bufsize);

/** Nonblocking sent data callback */
typedef void (*hal_spi_sent_cb_t)(hal_spi_t spi, const uint8_t *buf, int bufsize);


/** Initialise SPI */
int hal_spi_init(hal_spi_t spi);

int hal_spi_deinit(hal_spi_t spi);

/** Select SPI device */
#define hal_spi_select(_spi, _gpio_cs) \
   hal_gpio_set(_gpio_cs, 0)

/** Deselect SPI device */
#define hal_spi_deselect(_spi, _gpio_cs) \
   hal_gpio_set(_gpio_cs, 1)

/** Set SPI speed */
int hal_spi_set_speed(hal_spi_t spi, uint32_t speed);


/** Transmit byte */
int hal_spi_transmit(hal_spi_t spi, uint8_t c);

/** Send buffer to SPI */
int hal_spi_write(hal_spi_t spi, const uint8_t *buf, int bufsize);

/** Send burst buffer to SPI with cs state */
int hal_spi_write_burst(hal_spi_t spi, const uint8_t *buf, int bufsize, bool last);


/** Read buffer from SPI */
int hal_spi_read(hal_spi_t spi, uint8_t *buf, int bufsize);


/** Enable/disable receive */
int hal_spi_recv_enable(hal_spi_t spi, bool enable);

/** Clear recv buffer */
int hal_spi_recv_clear(hal_spi_t spi);


/** Register callback for nonblocking receive data */
int hal_spi_recv(hal_spi_t spi, hal_spi_recv_cb_t recv_cb);

/** Register callback for nonblocking sent data */
int hal_spi_sent(hal_spi_t spi, hal_spi_sent_cb_t sent_cb);


#endif // __HAL_SPI_H

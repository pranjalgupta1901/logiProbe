/*******************************************************************************
 * Copyright (C) 2023 by PRANJAL GUPTA
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. PRANJAL GUPTA and the University of Colorado are not liable for
 * any misuse of this material.
 * ****************************************************************************/

/**
 * @file    spi.h
 * @brief   This file contains the function prototypes for spi and its gpio initialisation along with read and write
 * 			fucntions
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */

#ifndef SRC_SPI_H_
#define SRC_SPI_H_

#include "stdint.h"

void spi_transmit_buffer(uint8_t *buffer, uint8_t len);
void spi_transmit_data(uint8_t data);
uint8_t spi_read_data();
void spi_write_read_data(uint8_t *write_data, uint8_t *read_data, uint8_t len);
void spi_init(void);
void spi_gpio_pin_init(void);
void gpio_set_cs_low();
void gpio_set_cs_high();

#endif /* SRC_SPI_H_ */

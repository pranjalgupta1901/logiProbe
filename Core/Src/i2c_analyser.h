/*******************************************************************************
 * Copyright (C) 2023 by Krish Shah
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Krish Shah and the University of Colorado are not liable for
 * any misuse of this material.
 * ****************************************************************************/

/**
 * @file    i2c_analyser.h
 * @brief   Header file for I2C interpreter code. It goes over a given buffer, where data is 8bit format
 * 			and certain pins are selected as SDA and SCL by calling code.
 *
 * 			It can work on 7bit i2c address.
 *
 * 			It can detect:
 * 			START
 * 			RESTART
 * 			STOP
 * 			Slave Address
 * 			R/W
 * 			ACK/NACK
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#ifndef __I2C_ANALYSER_H__
#define __I2C_ANALYSER_H__
#include "stdint.h"

/*
 * Function to run i2c analyzer task, on a given buffer with given scl and sda bit positions
 *
 * Parameters:
 *  buffer pointer to byte array containing samples
 *  buf_len length of byte array containing samples
 *  scl_pos position of scl signal in sample
 *  sda_pos position of sda signal in sample
 *
 * Returns:
 *  none
 */
void run_analyser(uint8_t buffer[],uint32_t buf_len,uint8_t scl_pos,uint8_t sda_pos);

/*
 *	Function to the run the analyzer on a sample array.
 *	To run the function, uncomment:
 *	#define TESTING
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void test_analyser();

#endif

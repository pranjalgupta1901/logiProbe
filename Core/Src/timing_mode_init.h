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
 * @file    timing_mode_init.h
 * @brief   This function contains the prototype related to the functions of timings mode defined in timing_mode_init.c
 * 			file, it also contains the enum for the sampling frequency accroding to timer can be configured
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */

#ifndef SRC_TIMING_MODE_INIT_H_
#define SRC_TIMING_MODE_INIT_H_

#define TIMING_MODE 1
#define STATE_MODE 2

#include "stdbool.h"
#include "stdint.h"

typedef enum{
	FREQ_100KHz = 0,
	FREQ_200KHz,
	FREQ_400KHz,
	FREQ_800KHz,
	FREQ_1000KHz
}timing_mode_freq_t;

extern int freq_table_len;
extern char* freq_table[5];

bool timing_mode_init(uint8_t mode, timing_mode_freq_t freq, bool is_i2c_asked, uint16_t count);

#endif /* SRC_TIMING_MODE_INIT_H_ */

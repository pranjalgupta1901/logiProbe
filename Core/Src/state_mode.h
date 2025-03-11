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
 * @file    spit.h
 * @brief   This file contains the function prototypes for initialising state mode and contains the macros
 * 			related to modes in state mode and also enum for the configuration of input capture edges
 *
 * @author  Pranjal Gupta and Krish Shah
 * @date    12/17/2023
 *
 *
 *
 */

#ifndef SRC_STATE_MODE_H_
#define SRC_STATE_MODE_H_

#define TRIG_MODE 1
#define BUTTON_MODE 2

#include "stdint.h"
#include "stdbool.h"

typedef enum{
	RISING_EDGE = 0,
	FALLING_EDGE,
	RISING_FALLING_EDGE
}input_capture_edge_t;

bool state_timing_init(input_capture_edge_t edge, uint8_t mode, uint8_t pattern, uint16_t count, uint8_t pin_num, uint32_t time_count);


#endif /* SRC_STATE_MODE_H_ */

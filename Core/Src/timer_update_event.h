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
 * @file    timer_update_event.h
 * @brief   This file contains the function prototypes for timer update events, flags and enable and disables of dma
 * 			timers.
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#ifndef SRC_TIMER_UPDATE_EVENT_H_
#define SRC_TIMER_UPDATE_EVENT_H_
#include "timing_mode_init.h"
#include "stdbool.h"


void timer_update_event_init(timing_mode_freq_t freq ,bool is_i2c_asked);
void button_dma_init_timing_mode(uint16_t count);
volatile bool get_done();
volatile void reset_done();
void enable_dma_2_stream5(void);
void reset_pull_states();
void disable_dma_2_stream5(void);
void enable_button_timer(void);
void disable_button_timer(void);
#endif /* SRC_TIMER_UPDATE_EVENT_H_ */

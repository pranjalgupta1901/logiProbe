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
 * @file    timing_mode_init.c
 * @brief   This file configures the MCU for timing mode by initialising peripheral like DMA, timers
 * 			and button.
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#include "timing_mode_init.h"
#include "stdbool.h"
#include "stdint.h"
#include "timer_update_event.h"
#include "state_mode.h"
#include "button_init.h"
#include "input_capture_dma.h"
#include "stm32f429xx.h"
char* freq_table[] ={"100","200","400","800","1000"};//order of this arr must match timing enum
int freq_table_len = sizeof(freq_table)/sizeof(freq_table[0]);


bool timing_mode_init(uint8_t mode, timing_mode_freq_t freq, bool is_i2c_asked, uint16_t count){
	if(mode != BUTTON_MODE)
		return false;

	disable_all_timers();
	disable_dma2_stream_2();
	disable_dma2_stream_3();
	disable_dma_2_stream5();
	disable_button_timer();
	button_init(TIMING_MODE);
	button_dma_init_timing_mode(count);
	timer_update_event_init(freq, is_i2c_asked);
	enable_button_timer();

	while(get_done() == false);
	reset_done();

	return true;

}




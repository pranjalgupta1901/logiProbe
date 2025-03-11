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
 * @file    button_init.c
 * @brief   This file configures the MCU for state mode by initialising peripheral like DMA, timers
 * 			and button and also uses logics of trigger detection
 *
 * @author  Pranjal Gupta and Krish Shah
 * @date    12/17/2023
 *
 */
#include "state_mode.h"
#include "timer_update_event.h"
#include "button_init.h"
#include "input_capture_dma.h"
#include "stdlib.h"
#include "stdbool.h"
#include "systick.h"
#include "timer.h"
volatile uint8_t *addr = NULL;
uint8_t pattern = 0x3F;
uint8_t bit = 0;
volatile uint8_t *addr_test = NULL;
uint8_t p_accumulator = 0;
bool trigger_found = false;
#define BUF_SIZE 32768

/*
 * Function to get the value of a specified bit from a byte
 *
 * Parameters:
 *  byte the byte from which bit is extracted
 *  bit the position in the byte where the bit is location
 *
 * Returns:
 *  1 if bit is set
 *  0 if bit is clear
 */
static inline uint8_t get_bit(uint8_t byte, uint8_t bit) {
	if (byte & 1 << bit) {
		return 1;
	} else {
		return 0;
	}
}

/*
 * Description: configures the state timing mode according to the user configures
 * Parameters:
 * 		input_capture_edge_t edge tells on which edge the data has to be capture
 * 		uint8_t mode tells which mode it is trigger or button
 * 		uint8_t pattern a trigger pattern asked by the user
 * 		uint8_t count size of data to be captured in terms of 32kb
 * 		uint8_t pin_num pin at which trigger detection will happen
 * 		uint32_t time_count timeout value in ms in which the trigger should be found
 * Returns:
 *   		bool true if trigger is detected
 *   			 false if timeout happened or any wrong arguments given by the user
 */

bool state_timing_init(input_capture_edge_t edge, uint8_t mode, uint8_t pattern,
		uint16_t count, uint8_t pin_num, uint32_t time_count) {

	if (mode != TRIG_MODE && mode != BUTTON_MODE)
		return false;

	disable_all_timers();
	disable_dma2_stream_2();
	disable_dma2_stream_3();
	disable_dma_2_stream5();
	reset_count_sdram_interrupts(mode);
	tim_gpio_init_state_mode();
	tim_init_input_capture(edge);
	if (mode == BUTTON_MODE) {
		button_init(STATE_MODE);
		dma_init_sdram(mode, count);
		enable_tim1();
	} else if (mode == TRIG_MODE) {
		dma_init_sdram(mode, count);
		dma_init_sram();
		enable_dma2_stream_2();
		enable_dma2_stream_3();
		init_timers_sync();
	}

	trigger_found = false;
	if (mode == TRIG_MODE) {
		ticktime_t current_tick = now();
		uint32_t i = 0;
		while (current_tick + time_count > now()) {
			if (get_process_flag() == 1) {
				addr = get_start_address();
				while (i < BUF_SIZE) {
					bit = get_bit(*addr, pin_num);
					p_accumulator = p_accumulator << 1 | (bit);//accumulate bits in byte
					if (p_accumulator == pattern) {//if match occurs, set flag.
						set_trigger_flag();
						trigger_found = true;
						addr_test = addr;
						i = 0;
						goto outside;
						break;
					}
					addr++;
					i++;
				}
			}
		}
		if (trigger_found == true)
			return false;
	} else
		goto outside;

	outside: while (get_done_flag() == false);
	reset_done_flag();

	return true;

}


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
 * @file    input_capture_dma.h
 * @brief   This file contains the function prototypes for initialisation of timers input capture, dma
 * 			contains getter and setter functions related to flag and enable and disable of timers and dma
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#ifndef SRC_INPUT_CAPTURE_DMA_H_
#define SRC_INPUT_CAPTURE_DMA_H_

#include "state_mode.h"
#include "stdint.h"
#include "stdbool.h"

void tim_init_input_capture(input_capture_edge_t edge);
void tim_gpio_init_state_mode();
void dma_init_sdram(uint8_t mode, uint16_t count);
void dma_init_sram();
bool get_trigger_status(void);
bool get_process_flag(void);
volatile uint8_t *get_start_address(void);
void set_trigger_flag(void);
void reset_process_flag(void);
void tim_init_sync(void);
volatile bool get_done_flag();
volatile void reset_done_flag();
void reset_count_sdram_interrupts(uint8_t mode);
void disable_all_timers();
void enable_tim1();
void enable_tim8();
void enable_dma2_stream_3();
void enable_dma2_stream_2();
void disable_dma2_stream_2();
void disable_dma2_stream_3();
#endif /* SRC_INPUT_CAPTURE_DMA_H_ */

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
 * @file    systick.h
 * @brief   Header file for Driver code for Systick. A tick is generated very 1ms
 *
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#ifndef __SYSTICK_H__
#define __SYSTICK_H__
#include "stm32f429xx.h"

typedef volatile uint32_t ticktime_t;

/*
 * Initializes the Systick timer. It is configured to generate an interrupt every 1ms which is used to
 * increment the tick variable.
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void init_systick();

/*
 * A function to get the current number of ticks elapsed in the system since startup
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  ticktime_t returns the current number of ticks elapsed
 */
ticktime_t now();

/*
 * A function to reset clock ticks
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void reset_clock();

/*
 * A function to get current clock ticks since reset
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  ticktime_t number of ticks since last reset
 */
ticktime_t get_clock();

/*
 * Function to create a blocking delay call for a given number of
 * ms.
 *
 * Parameters:
 *  ms number of ms to delay for
 *
 * Returns:
 *  none
 */
void b_delay(int ms);

#endif

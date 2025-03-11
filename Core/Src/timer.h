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
 * @file    timer.h
 * @brief   prototype for the function defined in timer.c which is related to synchronized
 * 			start for TIM1 and TIM8 using trigger mode on TIM5
 * 			overflow.
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

void init_timers_sync();

#endif

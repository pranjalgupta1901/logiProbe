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
 * @file    pll_clock.h
 * @brief   This file contains the function prototype for PLL initialisation and also the MACROs which represent
 * 			the buses frequency after initialisation
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */

#ifndef SRC_PLL_CLOCK_H_
#define SRC_PLL_CLOCK_H_

void init_clocks();

#define AHB1_CLK_FREQ 160000000
#define APB1_CLK_FREQ 40000000
#define APB2_CLK_FREQ 80000000

#endif /* SRC_PLL_CLOCK_H_ */

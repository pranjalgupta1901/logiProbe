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
 * @file    pll_clock.c
 * @brief   This file contain a function which configures the system and bus clocks using PLL clock.
 * 			It uses the formula given in the reference manual which is (input clock freq *PLLN )/(PLLP * PLLM)
 * 			and to get the bus freq
 * 			* APB1  = System Clock/ PPRE1 precalar
 * 			* APB2  = System Clock/ PPRE2 precalar
 * 			so system clock freq = 160 Mhz
 * 			APB1 = 40 MHZ
 * 			APB2 = 80 MHZ
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */



#include "stm32f429xx.h"




/*
 * Description: Configures the PLL clock to run at 160MHz
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void init_clocks(){


			RCC->CR	&= ~(RCC_CR_PLLON);
			RCC->CFGR |= (0b11<< RCC_CFGR_MCO1_Pos);
			RCC->CFGR |= (7<<24);
			RCC->APB1ENR |= RCC_APB1ENR_PWREN_Msk;
			PWR->CR |= PWR_CR_VOS_0 | PWR_CR_VOS_1;        // scale 1 mode(voltage scaling output)
			FLASH->ACR = FLASH_ACR_LATENCY_5WS | FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN;
			RCC->PLLCFGR = 0;
			RCC->PLLCFGR |= (RCC_PLLCFGR_PLLQ_3)|(0b001000<<RCC_PLLCFGR_PLLM_Pos)|(0b10100000<<RCC_PLLCFGR_PLLN_Pos)|(0b00<<RCC_PLLCFGR_PLLP_Pos);
			RCC->CR |= RCC_CR_HSION;
			while(!( RCC->CR & RCC_CR_HSIRDY));
			RCC->CFGR |= (0b101<<RCC_CFGR_PPRE1_Pos)|(0b100<<RCC_CFGR_PPRE2_Pos)|(0b0000<<RCC_CFGR_HPRE_Pos);
			RCC->CR |= RCC_CR_PLLON;
			while(!( RCC->CR & RCC_CR_PLLRDY));
			RCC->CFGR |= (0b10<<RCC_CFGR_SW_Pos);
			while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
}



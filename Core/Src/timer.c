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
 * @file    timer.c
 * @brief   Synchronized start for TIM1 and TIM8 using trigger mode on TIM5
 * 			overflow.
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#include "timer.h"
#include "stm32f429xx.h"

#define ITR0_MASK 0b000
#define ITR3_MASK 0b011
#define TRIGGER_MODE_MASK 0b110
#define UEV_TRGO_MASK 0b010

/*
 * Function to initialse the TIM1 and TIM8 in sync using trigger mode
 * , on TIM5 overflow.
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void init_timers_sync(){
	DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM1_STOP_Msk
			| DBGMCU_APB2_FZ_DBG_TIM8_STOP_Msk;
	DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM5_STOP_Msk;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN_Msk | RCC_APB2ENR_TIM8EN_Msk;
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN_Msk;

	TIM5->CR2 &= ~TIM_CR2_MMS_Msk;
	TIM5->CR2 |= UEV_TRGO_MASK << TIM_CR2_MMS_Pos;//uev is trgo

	TIM5->ARR = 5;//generate uev after 5 tick

	TIM1->SMCR &= ~TIM_SMCR_TS_Msk;
	TIM1->SMCR |= ITR0_MASK << TIM_SMCR_TS_Pos;   //ITR0 as trigger
	TIM1->SMCR &= ~TIM_SMCR_SMS_Msk;
	TIM1->SMCR |= TRIGGER_MODE_MASK << TIM_SMCR_SMS_Pos;   //trigger mode to start counter

	TIM8->SMCR &= ~TIM_SMCR_TS_Msk;
	TIM8->SMCR |= ITR3_MASK << TIM_SMCR_TS_Pos;   //ITR3 as trigger
	TIM8->SMCR &= ~TIM_SMCR_SMS_Msk;
	TIM8->SMCR |= TRIGGER_MODE_MASK << TIM_SMCR_SMS_Pos;   //trigger mode to start counter

	TIM5->CR1 |= TIM_CR1_CEN_Msk;//tim8 and tim1 will start when tim5 uev occurs
}


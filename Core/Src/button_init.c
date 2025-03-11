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
 * @brief   This file intialises the button and its interrupt on rising edge. the IRQ handler enables
 * 				the DMA stream according to the mode which can be state or timing mode.
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#include "button_init.h"
#include "stm32f429xx.h"
#include "stdbool.h"
#include "input_capture_dma.h"
#include "timing_mode_init.h"
#include "timer_update_event.h"
static volatile uint8_t _mode;

static void enable_button_interrupt(void);
/*
 * Description: initialises the button for the trigger of DMAs wrt mode i.e. state or timing.
 * Parameters:
 * 		uint8_t mode tells which mode it is state or timing
 *
 * Returns:
 *   	None
 */
void button_init(uint8_t mode){
	RCC->AHB1ENR &= ~(RCC_AHB1ENR_GPIOAEN);
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR &= ~RCC_APB2ENR_SYSCFGEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE0;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;
	EXTI->RTSR &= ~EXTI_RTSR_TR0;
	EXTI->RTSR |= EXTI_RTSR_TR0;
	EXTI->FTSR &= ~EXTI_FTSR_TR0;
	_mode = mode;
	enable_button_interrupt();
}

/*
 * Description: IRQ handler for the button interrupt which enable the dma stream acc to mode
 * Parameters:
 * 		None
 *
 * Returns:
 *   	None
 */
void EXTI0_IRQHandler(void){
	NVIC_ClearPendingIRQ(EXTI0_IRQn);
	NVIC_DisableIRQ(EXTI0_IRQn);
	EXTI->PR |= EXTI_PR_PR0;
	if(_mode == TIMING_MODE)
		enable_dma_2_stream5();
	else if(_mode == STATE_MODE)
		enable_dma2_stream_2();
	EXTI->IMR &= ~EXTI_IMR_IM0;   // disabling interrupts after one press of button

}


/*
 * Description: enable the button interrupt
 * Parameters:
 * 		None
 *
 * Returns:
 *   	None
 */
static void enable_button_interrupt(void){
	EXTI->IMR &= ~EXTI_IMR_IM0;

	EXTI->IMR |= EXTI_IMR_IM0;
	NVIC_EnableIRQ(EXTI0_IRQn);
}

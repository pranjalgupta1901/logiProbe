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
 * @file	timer_update_event.c
 * @brief   This file intialises the timer for its update event, initialises button DMA, sets the sampling freq
 * 			and cotains flag which tells about the capture is complete or not.
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#include "main.h"
#include "stdint.h"
#include "stdbool.h"
#include "timer_update_event.h"
#include "timing_mode_init.h"
#include "stm32f429xx.h"

#define TIMING_MODE_SDRAM_ADDR ((uint8_t*)0xD0000000)
#define SDRAM_SIZE_TEST 0x800000

static uint16_t _count = 0;
uint16_t count = 0;
static void set_arr(timing_mode_freq_t freq);
volatile bool done;


/*
 * Description: initialises the timer for the timer update event
 * Parameters:
 * 		timing_mode_freq_t it tells what should be the frequency
 * 		bool is_i2c_asked which tells that does the user wants to sample i2c data or not
 * Returns:
 *   		None
 */
void timer_update_event_init(timing_mode_freq_t freq ,bool is_i2c_asked){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN_Msk;

	if(is_i2c_asked == true)
		GPIOC->PUPDR = 0xAAA0 << 16;              // making all the pins pull down
	else
		GPIOC->PUPDR = 0xAAAA << 16;
	set_arr(freq);      // Set to the required period - 1
	TIM1->DIER |= TIM_DIER_UDE_Msk;  // enable dma on timer update


}


/*
 * Description: It disables the button timer and also disable the dma request from the timer
 * Parameters:
 * 		None
 *
 * Returns:
 *   		None
 */
void disable_button_timer(void){
	TIM1->DIER &= ~TIM_DIER_UDE_Msk;
	TIM1->CNT = 0;
	TIM1->CR1 &= ~TIM_CR1_CEN;   // start the timer
}


/*
 * Description: It enables the button timer
 * Parameters:
 * 		None
 *
 * Returns:
 *   		None
 */
void enable_button_timer(void){
	TIM1->CR1 |= TIM_CR1_CEN;   // start the timer
}


/*
 * Description: It enables the dma for the button mode required for the timings mode
 * Parameters:
 * 		uint16_t count It specifies the size of the data user wants to sample
 *
 * Returns:
 *   		None
 */

void button_dma_init_timing_mode(uint16_t count){

	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN_Msk;
	disable_dma_2_stream5();
	_count = count;
	DMA2->LIFCR = 0;//clear lisr hisr config
	DMA2->HIFCR = 0;

	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	DMA2_Stream5->PAR = (uint32_t)0x40020811;
	DMA2_Stream5->M0AR = (uint32_t)TIMING_MODE_SDRAM_ADDR;
	DMA2_Stream5->NDTR = 32768;
	DMA2_Stream5->CR |= DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 /*| DMA_SxCR_HTIE_Msk*/;
	DMA2_Stream5->CR |= DMA_SxCR_PL_1 | DMA_SxCR_PL_0;
	//PSIZE , MSIZE to be 8bit by default
	DMA2_Stream5->CR |= DMA_SxCR_MINC;
	DMA2_Stream5->CR |= DMA_SxCR_TCIE_Msk;

}


/*
 * Description: It sets the ARR value corresponfing what we frquency of sampling the user wants
 * Parameters:
 * 		timing_mode_freq_t freq which the user wants
 *
 * Returns:
 *   		None
 */
static void set_arr(timing_mode_freq_t freq){
	switch(freq){
	case FREQ_100KHz:
			TIM1->ARR = 1599;
			break;
	case FREQ_200KHz:
			TIM1->ARR = 799;
			break;

	case FREQ_400KHz:
			TIM1->ARR = 399;
			break;

	case FREQ_800KHz:
			TIM1->ARR = 199;
			break;

	case FREQ_1000KHz:
			TIM1->ARR = 159;
			break;

	default:
		break;
	}
}


/*
 * Description: IRQ handler for the dma2 stream 5 which occurs till the commplete asked data is not captured
 * Parameters:
 * 		None
 *
 * Returns:
 *   		None
 */
void DMA2_Stream5_IRQHandler(){
	DMA2->HIFCR |= DMA_HIFCR_CTCIF5;
	DMA2->HIFCR |= DMA_HIFCR_CHTIF5;
	NVIC_ClearPendingIRQ(DMA2_Stream5_IRQn);

	if(count == _count){
		TIM1->DIER &= ~(TIM_DIER_UDE_Msk);
		disable_dma_2_stream5();
		reset_pull_states();
		done = true;
		count = 0;
		_count = 0;
	}
	else{
		count++;

	disable_dma_2_stream5();
	DMA2_Stream5->M0AR += 32768;
	DMA2_Stream5->NDTR = 32768;
	enable_dma_2_stream5();
	}
}


/*
 * Description: It returns the done flag status
 * Parameters:
 * 		None
 *
 * Returns:
 *   		returns the status of the done flag
 */
volatile bool get_done(){

	return done;
}

/*
 * Description: resets the status of done flag to false
 * Parameters:
 * 		None
 *
 * Returns:
 *   		void
 */
volatile void reset_done(){
	done = false;
}



/*
 * Description: Enables the dma stream for the timings mode
 * Parameters:
 * 		None
 *
 * Returns:
 *   	None
 */
void enable_dma_2_stream5(void){
	DMA2_Stream5->CR |= DMA_SxCR_EN;
	while(!(DMA2_Stream5->CR & DMA_SxCR_EN_Msk));
}

/*
 * Description: Disables the dma stream for the timings mode
 * Parameters:
 * 		None
 *
 * Returns:
 *   	None
 */
void disable_dma_2_stream5(void){
	DMA2_Stream5->CR &= ~DMA_SxCR_EN;
	while(DMA2_Stream5->CR & DMA_SxCR_EN_Msk);
}

/*
 * Description: resets the pull states to none of the GPIO port C pins
 * Parameters:
 * 		None
 *
 * Returns:
 *   	None
 */
void reset_pull_states(){
GPIOC->PUPDR &= ~(0xFFFF << 16);
}

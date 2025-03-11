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
 * @file    input_capture_dma.c
 * @brief   This file configure the dma for trigger and button mode in the state mode. It also takes care of the
 * 			interrupts of the dma of all the streams used in the state mode. This also configures the input capture pins of tim1 and tim8
 * 			to trigger the dma at every event.
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#include "stm32f429xx.h"
#include "input_capture_dma.h"
#include "stdlib.h"
#include "systick.h"
#include "state_mode.h"
#include "string.h"

static uint16_t _count = 0;
static uint8_t _mode;
#define SIZE_32KB 32768
uint8_t array_1[SIZE_32KB] = { 0 };
uint8_t array_2[SIZE_32KB] = { 0 };

#define SDRAM_BANK_ADDR_TEST ((uint8_t*)0xD0000000)
#define SDRAM_BANK_ADDR_TEST_DMA ((uint8_t*)0xD0008000)
#define SDRAM_SIZE_TEST 0x800000
#define GPIOC_UPPER_8_BITS_ADDR 0x40020811
#define BUTTON_MODE_COUNT_SDRAM 0
#define TRIGGER_MODE_COUNT_SDRAM 2

volatile uint32_t start_time_dma, end_time_dma;
volatile uint8_t *process_start_addr = NULL;
volatile uint16_t count_sdram_interrupts, count_sram_interrupts = 0;
bool trigger_flag = false, process_flag = false;
volatile bool done_flag = 0;


static void configure_input_capture_edge(TIM_TypeDef *pTIM,
		input_capture_edge_t edge);




/*
 * Description: initialises the input capture for both TIM1 and TIM8
 * Parameters:
 * 		input_capture_edge_t it tells which edge to configure for the capture
 * Returns:
 *   		None
 */
void tim_init_input_capture(input_capture_edge_t edge) {

	RCC->APB2ENR |= (RCC_APB2ENR_TIM1EN_Msk | RCC_APB2ENR_TIM8EN_Msk);

	TIM1->ARR = 0xffffffff;
	TIM1->CCMR1 &= ~TIM_CCMR1_CC2S;
	TIM1->CCMR1 |= (1 << 8);    // direct input  IC2 IS MAPPED ON TI2
	TIM1->DIER |= (1 << 10);         // CC2DE ENABLE DMA REQUEST
	configure_input_capture_edge(TIM1, edge);

	TIM1->CCER = TIM_CCER_CC2E;            // enabling channel

	TIM8->ARR = 0xffffffff;
	TIM8->CCMR1 &= ~TIM_CCMR1_CC2S;
	TIM8->CCMR1 |= (1 << 8);    // direct input  IC2 IS MAPPED ON TI2
	TIM8->DIER |= (1 << 10);         // CC2DE ENABLE DMA REQUEST
	configure_input_capture_edge(TIM8, edge);

	TIM8->CCER = TIM_CCER_CC2E;            // enabling channel

}


/*
 * Description: initialises the input capture pins both TIM1 and TIM8
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void tim_gpio_init_state_mode() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOAEN;

	GPIOA->MODER |= GPIO_MODER_MODE9_1; // pa9 af1
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR9_1;
	GPIOA->OSPEEDR |= 3 << 18;
	GPIOA->AFR[1] |= (1 << 4); // making Pa9 as Aternate function AF01

	GPIOC->MODER |= GPIO_MODER_MODE7_1; // pc7
	GPIOC->PUPDR |= GPIO_PUPDR_PUPD7_1;
	GPIOC->OSPEEDR |= (3 << 14);
	GPIOC->AFR[0] |= 3 << 28; // making Pc7 as Aternate function AF03

	GPIOC->PUPDR |= 0xAAAA << 16;

}

/*
 * Description: initialises the DMA for the SDRAM
 * Parameters:
 * 		uint8_t mode which tells it is button or trigger mode
 * 		uint16_t count which tells how many bytes of data to be captured in terms of 32KB
 * Returns:
 *   		None
 */

void dma_init_sdram(uint8_t mode, uint16_t count) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN_Msk;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;

	_count = count;
	disable_dma2_stream_2();
	while (DMA2_Stream2->CR & DMA_SxCR_EN_Msk)
		;   //wait till it is zero
	DMA2->LISR = 0;   //clear lisr hisr config
	DMA2->HISR = 0;
	_mode = mode;
	DMA2_Stream2->PAR = (uint32_t) GPIOC_UPPER_8_BITS_ADDR;
	if (mode == TRIG_MODE)
		DMA2_Stream2->M0AR = (uint32_t) SDRAM_BANK_ADDR_TEST_DMA;
	else
		DMA2_Stream2->M0AR = (uint32_t) SDRAM_BANK_ADDR_TEST;
	DMA2_Stream2->NDTR = SIZE_32KB;
	DMA2_Stream2->CR |= (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2);

	DMA2_Stream3->CR &= ~DMA_SxCR_MSIZE;
	DMA2_Stream3->CR &= ~DMA_SxCR_PSIZE;
	DMA2_Stream2->CR |= DMA_SxCR_MINC;

	DMA2_Stream2->CR |= DMA_SxCR_TCIE_Msk;    // TCIE bit enable
	NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}


/*
 * Description: initialises the input capture for SRAM
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void dma_init_sram(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN_Msk;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN_Msk;
	disable_dma2_stream_3();
	while (DMA2_Stream3->CR & DMA_SxCR_EN_Msk)
		;    //wait till it is zero
	DMA2->LISR = 0;    //clear lisr hisr config
	DMA2->HISR = 0;

	DMA2_Stream3->PAR = (uint32_t) 0x40020811;       // gpioc->idr upper 8 lines
	DMA2_Stream3->M0AR = (uint32_t) array_1;
	DMA2_Stream3->M1AR = (uint32_t) array_2;
	DMA2_Stream3->NDTR = SIZE_32KB;
	DMA2_Stream3->CR |=
			(DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0); // channel 7
	DMA2_Stream3->CR |= DMA_SxCR_PL_1 | DMA_SxCR_PL_0;
	DMA2_Stream3->CR |= DMA_SxCR_DBM_Msk;

	DMA2_Stream3->CR &= ~DMA_SxCR_MSIZE;
	DMA2_Stream3->CR &= ~DMA_SxCR_PSIZE;
	DMA2_Stream3->CR |= DMA_SxCR_MINC;

	DMA2_Stream3->CR |= DMA_SxCR_TCIE_Msk;    // TCIE bit enable
	NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}


/*
 * Description: irq handler for SRAM
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void DMA2_Stream3_IRQHandler() {
	DMA2->LIFCR |= DMA_LIFCR_CTCIF3_Msk;  // clearing the interrupt flags
	DMA2->LIFCR |= DMA_LIFCR_CHTIF3;
	NVIC_ClearPendingIRQ(DMA2_Stream3_IRQn); // clearing the PR bit in PR register

	if (trigger_flag == true) {
		TIM8->DIER &= ~(TIM_DIER_CC2DE_Msk);
		disable_dma2_stream_3();
		enable_dma2_stream_2();
		process_flag = false;
	} else {
		if (DMA2_Stream3->CR & DMA_SxCR_CT_Msk)
			process_start_addr = array_1;
		else
			process_start_addr = array_2;
		process_flag = true;
	}

}


/*
 * Description: tells if the data capture is complete or not
 * Parameters:
 * 		None
 * Returns:
 *   		bool flag status
 */

volatile bool get_done_flag() {
	if (done_flag == true && _mode == TRIG_MODE) {
		if (DMA2_Stream3->CR & DMA_SxCR_CT_Msk) {
			memcpy(SDRAM_BANK_ADDR_TEST, array_1, SIZE_32KB);
			memcpy(SDRAM_BANK_ADDR_TEST + SIZE_32KB, array_2, SIZE_32KB);
		} else {
			memcpy(SDRAM_BANK_ADDR_TEST, array_2, SIZE_32KB);
			memcpy(SDRAM_BANK_ADDR_TEST + SIZE_32KB, array_1, SIZE_32KB);
		}

	}

	return done_flag;
}


/*
 * Description: resets the flag
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
volatile void reset_done_flag() {
	done_flag = false;
}


/*
 * Description: irq handler for SDRAM
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void DMA2_Stream2_IRQHandler() {
	DMA2->LIFCR |= DMA_LIFCR_CTCIF2;     // clearing the interrupt flags
	DMA2->LIFCR |= DMA_LIFCR_CHTIF2;     // clearing the interrupt flags
	NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn); // clearing the PR bit in PR register

	if (count_sdram_interrupts == _count) {
		disable_dma2_stream_2();                   // SDRAM IS FULL
		done_flag = true;
	} else {
		DMA2_Stream2->CR &= ~DMA_SxCR_EN;
		DMA2_Stream2->M0AR += SIZE_32KB;
		DMA2_Stream2->NDTR = SIZE_32KB;
		enable_dma2_stream_2();
		count_sdram_interrupts++;
	}
}


/*
 * Description: reset the starting count value according to the mode
 * Parameters:
 * 		uint8_t mode tells which mode it is
 * Returns:
 *   		None
 */
void reset_count_sdram_interrupts(uint8_t mode) {
	if (mode == BUTTON_MODE)
		count_sdram_interrupts = BUTTON_MODE_COUNT_SDRAM;
	else
		count_sdram_interrupts = TRIGGER_MODE_COUNT_SDRAM;
}


/*
 * Description: tells the status if trigger is found is found or not
 * Parameters:
 * 		None
 * Returns:
 *   		bool returns the status
 */
bool get_trigger_status(void) {
	return trigger_flag;
}


/*
 * Description: tells the status of process flag
 * Parameters:
 * 		None
 * Returns:
 *   		bool returns the status
 */
bool get_process_flag(void) {
	return process_flag;
}


/*
 * Description: returns the start address of the buffers used in dual buffer mode
 * Parameters:
 * 		None
 * Returns:
 *   		bool returns the address
 */
volatile uint8_t* get_start_address() {
	return process_start_addr;
}


/*
 * Description: sets the status of trigger flag
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void set_trigger_flag(void) {
	trigger_flag = true;

}


/*
 * Description: resets the status of process flag
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void reset_process_flag(void) {
	process_flag = false;
}

static void configure_input_capture_edge(TIM_TypeDef *pTIM,
		input_capture_edge_t edge) {

	switch (edge) {

	case RISING_EDGE:
		pTIM->CCER &= ~ TIM_CCER_CC2P;
		pTIM->CCER &= ~ TIM_CCER_CC2NP;
		break;

	case FALLING_EDGE:
		pTIM->CCER |= TIM_CCER_CC2P;
		pTIM->CCER &= ~ TIM_CCER_CC2NP;
		break;

	case RISING_FALLING_EDGE:
		pTIM->CCER |= TIM_CCER_CC2P;
		pTIM->CCER |= TIM_CCER_CC2NP;
		break;

	default:
		break;

	}

}



/*
 * Description: Starts the counter of tim1
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void enable_tim1() {
	TIM1->CR1 |= TIM_CR1_CEN_Msk;
}


/*
 * Description: Starts the counter of tim8
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void enable_tim8() {
	TIM8->CR1 |= TIM_CR1_CEN_Msk;
}

/*
 * Description: enables the dma2 stream 2
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void enable_dma2_stream_2() {
	DMA2_Stream2->CR |= DMA_SxCR_EN;
}



/*
 * Description: enables the dma2 stream 3
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void enable_dma2_stream_3() {
	DMA2_Stream3->CR |= DMA_SxCR_EN;
}


/*
 * Description: disable the dma2 stream 2
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void disable_dma2_stream_2() {
	DMA2_Stream2->CR &= ~DMA_SxCR_EN;
	while (DMA2_Stream2->CR & DMA_SxCR_EN_Msk)
		;
}


/*
 * Description: disables the dma2 stream 3
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void disable_dma2_stream_3() {
	DMA2_Stream3->CR &= ~DMA_SxCR_EN;
	while (DMA2_Stream3->CR & DMA_SxCR_EN_Msk)
		;
}


/*
 * Description: disables all the timer running
 * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void disable_all_timers() {
	TIM5->CR1 &= ~TIM_CR1_CEN_Msk;
	TIM1->CR1 &= ~TIM_CR1_CEN_Msk;
	TIM8->CR1 &= ~TIM_CR1_CEN_Msk;
}


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
 * @file   spi.c
 * @brief   This file contains the GPIO initailisation, SPI initialisation, write and read functionality and
 * 			also functions related to chip select GPIO pin
 *
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */
#include "stm32f429xx.h"
#include "spi.h"

/*
 * Description: initialises the spi to transfer and receive data from SD Card using FatFS
 *  * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void spi_init(void){

	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	SPI1->CR1 = 0;
	SPI1->CR1 |= SPI_CR1_MSTR;
	SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
	SPI1->CR1 |= SPI_CR1_BR_1 | SPI_CR1_BR_2;   // PRESCALCING BY 128
	SPI1->CR1 &= ~(SPI_CR1_CPHA | SPI_CR1_CPOL);
	SPI1->CRCPR = 10;



	// CONFIGURE THE BAUDRATE
	SPI1->CR1 |= SPI_CR1_SPE;                // ENABLING THE SPI

}

void spi_gpio_pin_init(void){

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

	GPIOA->MODER |= GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;
	GPIOA->AFR[0] |= GPIO_AFRL_AFRL5_0 | GPIO_AFRL_AFRL5_2 | GPIO_AFRL_AFRL6_0 | GPIO_AFRL_AFRL6_1 | GPIO_AFRL_AFRL7_0 | GPIO_AFRL_AFRL7_2;

	GPIOB->MODER |= GPIO_MODER_MODE12_0;
}

/*
 * Description: transfer a complete buffer through SPI
 * Parameters:
 * 		uint8_t *buffer an array which contains the data to be transferred
 * 		uint8_t len which tells the length of the data to be transferred
 * Returns:
 *   		None
 */

void spi_transmit_buffer(uint8_t *buffer, uint8_t len){

	while(len > 0){
		spi_transmit_data(*buffer);
		len--;
		buffer++;
	}

}


/*
 * Description: transfer a single byte through SPI
 * Parameters:
 * 		uint8_t data the data which is to be transferred
 * Returns:
 *   		None
 */

void spi_transmit_data(uint8_t data){
	while(!(SPI1->SR & SPI_SR_TXE));
	SPI1->DR = data;
	while(SPI1->SR & SPI_SR_BSY);
}



/*
 * Description: receives a data from the spi
 * Parameters:
 *
 * Returns:
 *   	uint8_t returns the received data
 */

uint8_t spi_read_data(){
	while(!(SPI1->SR & SPI_SR_RXNE));
	return SPI1->DR;
}


/*
 * Description: transfer a complete buffer through SPI
 * Parameters:
 * 		uint8_t *buffer an array which contains the data to be transferred
 * 		uint8_t len which tells the length of the data to be transferred
 * Returns:
 *   		None
 */

void spi_write_read_data(uint8_t *write_data, uint8_t *read_data, uint8_t len){

	for(uint8_t i = 0; i<len; i++){
	spi_transmit_data(write_data[i]);
	read_data[i] = spi_read_data();
	}

}


/*
 * Description: sets the chip select high
 *  * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void gpio_set_cs_high(){

	GPIOB->ODR |= GPIO_ODR_OD12;


}

/*
 * Description: sets the chip select pin low
 *  * Parameters:
 * 		None
 * Returns:
 *   		None
 */
void gpio_set_cs_low(){

	GPIOB->ODR &= ~GPIO_ODR_OD12;


}

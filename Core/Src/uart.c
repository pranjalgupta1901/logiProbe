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
 * @file    uart.c
 * @brief   UART Drive Coder. Provides functions to initialize the UART,
 * 			and remap printf to uart
 *
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#include "uart.h"
#include "stm32f429xx.h"
#include <stdio.h>

#define UART_BRR_MANTISSA_115200_BAUD 21
#define UART_BRR_FRACTION_115200_BAUD 11


/*
 * Initializes USART to 1152000 Baud. It uses USART2 on PD5(TX) and PD6(RX).
 *
 * Baud = Fck/((8 x (2 - OVER8) x USARTDIV)
 *
 * Fck = 40MHz
 * Over8 = 0
 * USARTDIV = (40MHz)/(8 x 2 x 115200) = 21.7
 *
 * Therefore, Mantissa = 21 and Fraction = 11 in BRR
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void init_uart(){

	setbuf(stdout, NULL);

	//provide clock to gpio and usart2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN_Msk;


	//select alternate function on gpio
	GPIOD->MODER &= ~(GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE6_Msk);
	GPIOD->MODER |= GPIO_MODER_MODE5_1|GPIO_MODER_MODE6_1;//10 for alt func

	GPIOD->AFR[0] &= ~(GPIO_AFRL_AFSEL5_Msk | GPIO_AFRL_AFSEL6_Msk);
	GPIOD->AFR[0] |= 0b111<<GPIO_AFRL_AFSEL5_Pos;
	GPIOD->AFR[0] |= 0b111<<GPIO_AFRL_AFSEL6_Pos;

	//set baudrate to 115200
	USART2->BRR &= ~(USART_BRR_DIV_Mantissa_Msk | USART_BRR_DIV_Fraction_Msk);
	USART2->BRR |= ((UART_BRR_MANTISSA_115200_BAUD << USART_BRR_DIV_Mantissa_Pos)
				   |(UART_BRR_FRACTION_115200_BAUD << USART_BRR_DIV_Fraction_Pos));
	//enable usart
	USART2->CR1 |= USART_CR1_UE_Msk;//keep m=0 for 8 bit communication
	//keep STOP in cr2 to 00 for 1 stop bit


	//set te and re bit
	USART2->CR1 |= USART_CR1_TE_Msk | USART_CR1_RE_Msk ;
}

/* Serial Input function. Receives 1 character at a time
 *
 * Parameters:
 * 	none
 *
 * Returns:
 *  unsigned char value of the character input
 */
unsigned char get_char(){
	char result = 0;
	while(!(USART2->SR & USART_SR_RXNE));
	result = USART2->DR;
	return result;
}

/* Serial Output function. Transmits 1 character at a time
 *
 * Parameters:
 * 	unsigned char value of character to be transmitted
 *
 * Returns:
 *  unsigned char value of character transmitted
 */
unsigned char put_char(unsigned char c_out){
	while(!(USART2->SR & USART_SR_TXE));
	USART2->DR = c_out;
	return c_out;
}

/*
 * Underlying syscall used by printf to output data. It is defined as WEAK in syscalls.c
 * By defining it here we overwrite it redirect printf over UART.
 *
 * Parameters:
 *  file output to which _write operates, not used in our case
 *  ptr pointer to buffer
 *  len length of buffer
 *
 * Returns:
 *  number of characters written
 */
int _write(int file, char *ptr, int len)
{
  for (int i = 0; i < len; i++)
  {
    put_char(*ptr++);
  }
  return len;
}


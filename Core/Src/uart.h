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
 * @file    uart.h
 * @brief   Header file for UART Driver Code. Provides functions to initialize the UART,
 * 			and remap printf to uart
 *
 *
 * @author  Krish Shah
 * @date    October 27 2023
 *
 **/
#ifndef __UART_H__
#define __UART_H__

#define LF 0xA
#define RE 0xD

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
void init_uart();

/* Serial Input function. Receives 1 character at a time
 *
 *
 * Parameters:
 * 	none
 *
 * Returns:
 *  unsigned char value of the character input
 */
unsigned char get_char();

/* Serial Output function. Transmits 1 character at a time
 *
 * Parameters:
 * 	unsigned char value of character to be transmitted
 *
 * Returns:
 *  unsigned char value of character transmitted
 */
unsigned char put_char(unsigned char c_out);
#endif

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
 * @file    cmd_processor.h
 * @brief   Header file for Code for UART based Command processor to creating a user interface
 * 			to control the functioning of the different modules of the logic analyzer
 *
 * 			The concept of the command processor was covered in the course ECEN5813:
 * 			Principles of Embedded Software. We extended those concepts for use in our system,
 * 			by using c standard library functions to allow a user to enter parameters in any order.
 * 			For example, the following commands will result in the same functionality:
 *          > tmode -f 200 -i i2c
 *          > tmode -i i2c -f 200
 *
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#ifndef __CMD_PROCESSOR_H__
#define __CMD_PROCESSOR_H__


/* Function to run the command processor. It first print the "> " prompt onto the screen.
 * After which is accepts user input, which is delimited by a carriage return. Then the
 * line input is tokenised and processed.
 *
 * Parameters:
 *	none
 *
 * Returns:
 *  none
 */
void run_command_processor();
#endif

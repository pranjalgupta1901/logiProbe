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
 * @file    i2c_analyser.c
 * @brief   I2C interpreter code. It goes over a given buffer, where data is 8bit format
 * 			and certain pins are selected as SDA and SCL by calling code.
 *
 * 			It can work on 7bit i2c address.
 *
 * 			It can detect:
 * 			START
 * 			RESTART
 * 			STOP
 * 			Slave Address
 * 			R/W
 * 			ACK/NACK
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#include "i2c_analyser.h"
#include "stdint.h"
#include "stdio.h"


#ifdef TESTING
static uint8_t buffer[] = {0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 1, 1, 1, 1, 1, 3, 3, 3, 3, 1, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 3, 3, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 1, 3, 3, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 1, 1, 0, 0, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
uint16_t buf_len = sizeof(buffer)/sizeof(buffer[0]);
#endif

/*
 * Function to check if a bit is set in a given sample at a given position
 *
 * Parameters:
 *  sample the 8-bit sample to check
 *  position the bit position to check
 *
 * Returns:
 *  1 if bit is set
 *  0 if bit is clear
 */
uint8_t is_bit_set(uint8_t sample,uint8_t position){
	if(sample & (1<<position)){
		return 1;
	}else{
		return 0;
	}
}

/*
 * Function to check if a bit is clear in a given sample at a given position
 *
 * Parameters:
 *  sample the 8-bit sample to check
 *  position the bit position to check
 *
 * Returns:
 *  1 if bit is clear
 *  0 if bit is set
 */
uint8_t is_bit_clear(uint8_t sample,uint8_t position){
	if(sample & (1<<position)){
		return 0;
	}else{
		return 1;
	}
}

/*
 *	Function to check if a start condition occurred, based on current sample and previous sample, along
 *	with the position of sda and scl.
 *
 *	Start condition is detected when the SDA value changes from 1->0 when the SCL value is held high from
 *	1->1
 *
 * Parameters:
 *  previous_sample value of previous sample
 *  current_sample 	value of current sample
 *  scl_pos position of scl in the sample
 *  sda_pos posititon of sda in the sample
 *
 * Returns:
 *  1 if start condition detected
 *  0 if start condition not detected
 */
uint8_t is_start_condition(uint8_t previous_sample, uint8_t current_sample, uint8_t scl_pos, uint8_t sda_pos){
	if(
	   (is_bit_set(previous_sample,sda_pos)&& is_bit_set(previous_sample,scl_pos)) &&
	   (is_bit_clear(current_sample,sda_pos) && is_bit_set(current_sample,scl_pos))
	   ){
		return 1;
	}else{
		return 0;
	}
}


/*
 *	Function to check if a stop condition occurred, based on current sample and previous sample, along
 *	with the position of sda and scl.
 *
 *	Start condition is detected when the SDA value changes from 0->1 when the SCL value is held high from
 *	1->1
 *
 * Parameters:
 *  previous_sample value of previous sample
 *  current_sample 	value of current sample
 *  scl_pos position of scl in the sample
 *  sda_pos posititon of sda in the sample
 *
 * Returns:
 *  1 if stop condition detected
 *  0 if stop condition not detected
 */
uint8_t is_stop_condition(uint8_t previous_sample, uint8_t current_sample, uint8_t scl_pos, uint8_t sda_pos){
	if(
	   (is_bit_clear(previous_sample,sda_pos) && is_bit_set(previous_sample,scl_pos)) &&
	   (is_bit_set(current_sample,sda_pos) && is_bit_set(current_sample,scl_pos))
	   ){
		return 1;
	}else{
		return 0;
	}
}

/*
 *	Function to detect rising edge at a given bit position, according to the previous sample
 *	and current sample.
 *
 *	Rising edge is when value changes from 0->1
 *
 * Parameters:
 *  previous_sample value of previous sample
 *  current_sample 	value of current sample
 *  bit_position the position to check in the sample
 *
 * Returns:
 *  1 if positive edge detected
 *  0 if positive edge not detected
 */
uint8_t is_positive_edge(uint8_t previous_sample, uint8_t current_sample, uint8_t bit_position){
	if(
	   is_bit_clear(previous_sample,bit_position) &&
	   is_bit_set(current_sample,bit_position)
	   ){
		return 1;
	}else{
		return 0;
	}
}

typedef struct{
	uint16_t accumulator;
	uint8_t length;
}accumulator_type_t;

/*
 * Function to clear data present in the accumulator structure
 *
 * Parameters:
 *  handler pointer to handler structure
 *
 * Returns:
 *  none
 */
void clear_accumulator(accumulator_type_t *handler){
	handler->accumulator = 0;
	handler->length = 0;
}

/*
 * Function to accumulate bit in the accumulator structure until capacity of accumulator is
 * reached. The bits are accumulated by or'ing with 1 left shifted value present in the
 * accumulator.
 *
 * This function is used to accumulate bits from the I2C communication to collect the i2c frame
 * 9 bits at a time
 *
 * Parameters:
 *  handler pointer to handler structure
 *  capacity the capacity up till which the data is accumulated
 *  bit the bit to accumulate
 *
 * Returns:
 *  1 if capacity is reached
 *  0 if capacity is not reached
 */
uint8_t accumulate(accumulator_type_t *handler,uint8_t capacity,uint8_t bit){
	handler->accumulator = (handler->accumulator << 1) | bit;
	handler->length += 1;
	if(handler->length == capacity){
		return 1;
	}else{
		return 0;
	}
}

/*
 * Function to print the address field of a i2c transaction
 * in a clear format.
 *
 * Parameters:
 *  data the 9 bits of address field to be processed and printed
 *
 * Returns:
 *  none
 */
void print_processed_addr(uint16_t data){
	printf("ADDR: 	  %x\r\n", data>>2);
	printf("RW:   	  %x\r\n",(data&0b10)>>1);
	printf("ACK/NACK: %x\r\n", data&0b1);
}

/*
 * Function to print the data field of a i2c transaction
 * in a clear format.
 *
 * Parameters:
 *  data the 9 bits of data field to be processed and printed
 *
 * Returns:
 *  none
 */
void print_processed_data(uint16_t data){
	printf("DATA: 	  %x\r\n", data>>1);
	printf("ACK/NACK: %x\r\n", data&0b1);
}

/*
 * Function to run i2c analyzer task, on a given buffer with given scl and sda bit positions
 *
 * Parameters:
 *  buffer pointer to byte array containing samples
 *  buf_len length of byte array containing samples
 *  scl_pos position of scl signal in sample
 *  sda_pos position of sda signal in sample
 *
 * Returns:
 *  none
 */
void run_analyser(uint8_t buffer[],uint32_t buf_len,uint8_t scl_pos,uint8_t sda_pos){
	uint8_t previous_sample = 0;
	uint8_t current_sample = 0;

	uint8_t event_has_start_occured = 0, event_have_bits_accumulated = 0;
	uint16_t i2c_transaction_byte_number = 0;
	accumulator_type_t accumulator ={
										.accumulator = 0,
										.length = 0
									};

	for(int i = 1; i < buf_len;i++){
		previous_sample = buffer[i-1];
		current_sample = buffer[i];
		if(is_start_condition(previous_sample, current_sample, scl_pos, sda_pos)){
			if(event_has_start_occured == 0){
				printf("START DETECTED AT %d\r\n",i);//if start has occurred for the first time or
													 //for the first time after stop
				event_has_start_occured = 1;
			}else{
				printf("REPEATED START DETECTED AT %d\r\n",i);//if start has occurred again without a stop
															  //condition
				i2c_transaction_byte_number = 0;//clear are variables so that they dont
												//interfere with next calculation
				clear_accumulator(&accumulator);
				continue;
			}

		}
		if(is_stop_condition(previous_sample, current_sample, scl_pos, sda_pos)){
			printf("STOP DETECTED AT %d\r\n",i);//if stop is detected, clear are variables so that they dont
												//interfere with next calculation
			event_has_start_occured = 0;
			i2c_transaction_byte_number = 0;
			clear_accumulator(&accumulator);
		}

		if(event_has_start_occured){//if start has occured, sample SDA on every positive edge of the SCL line
			if(is_positive_edge(previous_sample, current_sample, scl_pos)){
				if(accumulate(&accumulator, 9, is_bit_set(current_sample, sda_pos)) == 1){
					event_have_bits_accumulated = 1;//accumulate 9 bits before processing them
													//in case of address: 7 bit address + 1 bit RW + 1 bit ACK/NACK
													//in case of data: 8 bit data + 1 bit ACK/NACK
				}
			}

			if(event_have_bits_accumulated){//if bits have been accumulated, process them and print the result
				event_have_bits_accumulated = 0;
				if(i2c_transaction_byte_number == 0){//if it is the first byte after start or restart, process it as address,
													 //else process it as data
					print_processed_addr(accumulator.accumulator);
				}else{
					print_processed_data(accumulator.accumulator);
				}
				clear_accumulator(&accumulator);
				i2c_transaction_byte_number++;
			}

		}
	}
}

/*
 *	Function to the run the analyzer on a sample array.
 *	To run the function, uncomment:
 *	#define TESTING
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
#ifdef TESTING
void test_analyser(){
	run_analyser(buffer,buf_len,1,0);
}
#endif


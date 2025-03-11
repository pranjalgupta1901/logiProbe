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
 * @file    fmc.h
 * @brief   Header file for FMC driver code to make use of on board SDRAM on STM32f429 Discovery board
 * 			to provide 8megabytes of additional memory. Once the initialization process is
 * 			complete and the SDRAM parameters are all initialized to their correct values,
 * 			the 8 megabytes of SDRAM memory are memory mapped the the processor, with the access
 * 			to those memory ranges being done similarly to other memory. The FMC peripheral will
 * 			take care of the underlying protocol and timings for the SDRAM.
 *
 * 			The the SDRAM on the discovery board is connected to the bank 2 and the
 * 			memory range of the FMC after initialisation is 0xD0000000 to 0xD08000000.
 *
 * 			Calculation for timing parameters based on:
 * 			https://community.st.com/t5/stm32-mcus/how-to-set-up-the-fmc-peripheral-to-interface-with-the-sdram/ta-p/49457
 *
 *			This link present above uses the HAL to initialize the SDRAM, whereas we have created our own
 *			baremetal implementation
 *
 * @author  Krish Shah
 * @date    December 17 2023
 *
 */
#ifndef __FMC_H__
#define __FMC_H__
#include "stm32f429xx.h"

#define FMC_CR_DNC_MASK (FMC_SDCR1_RPIPE_Msk | FMC_SDCR1_RBURST_Msk | FMC_SDCR1_SDCLK_Msk)
#define FMC_TR_DNC_MASK (FMC_SDTR1_TRP_Msk | FMC_SDTR1_TRC_Msk)

//TODO: Add calculation
#define FMC_TR_TMRD_VAL 		0b0001
#define FMC_TR_TXSR_VAL 		0b0101
#define FMC_TR_TRAS_VAL 		0b0011
#define FMC_TR_TRC_VAL  		0b0101
#define FMC_TR_TWR_VAL  		0b0001
#define FMC_TR_TRP_VAL  		0b0001
#define FMC_TR_TRCD_VAL 		0b0001

#define FMC_CR_NC_8_BITS 		0b00
#define FMC_CR_NR_12_BITS		0b01
#define FMC_CR_MWID_16_BITS		0b01
#define FMC_CR_NB_2_BANKS		0b01
#define FMC_CR_CAS_LATENCY_2	0b10
#define FMC_CR_WP_NONE			0b00
#define FMC_CR_SDCLK_2X			0b10
#define FMC_CR_RPIPE_1			0b01

#define FMC_CMR_NRFS_VAL		0b11

#define SDRAM_CMD_NORMAL_MODE	0b000
#define SDRAM_CMD_CLOCK_ENABLE	0b001
#define SDRAM_CMD_PALL			0b010
#define SDRAM_CMD_AUTO_REFRESH	0b011

#define SDRAM_CMD_LOAD_MODE_REG	0b100
#define SDRAM_CMD_SELF_REFRESH	0b101
#define SDRAM_CMD_POWER_DOWN	0b110

#define SDRAM_DEFAULT_MODE_VAL 	0b0

#define SDRAM_MODE_REG_WRITE_BURST_Pos 		9
#define SDRAM_MODE_REG_OPERATING_MODE_Pos 	7
#define SDRAM_MODE_REG_CAS_LATENCY_Pos 		4
#define SDRAM_MODE_REG_BURST_TYPE_Pos 		3
#define SDRAM_MODE_REG_BURST_LEN_Pos 		0

#define SDRAM_MODE_REG_WRITE_BURST_SINGLE_LOCATION_ACCESS 0b1
#define SDRAM_MODE_REG_OPERATING_MODE_STANDARD			  0b0
#define SDRAM_MODE_REG_CAS_LATENCY_2					 0b10
#define SDRAM_MODE_REG_BURST_TYPE_SEQUENTIAL			  0b0
#define SDRAM_MODE_REG_BURST_LEN_1 						  0b0

//TODO: Add calculation
#define SDRAM_RTR_COUNT_VAL 210

#define SDRAM_BANK_ADDR ((uint8_t*)0xD0000000)
#define SDRAM_SIZE 0x800000

#define SMALL_BUF_SIZE 4
#define MEDIUM_BUF_SIZE 64
#define LARGE_BUF_SIZE 255
/*
 *	Function to initialize the SDRAM.
 *	It first configures all the port pins required for functioning, after than it follows the
 *	initialization process provided in the reference manual.
 *	Once that is complete the memory range 0xD0000000 to 0xD08000000 is available for use
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void init_sdram();
#endif

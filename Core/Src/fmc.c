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
 * @file    fmc.c
 * @brief   FMC driver code to make use of on board SDRAM on STM32f429 Discovery board
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
#include "fmc.h"
#include "stdint.h"
#include "stm32f429xx.h"
#include "systick.h"
#include "string.h"

#define TWO_BIT_MASK 0b11
#define FOUR_BIT_MAKS 0b1111
#define ALT_FUNC_MODE_MASK 0b10
#define ALT_FUNC_12_MASK 0b1100
#define OSPEED_VHIGH_MASK 0b11

/*
 *	Function to set the configuration of a pin which is used for the SDRAM.
 *	The Configuration is:
 *
 *	Alternate Function Mode 12
 *	No Pull up or Pull down (default)
 *	Output Speed Very High (0b11)
 *
 * Parameters:
 *  gpio pointer to gpio structure whose pin is to be configured
 *  pin integer pin number which is to be configured
 *
 * Returns:
 *  none
 */
void set_pin_func(GPIO_TypeDef* gpio,int pin){
	gpio->MODER &= ~(TWO_BIT_MASK << (2*pin));
	gpio->MODER |= ALT_FUNC_MODE_MASK << (2*pin);
	gpio->OSPEEDR &= ~(TWO_BIT_MASK << (2*pin));
	gpio->OSPEEDR |= OSPEED_VHIGH_MASK << (2*pin);
	gpio->PUPDR &= ~(TWO_BIT_MASK << (2*pin));

	if(pin <= 7){//if pin number is less than 7, it is in AFRL, else in AFRH
		gpio->AFR[0] &= ~(FOUR_BIT_MAKS << (4*pin));
		gpio->AFR[0] |= ALT_FUNC_12_MASK << (4*pin);
	}else{
		gpio->AFR[1] &= ~(FOUR_BIT_MAKS << (4*(pin-8)));// -8 to offset the pin number from register position
		gpio->AFR[1] |= ALT_FUNC_12_MASK << (4*(pin-8));
	}
}
//pin requirements can be seen from FMC block diagram in reference manual and the board schematic
static const uint8_t GPIOB_FMC_PINS[] = {5,6};
static const uint8_t GPIOB_FMC_PINS_LEN = sizeof(GPIOB_FMC_PINS)/sizeof(GPIOB_FMC_PINS[0]);

static const uint8_t GPIOC_FMC_PINS[]  = {0};
static const uint8_t GPIOC_FMC_PINS_LEN = sizeof(GPIOC_FMC_PINS)/sizeof(GPIOC_FMC_PINS[0]);

static const uint8_t GPIOD_FMC_PINS[]  = {0,1,8,9,10,14,15};
static const uint8_t GPIOD_FMC_PINS_LEN = sizeof(GPIOD_FMC_PINS)/sizeof(GPIOD_FMC_PINS[0]);

static const uint8_t GPIOE_FMC_PINS[]  = {0,1,7,8,9,10,11,12,13,14,15};
static const uint8_t GPIOE_FMC_PINS_LEN = sizeof(GPIOE_FMC_PINS)/sizeof(GPIOE_FMC_PINS[0]);

static const uint8_t GPIOF_FMC_PINS[]  = {0,1,2,3,4,5,11,12,13,14,15};
static const uint8_t GPIOF_FMC_PINS_LEN = sizeof(GPIOF_FMC_PINS)/sizeof(GPIOF_FMC_PINS[0]);

static const uint8_t GPIOG_FMC_PINS[]  = {0,1,4,5,8,15};
static const uint8_t GPIOG_FMC_PINS_LEN = sizeof(GPIOG_FMC_PINS)/sizeof(GPIOG_FMC_PINS[0]);


/*
 *	Block delay function to wait while SDRAM Busy Flag is set
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void delay_sdram_busy(){
	while(FMC_Bank5_6->SDSR & FMC_SDSR_BUSY_Msk);
}

/*
 *	Function to send command to SDRAM
 *
 * Parameters:
 *  cmd the command to send to the sdram
 *  mode_reg the value of mode register when load mode register command is to be used
 *
 * Returns:
 *  none
 */
void send_sdram_cmd(uint8_t cmd,uint16_t mode_reg){
	uint32_t to_send = 0;
	to_send |= FMC_SDCMR_CTB2_Msk;
	to_send |= FMC_CMR_NRFS_VAL<<FMC_SDCMR_NRFS_Pos;
	to_send |= mode_reg << FMC_SDCMR_MRD_Pos;
	to_send |= cmd << FMC_SDCMR_MODE_Pos;

	delay_sdram_busy();

	FMC_Bank5_6->SDCMR= to_send;
}

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
void init_sdram(){
    uint32_t fmc_tr = 0, fmc_cr = 0;
    uint16_t fmc_mode_reg = 0;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN_Msk | RCC_AHB1ENR_GPIOCEN_Msk | RCC_AHB1ENR_GPIODEN_Msk |
			  	  	  RCC_AHB1ENR_GPIOEEN_Msk | RCC_AHB1ENR_GPIOFEN_Msk | RCC_AHB1ENR_GPIOGEN_Msk;

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN_Msk;
	SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD_Msk;
    while(SYSCFG->CMPCR & SYSCFG_CMPCR_READY_Msk);

    RCC->AHB3ENR |= RCC_AHB3ENR_FMCEN_Msk;

    for(int i = 0; i < GPIOB_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOB, GPIOB_FMC_PINS[i]);
    }

    for(int i = 0; i < GPIOC_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOC, GPIOC_FMC_PINS[i]);
    }

    for(int i = 0; i < GPIOD_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOD, GPIOD_FMC_PINS[i]);
    }

    for(int i = 0; i < GPIOE_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOE, GPIOE_FMC_PINS[i]);
    }

    for(int i = 0; i < GPIOF_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOF, GPIOF_FMC_PINS[i]);
    }

    for(int i = 0; i < GPIOG_FMC_PINS_LEN;i++){
    	set_pin_func(GPIOG, GPIOG_FMC_PINS[i]);
    }

    //The values for timings are calculated on the basis of a 160MHz clock
    fmc_tr |= FMC_TR_TMRD_VAL<<FMC_SDTR1_TMRD_Pos;
    fmc_tr |= FMC_TR_TXSR_VAL<<FMC_SDTR1_TXSR_Pos;
    fmc_tr |= FMC_TR_TRAS_VAL<<FMC_SDTR1_TRAS_Pos;
    fmc_tr |= FMC_TR_TRC_VAL<<FMC_SDTR1_TRC_Pos;
    fmc_tr |= FMC_TR_TWR_VAL<<FMC_SDTR1_TWR_Pos;
    fmc_tr |= FMC_TR_TRP_VAL<<FMC_SDTR1_TRP_Pos;
    fmc_tr |= FMC_TR_TRCD_VAL<<FMC_SDTR1_TRCD_Pos;

    FMC_Bank5_6->SDTR[1] = fmc_tr;
    FMC_Bank5_6->SDTR[0] = 0;
    FMC_Bank5_6->SDTR[0] = (fmc_tr & FMC_TR_DNC_MASK);//certain parameters in the TR are DNC for
    												  //bank 2, therefore they must be copied into
    												  //TR for bank 1

    fmc_cr |= FMC_CR_NC_8_BITS<<FMC_SDCR1_NC_Pos;
    fmc_cr |= FMC_CR_NR_12_BITS<<FMC_SDCR1_NR_Pos;
    fmc_cr |= FMC_CR_MWID_16_BITS<<FMC_SDCR1_MWID_Pos;
    fmc_cr |= FMC_CR_NB_2_BANKS<<FMC_SDCR1_NB_Pos;
    fmc_cr |= FMC_CR_CAS_LATENCY_2<<FMC_SDCR1_CAS_Pos;
    fmc_cr |= FMC_CR_WP_NONE<<FMC_SDCR1_WP_Pos;
    fmc_cr |= FMC_CR_SDCLK_2X<<FMC_SDCR1_SDCLK_Pos;
    fmc_cr |= FMC_CR_RPIPE_1<<FMC_SDCR1_RPIPE_Pos;

    FMC_Bank5_6->SDCR[1] = 0;
    FMC_Bank5_6->SDCR[1] = fmc_cr;
    FMC_Bank5_6->SDCR[0] = 0;
    FMC_Bank5_6->SDCR[0] = (fmc_cr & FMC_CR_DNC_MASK);//certain parameters in the CR are DNC for
	  	  	  	  	  	  	  	  	  	  	  	  	  //bank 2, therefore they must be copied into
	  	  	  	  	  	  	  	  	  	  	  	  	  //CR for bank 1

    //following is the initialization process according to the datasheet
    send_sdram_cmd(SDRAM_CMD_CLOCK_ENABLE, SDRAM_DEFAULT_MODE_VAL);

    b_delay(1);//must be more than 100us

    send_sdram_cmd(SDRAM_CMD_PALL,SDRAM_DEFAULT_MODE_VAL);

    send_sdram_cmd(SDRAM_CMD_AUTO_REFRESH,SDRAM_DEFAULT_MODE_VAL);

    //configure the SDRAM for single location access, standard operating mode, cas latency of 2, sequential access
    //and burst length of 1
    fmc_mode_reg |= SDRAM_MODE_REG_WRITE_BURST_SINGLE_LOCATION_ACCESS<<SDRAM_MODE_REG_WRITE_BURST_Pos;
    fmc_mode_reg |= SDRAM_MODE_REG_OPERATING_MODE_STANDARD<<SDRAM_MODE_REG_OPERATING_MODE_Pos;
    fmc_mode_reg |= SDRAM_MODE_REG_CAS_LATENCY_2<<SDRAM_MODE_REG_CAS_LATENCY_Pos;
    fmc_mode_reg |= SDRAM_MODE_REG_BURST_TYPE_SEQUENTIAL<<SDRAM_MODE_REG_WRITE_BURST_Pos;
    fmc_mode_reg |= SDRAM_MODE_REG_BURST_LEN_1<<SDRAM_MODE_REG_BURST_LEN_Pos;

    send_sdram_cmd(SDRAM_CMD_LOAD_MODE_REG, fmc_mode_reg);

    //refresh counter value based on formula provided in reference manual
    FMC_Bank5_6->SDRTR = (SDRAM_RTR_COUNT_VAL<<FMC_SDRTR_COUNT_Pos);

    memset((uint32_t*)SDRAM_BANK_ADDR,0,SDRAM_SIZE);//to clear the memory
    b_delay(100);
}

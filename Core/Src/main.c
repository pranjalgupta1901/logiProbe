/*******************************************************************************
 * Copyright (C) 2023 by Krish Shah and Pranjal Gupta
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Krish Shah, Pranjal Gupta ,and the University of Colorado are not liable for
 * any misuse of this material.
 * ****************************************************************************/

/**
 * @file    main.c
 * @brief   main file for LogiProbe Logic Analyzer Project. Completed for ECEN5613:
 * 			Embedded System Design final project.
 *
 * 			This file initializes all the peripherals and then runs the command processor.
 *
 * @author  Krish Shah and Pranjal Gupta
 * @date    December 17 2023
 *
 */
#include "fatfs_sd.h"
#include "main.h"
#include "fatfs.h"
#include "stdio.h"
#include "pll_clock.h"
#include "uart.h"
#include "systick.h"
#include "i2c_analyser.h"
#include "state_mode.h"
#include "stdbool.h"
#include "timing_mode_init.h"
#include "fmc.h"
#include "user_fatfs.h"
#include "cmd_processor.h"
#include "spi.h"

int main(void)
{

  init_clocks();
  init_uart();
  init_systick();
  spi_init();
  spi_gpio_pin_init();
  MX_FATFS_Init();
  init_sdram();

  /* Infinite loop */
  while (1)
  {
	  run_command_processor();
  }

}

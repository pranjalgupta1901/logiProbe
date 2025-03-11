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
 * @file    systick.c
 * @brief   Driver code for Systick. A tick is generated very 1ms
 *
 *
 * @author  Krish Shah
 * @date    October 27 2023
 *
 */
#include "systick.h"
#include "stm32f429xx.h"

extern uint16_t Timer1, Timer2;

#define SYSTICK_LOAD_VALUE 20000//for tick every 1ms

ticktime_t tick = 0;
ticktime_t clock_tick = 0;
/*
 * Initializes the Systick timer. It is configured to generate an interrupt every 1ms which is used to
 * increment the tick variable.
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void init_systick()
{
	SysTick->LOAD = SYSTICK_LOAD_VALUE;
	SysTick->VAL = 0;//to clear SYST_CVR, since startup value is unknown
	NVIC_EnableIRQ(SysTick_IRQn);
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/*
 * A function to get the current number of ticks elapsed in the system since startup
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  ticktime_t returns the current number of ticks elapsed
 */
ticktime_t now()
{
	return tick;
}

/*
 * A function to reset clock ticks
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void reset_clock(){
	clock_tick = tick;
}

/*
 * A function to get current clock ticks since reset
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  ticktime_t number of ticks since last reset
 */
ticktime_t get_clock(){
	return (tick-clock_tick);
}

/*
 * Function to create a blocking delay call for a given number of
 * ms.
 *
 * Parameters:
 *  ms number of ms to delay for
 *
 * Returns:
 *  none
 */
void b_delay(int ms){
	reset_clock();
	while(get_clock() < ms);
}



/*
 * Systick Interrupt Handler is used to increment the value of the ticks variable .
 *
 * Parameters:
 *  none
 *
 * Returns:
 *  none
 */
void SysTick_Handler()
{
	if(Timer1 > 0)//fatfs state variables handling
		Timer1--;

	if(Timer2 > 0)
		Timer2--;

	tick++;
}

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
 * @file    cmd_processor.c
 * @brief   Code for UART based Command processor to creating a user interface
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
#include "cmd_processor.h"
#include "stdio.h"
#include "stdint.h"
#include "uart.h"
#include "string.h"
#include "ctype.h"
#include "unistd.h"
#include "stdbool.h"
#include "timing_mode_init.h"
#include "state_mode.h"
#include "i2c_analyser.h"
#include "fmc.h"
#include "stdlib.h"
#include "user_fatfs.h"

#define CMD_PROCESSOR_LINE_BUFFER_SIZE 256
#define CMD_PROCESSOR_ARGV_SIZE 64
#define iseot(x) (((x == ' ')||(x == '\r'))?(1):(0)) //end of token can only be space or cr
#define ishyphen(x) ((x == '-'))

/* Function to get a line input from the uart using get_char. The line is delimited by
 * a  carriage return character. This function also handles the backspace capability of
 * the command processor.
 *
 * Parameters:
 * 	line_buffer(out) pointer to byte buffer where the line input is saved and returned
 *
 * Returns:
 *  none
 */
void get_line(uint8_t line_buffer[]) {
	uint8_t *line_wr_ptr = line_buffer;
	uint8_t byte;

	while (1) {
		byte = get_char();

		if (byte == '\b') {
			if (line_wr_ptr == line_buffer) {
				continue; //to prevent backspace at the start of the line
			}
			printf("\b \b");
			line_wr_ptr--;  //to move the pointer back 1 place to erase previous
							//character in the line buffer
			*line_wr_ptr = '\0';
		} else {
			printf("%c", byte);	//echo user input and save it in the line buffer
			*line_wr_ptr = byte;
			line_wr_ptr++;
		}
		if (byte == '\r') {//exit on carriage return
			printf("\n");
			break;
		}
	}
}

/* Function to tokenise a line buffer and return argc and agrv values. argc is the
 * number of tokens and argv is a pointer to the start address of those tokens.
 *
 * Parameters:
 * 	line(in/out) pointer to byte buffer where the line input is saved and returned
 * 	argc(out) pointer an integer holding the value of the number of tokens
 * 	argv(out) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void get_tokens(uint8_t line[], uint8_t *argc, uint8_t *argv[]) {
	uint8_t *ptr = line;

	uint8_t *end = line;
	while (*end != '\r') {//after this while loop, it will point to \r
		end++;
	}

	while (ptr <= end) {
		if (isalpha(*ptr) || isdigit(*ptr) || ishyphen(*ptr)) {
			argv[*argc] = ptr;
			(*argc)++;
			while (!iseot(*ptr)) {	//a '\r' or a ' ' shows the end of a token
				ptr++;
			}
			*ptr = '\0';//end of each token should be a '\0' for null termination
		}
		ptr++;
	}
}

typedef void (*command_handler_t)(int argc, char *argv[]);
void invalid_handler(int argc, char *argv[]);
void help_handler(int argc, char *argv[]);
void timing_mode_handler(int argc, char *argv[]);
void state_mode_handler(int argc, char *argv[]);
void save_handler(int argc, char *argv[]);
void analyser_handler(int argc, char *argv[]);

typedef struct {
	const char *name;
	command_handler_t handler;
	const char *help_string;
} command_table_t;

static const command_table_t commands[] =
		{
				{ "HELP", help_handler,
						"Displays the help menu with a list of available commands\r\n" },
				{ "TMODE", timing_mode_handler,
						"Run the Timing mode of the logic analyzer\r\n\n"
								"	-m {select the mode of acquisition, defaults to button mode}\r\n"
								"	-f {select the frequency of acquisition, it can be one of [100,200,400,800,1000].defaults to 400}\r\n"
								"	-i {selects the interpreter, it can be [i2c], it defaults to no intepreter selected}\r\n"
								"	-s {selects the size of acquisition, it can be [s,m,l], it defaults to small}\r\n"
								"	for i2c interpreter, SDA must be connected to P1, and SCL to P0}\r\n" },
				{ "SMODE", state_mode_handler,
						"Run the State mode of the logic analyzer\r\n\n"
								"	-e {selects the edge at which to sample, can be [r,f,b],defaults to rising edge}\r\n"
								"	-m {selects the mode of acquisition, it can be [button,trigger],default to button}\r\n"
								"	-s {selects the size of acquisition, it can be [s,m,l], it defaults to small}\r\n"
								"	-p {selects the pin for trigger detection, it can be from 0..7,no default value}\r\n"
								"	-t {selects the pattern for trigger, must be a hex number in the format 0x..,no default value}\r\n"
								"	-d {selects the timeout delay in ms for exit incase trigger not detected, defaults to 100000}\r\n"
								"	-t -d and -p fields are only used if trigger mode is selected, otherwise they are ignored.}\r\n" },
				{ "ANALYSE", analyser_handler,
						"Run the Interpreter of choice on the data\r\n\n"
								"	-m {select the mode of analysis, it can be [i2c],defaults to i2c mode}\r\n"
								"	-s {selects the size of interpreter, it can be [s,m,l], it defaults to small}\r\n" },
				{ "SAVE", save_handler,
						"Save the Data on the SD Card\r\n\n"
								"	-s {selects the size of save, it can be [s,m,l], it defaults to small}\r\n" }, };
static const int num_commands = sizeof(commands) / sizeof(commands[0]);

/*
 * Function to handle invalid input
 *
 * 	Example:
 *	> <invalid command>
 * 	Unknown Command(<invalid command>)
 *
 * Parameters:
 * 	argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void invalid_handler(int argc, char *argv[]) {
	int i = 0;
	printf("Unknown Command(");
	while (i < argc) {
		char *strptr = *(argv + i);
		while (*strptr) {
			printf("%c", *strptr);
			strptr++;
		}
		printf(" ");
		i++;
	}
	printf("\b)\r\n");
}

/*
 * Callback function for timing mode command. It first uses the getopt function to match the
 * flags (for example: -f) with it parameter( for example: frequency). Using getopt function allows
 * the user to enter commands parameters in any order, without following some arbitrary predefined
 * order.
 *
 * This function first validates if all parameters are received, or it initialses them with a default
 * value. Then it checks if the inputs are in a permissible range or not. After that it runs the function
 * call to run the timing mode of the logic analyser
 *
 * 	-m {select the mode of acquisition, defaults to button mode
 *	-f {select the frequency of acquisition, it can be one of [100,200,400,800,1000].defaults to 400
 *	-i {selects the interpreter, it can be [i2c], it defaults to no intepreter selected
 *	-s {selects the size of acquisition, it can be [s,m,l], it defaults to small
 *	for i2c interpreter, SDA must be connected to P1, and SCL to P0
 *
 * Parameters:
 * 	argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void timing_mode_handler(int argc, char *argv[]) {
	timing_mode_freq_t timing_freq = 0;
	uint8_t _mode = 0;
	int count = 0;
	int8_t c;
	bool is_i2c_used = false;
	char freq[5], mode[10], i[4], s[10];
	bool gotfreq = false, gotmode = false, goti = false, gots = false;
	bool isfreqvalid = false, ismodevalid = false, isi2cvalid = false,
			issizevalid = false;
	optind = 0;
	while (1) {
		c = getopt(argc, (char**) argv, "f:m:i:s:");
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'f':
			strcpy(freq, optarg);
			gotfreq = true;
			break;
		case 'm':
			strcpy(mode, optarg);
			gotmode = true;
			break;
		case 'i':
			strcpy(i, optarg);
			goti = true;
			break;
		case 's':
			strcpy(s, optarg);
			gots = true;
			break;
		case '?':
			printf("\r\n");
			return;
			break;
		}
	}

	printf("\r\n");
	if ((gotfreq && gotmode && goti && gots) != true) {
		printf("All Arguments not received!\r\n");
		printf("List of Missing Arguments:\r\n");
		if (!gotfreq) {
			printf("Frequency, initialized to default value 400\r\n");
			strcpy(freq, "400");
		}
		if (!gotmode) {
			gotmode = true;
			printf("Mode, initialized to button mode\r\n");
			strcpy(mode, "button");
		}
		if (!goti) {
			printf("No interpreter selected, initialized to off\r\n");
		}
		if (!gots) {
			printf("Size, initialized to Small\r\n");
			strcpy(s, "s");
		}
		printf("\r\n");
	}

	for (int i = 0; i < freq_table_len; i++) {
		if (strcasecmp(freq, freq_table[i]) == 0) {
			isfreqvalid = true;
			timing_freq = i;
			break;
		}
	}
	if (!isfreqvalid) {
		printf("Invalid Frequency Provided!\r\n");
		printf("Frequency must be one of the following(in kHz):\r\n");
		for (int i = 0; i < freq_table_len; i++) {
			printf("%s\r\n", freq_table[i]);
		}
	}

	if (strcasecmp(s, "s") == 0) {
		issizevalid = true;
		count = SMALL_BUF_SIZE;
	} else if (strcasecmp(s, "m") == 0) {
		issizevalid = true;
		count = MEDIUM_BUF_SIZE;
	} else if (strcasecmp(s, "l") == 0) {
		issizevalid = true;
		count = LARGE_BUF_SIZE;
	}

	if (strcasecmp(mode, "button") == 0) {
		ismodevalid = true;
		_mode = BUTTON_MODE;
	}

	if (strcasecmp(i, "i2c") == 0) {
		isi2cvalid = true;
		is_i2c_used = true;
	} else {
		isi2cvalid = true;
		is_i2c_used = false;
	}

	if (!ismodevalid) {
		printf("Invalid Mode Provided!\r\n");
		printf("Mode must be:\r\n");
		printf("Button");
	}

	if (!isi2cvalid) {
		printf("Invalid Choice for Interpreter\r\n");
		printf("Choice must be:\r\n");
		printf("I2C\r\n");
		printf("<none>\r\n");
	}

	if (!issizevalid) {
		printf("Invalid Size\r\n");
		printf("Size must be greater than 0 and less than 255\r\n");
	}

	if ((isfreqvalid && ismodevalid && issizevalid && isi2cvalid) == false) {
		printf(
				"Invalid Configuration Provided. Returning without execution\r\n");
		return;
	}

	printf("Configuration Valid!\r\n");
	printf("Frequency set to %s\r\n", freq);
	printf("Mode is set to %s\r\n", mode);
	if (strcasecmp(i, "i2c") == 0) {
		printf("I2C Interpreter Selected\r\n");
	} else {
		printf("No Interpreter Selected\r\n");
	}
	printf("Size Count is set to %s\r\n", s);
	printf("Press Button to begin acquisition...\r\n");

	if (timing_mode_init(_mode, timing_freq, is_i2c_used, count) == true) {
		printf("Logic Capture Completed successfully\r\n");
	} else {
		printf("Logic Capture not successful\r\n");
	}
}

/*
 * Callback function for state mode command. It first uses the getopt function to match the
 * flags (for example: -e) with it parameter( for example: edge). Using getopt function allows
 * the user to enter commands parameters in any order, without following some arbitrary predefined
 * order.
 *
 * This function first validates if all parameters are received, or it initialses them with a default
 * value. Then it checks if the inputs are in a permissible range or not. After that it runs the function
 * call to run the state mode of the logic analyzer
 *
 * -e {selects the edge at which to sample, can be [r,f,b],defaults to rising edge}
 * -m {selects the mode of acquisition, it can be [button,trigger],default to button}
 * -s {selects the size of acquisition, it can be [s,m,l], it defaults to small}
 * -p {selects the pin for trigger detection, it can be from 0..7,no default value}
 * -t {selects the pattern for trigger, must be a hex number in the format 0x..,no default value}
 * -d {selects the timeout delay in ms for exit incase trigger not detected, defaults to 100000}
 * -t -d and -p fields are only used if trigger mode is selected, otherwise they are ignored.
 *
 * Parameters:
 * 	argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void state_mode_handler(int argc, char *argv[]) {
	optind = 0;
	int8_t c = 0;
	char edge[2], mode[8], pin[2], size[8], trigger_pattern[5], delay[10];
	bool gotedge = false, gotmode = false, gotpin = false, gotsize = false,
			gotpattern = false, gotdelay = false;
	bool invalid_config = false;
	uint32_t _delay_timeout = 0;
	while (1) {
		c = getopt(argc, (char**) argv, "e:m:p:s:t:d:");
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'e':
			strcpy(edge, optarg);
			gotedge = true;
			break;
		case 'm':
			strcpy(mode, optarg);
			gotmode = true;
			break;
		case 'p':
			strcpy(pin, optarg);
			gotpin = true;
			break;
		case 's':
			strcpy(size, optarg);
			gotsize = true;
			break;
		case 't':
			strcpy(trigger_pattern, optarg);
			gotpattern = true;
			break;
		case 'd':
			strcpy(delay, optarg);
			gotdelay = true;
			break;
		case '?':
			printf("\r\n");
			return;
			break;
		}
	}
	printf("\r\n");

	if ((gotedge && gotmode && gotpin && gotsize && gotpattern) == false) {
		printf("All Arguments not received!\r\n");
		printf("List of Missing Arguments:\r\n");
		if (!gotedge) {
			printf("Edge, initialized to default value rising edge\r\n");
			strcpy(edge, "r");
		}
		if (!gotmode) {
			gotmode = true;
			printf("Mode, initialized to button mode\r\n");
			strcpy(mode, "button");
		}
		if (strcasecmp(mode, "trigger") == 0) {
			if (!gotpin) {
				printf(
						"Pin not selected, trigger initialized to off, using button mode instead\r\n");
				strcpy(mode, "button");
			}
			if (!gotpattern) {
				printf(
						"Pattern not provided, trigger initialized to off, using button mode instead\r\n");
			}
		}
		if (!gotsize) {
			printf("Size, initialized to Small\r\n");
			strcpy(size, "s");
		}
		if (!gotdelay) {
			printf("Delay Timeout, initialized to 100000\r\n");
			strcpy(delay, "100000");
		}
		printf("\r\n");
	}

	input_capture_edge_t _edge = 0;
	if (strcasecmp(edge, "r") == 0) {
		_edge = RISING_EDGE;
	} else if (strcasecmp(edge, "f") == 0) {
		_edge = FALLING_EDGE;
	} else if (strcasecmp(edge, "b") == 0) {
		_edge = RISING_FALLING_EDGE;
	} else {
		printf("Invalid Option for Edge Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("R\r\n");
		printf("F\r\n");
		printf("B\r\n");
		invalid_config = true;
	}
	uint8_t _mode = 0;
	if (strcasecmp(mode, "button") == 0) {
		_mode = 2;							//TODO: replace with macro
	} else if (strcasecmp(mode, "trigger") == 0) {
		_mode = 1;							//TODO: replace with macro
	} else {
		printf("Invalid Option for Mode Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("Trigger Mode\r\n");
		printf("Button Mode\r\n");
		invalid_config = true;
	}

	uint16_t _count = 0;
	if (strcasecmp(size, "s") == 0) {
		_count = SMALL_BUF_SIZE;							//TODO: Make macros
	} else if (strcasecmp(size, "m") == 0) {
		_count = MEDIUM_BUF_SIZE;
	} else if (strcasecmp(size, "l") == 0) {
		_count = LARGE_BUF_SIZE;
	} else {
		printf("Invalid Option for Count Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("S\r\n");
		printf("M\r\n");
		printf("L\r\n");
		invalid_config = true;
	}
	uint8_t _bitpattern = 0;
	uint8_t _pin = 0;
	if (_mode == 1) {
		_bitpattern = strtoul(trigger_pattern, NULL, 16);
		sscanf(pin, "%d", (int*) &_pin);
		if (_pin >= 8) {
			printf("Invalid Option for Pin Selected\r\n");
			printf("Must range from 0..7\r\n");
			invalid_config = true;
		}
	}

	sscanf(delay, "%d", (int*) &_delay_timeout);

	if (invalid_config) {
		printf(
				"Invalid Configuration Provided. Returning without execution\r\n");
		return;
	} else {
		printf("Configuration is Valid!\r\n");
		printf("Edge set to %s\r\n", edge);
		printf("Mode set to %s\r\n", mode);
		printf("Size set to %s\r\n", size);
		printf("Delay Timeout Set to %s\r\n", delay);
		if (_mode == 1) {
			printf("Trigger Pin set to %s\r\n", pin);
			printf("Trigger Pattern set to 0x%x\r\n", _bitpattern);
		}
	}
	if (_mode == 1) {
		printf("Acquisition will begin on trigger detection...\r\n");
	} else if (_mode == 2) {
		printf("Press button to begin acquisition...\r\n");
	}
	if (state_timing_init(_edge, _mode, _bitpattern, _count, _pin, 100) == true) {
		printf("Logic Capture Completed successfully\r\n");
	} else {
		printf("Logic Capture not successful\r\n");
	}
}

/*
 * Callback function for the analyse command. It runs the analyzer on the saved buffer.
 * It first uses the getopt function to match the
 * flags (for example: -s) with it parameter( for example: size). Using getopt function allows
 * the user to enter commands parameters in any order, without following some arbitrary predefined
 * order.
 *
 * This function first validates if all parameters are received, or it initialses them with a default
 * value. Then it checks if the inputs are in a permissible range or not. After that it runs the function
 * call to run the analyser of the logic analyzer
 *
 * -m {select the mode of analysis, it can be [i2c],defaults to i2c mode}
 * -s {selects the size of interpreter, it can be [s,m,l], it defaults to small}
 *
 * Parameters:
 *  argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void analyser_handler(int argc, char *argv[]) {
	optind = 0;
	int8_t c = 0;
	char mode[4], size[2];
	bool gotmode = false, gotsize = false;
	uint8_t mode_flag = 0;
	bool invalid_config = false;
	uint8_t _count = 0;

	while (1) {
		c = getopt(argc, (char**) argv, "m:s:");
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'm':
			strcpy(mode, optarg);
			gotmode = true;
			break;
		case 's':
			strcpy(size, optarg);
			gotsize = true;
			break;
		case '?':
			printf("\r\n");
			return;
			break;
		}
	}
	printf("\r\n");

	if ((gotmode && gotsize) == false) {
		printf("All Arguments not received!\r\n");
		printf("List of Missing Arguments:\r\n");
		if (!gotmode) {
			gotmode = true;
			printf("Mode, initialized to I2C Analyser\r\n");
			strcpy(mode, "i2c");
		}
		if (!gotsize) {
			printf("Size, initialized to Small\r\n");
			strcpy(size, "s");
		}
	}

	if (strcasecmp(size, "s") == 0) {
		_count = SMALL_BUF_SIZE;
	} else if (strcasecmp(size, "m") == 0) {
		_count = MEDIUM_BUF_SIZE;
	} else if (strcasecmp(size, "l") == 0) {
		_count = LARGE_BUF_SIZE;
	} else {
		printf("Invalid Option for Count Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("S\r\n");
		printf("M\r\n");
		printf("L\r\n");
		invalid_config = true;
	}

	if (strcasecmp(mode, "i2c") == 0) {
		mode_flag = 1;
	} else {
		printf("Invalid Option for Mode Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("I2C\r\n");
		invalid_config = true;
	}

	if (invalid_config) {
		printf(
				"Invalid Configuration Provided. Returning without execution\r\n");
		return;
	} else {
		printf("Configuration is Valid!\r\n");
		printf("Mode set to %s\r\n", mode);
		printf("Size set to %s\r\n", size);
	}

	uint32_t buf_len = 0;
	buf_len = _count * 32768;

	if (mode_flag == 1) {
		printf("Running I2C Analyzer!\r\n");
		run_analyser(SDRAM_BANK_ADDR, buf_len, 0, 1);
		printf("Done Running I2C Analyzer!\r\n");

	}
}

/*
 * Callback function for the save command. It runs the function to save buffer to sdcard
 * It first uses the getopt function to match the
 * flags (for example: -s) with it parameter( for example: size). Using getopt function allows
 * the user to enter commands parameters in any order, without following some arbitrary predefined
 * order.
 *
 * This function first validates if all parameters are received, or it initialses them with a default
 * value. Then it checks if the inputs are in a permissible range or not. After that it runs the function
 * call to run the analyser of the logic analyzer
 *
 * -s {selects the size of interpreter, it can be [s,m,l], it defaults to small}
 *
 * Parameters:
 *  argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void save_handler(int argc, char *argv[]) {
	optind = 0;
	int8_t c = 0;
	char size[2];
	bool gotsize = false;
	uint8_t _count = 0;
	bool invalid_config = false;

	while (1) {
		c = getopt(argc, (char**) argv, "s:");
		if (c == -1) {
			break;
		}
		switch (c) {
		case 's':
			strcpy(size, optarg);
			gotsize = true;
			break;
		case '?':
			printf("\r\n");
			return;
			break;
		}
	}
	printf("\r\n");
	if (!gotsize) {
		printf("All Arguments not received!\r\n");
		printf("List of Missing Arguments:\r\n");
		if (!gotsize) {
			printf("Size, initialized to Small\r\n");
			strcpy(size, "s");
		}
	}

	if (strcasecmp(size, "s") == 0) {
		_count = SMALL_BUF_SIZE;
	} else if (strcasecmp(size, "m") == 0) {
		_count = MEDIUM_BUF_SIZE;
	} else if (strcasecmp(size, "l") == 0) {
		_count = LARGE_BUF_SIZE;
	} else {
		printf("Invalid Option for Count Selected\r\n");
		printf("Must be one of the following\r\n");
		printf("S\r\n");
		printf("M\r\n");
		printf("L\r\n");
		invalid_config = true;
	}

	if (invalid_config) {
		printf(
				"Invalid Configuration Provided. Returning without execution\r\n");
		return;
	} else {
		printf("Configuration is Valid!\r\n");
		printf("Size set to %s\r\n", size);
	}

	printf("Saving Data on SD Card!\r\n");
	if (user_fatfs_init(_count)) {
		printf("Done Saving Data on SD Card!\r\n");
	} else {
		if (user_fatfs_init(_count)) {
			printf("Done Saving Data on SD Card!\r\n");
		} else {
			printf("SD Card Save Failed!\r\n");
		}

	}
}

/*
 * Callback function to run the help menu, which prints out a list of all the commands as well
 * as their parameters
 *
 * Parameters:
 *  argc(in) integer holding the value of the number of tokens
 * 	argv(in) array of pointers to an byte holding the start address of those tokens
 *
 * Returns:
 *  none
 */
void help_handler(int argc, char *argv[]) {
	printf("Commands Available:\r\n");
	for (int i = 0; i < num_commands; i++) {
		printf("%s:\r\n", commands[i].name);
		printf("\t%s\r\n", commands[i].help_string);
	}
}

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
void run_command_processor() {
	uint8_t line[CMD_PROCESSOR_LINE_BUFFER_SIZE] = { 0 };
	uint8_t argc = 0, *argv[CMD_PROCESSOR_ARGV_SIZE] = { 0 };

	printf("> ");
	get_line(line);
	get_tokens(line, &argc, argv);

	if (argc == 0) {
		return;
	}

	int i = 0;
	while (i < num_commands) {
		if (!strcasecmp((const char*) argv[0], commands[i].name)) {
			commands[i].handler(argc, (char**) argv);
			break;
		}
		i++;
	}
	if (i == num_commands) {//if no match occurs after iterating over command table, raise an error
		invalid_handler(argc, (char**) argv);
	}
}

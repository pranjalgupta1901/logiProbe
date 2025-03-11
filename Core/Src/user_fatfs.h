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
 * @file    user_fatfs.h
 * @brief   This file contains the function prototype which carries out the operation related to writing
 * 			of the data on the SD Card using Fatfs drivers.
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 *
 *
 */



#ifndef __USER_FATFS_H__
#define __USER_FATFS_H__
#include "stdint.h"
#include "stdbool.h"
bool user_fatfs_init(uint16_t count);
#endif

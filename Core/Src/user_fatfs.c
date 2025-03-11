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
 * @file    user_fatfs.c
 * @brief   This file contains the function definitions and algorithms for the file writing using fatfs driver.
 * 			This file uses STM32CUBEIDE HAL generated middleware related to file systems. The algorithm in which how we wanted
 * 			our file should be and in which arrangement is writte by me.
 * @author  Pranjal Gupta
 * @date    12/17/2023
 *
 */


#include "fatfs.h"
#include "stdint.h"
#include "user_fatfs.h"
#include "fatfs_sd.h"
#include "fmc.h"
#include"stdio.h"
#include "string.h"


FATFS fs;
FATFS *pfs;
FIL fil;
FIL fil1;
FRESULT fres;
DWORD fre_clust;
uint32_t totalSpace, freeSpace;
uint8_t *fill_address = SDRAM_BANK_ADDR;
uint8_t file_num = 1;

bool user_fatfs_init(uint16_t count){

		if(f_mount(&fs, "", 0) != FR_OK){
			return false;
		}
		char filename[15];

		do{
			sprintf(filename, "file%d.txt", file_num);
			file_num++;
		}while(f_stat(filename, NULL) == FR_OK);

		if(f_open(&fil, filename, FA_OPEN_ALWAYS | FA_READ | FA_WRITE) != FR_OK){
			return false;
		}

		/* Check freeSpace space */
		if(f_getfree("", &fre_clust, &pfs) != FR_OK)
			return false;

		totalSpace = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
		freeSpace = (uint32_t)(fre_clust * pfs->csize * 0.5);

		/* free space is less than 1kb */
		if(freeSpace < 4)
			return false;

		char buffer[512] = {0};

		for(uint16_t i = 0; i<((count * 32768) / 256); i++){

			for(int k = 0; k<8; k++){
			for(uint16_t j = 0; j<32; j++){
				if(i == 0 && j == 0 && k == 0){
					sprintf(buffer, " ");
				}

			if(j == 31){
				sprintf(buffer+(j*2 + k*64), "%d\n", (fill_address + (512*i))[j + k*32]);
				j++;
			}
			else
				sprintf(buffer+(j*2 + k*64), "%d ", (fill_address + (512*i))[j + k*32]);
		}
			}

		if(f_write(&fil, buffer , 512, NULL) != FR_OK)
			return false;

		if(f_lseek(&fil, (i +1)*512) != FR_OK)
			return false;
}
		if(f_close(&fil) != FR_OK)
			return false;

		/* Unmount SDCARD */
		if(f_mount(NULL, "", 0) != FR_OK)
			return false;

		return true;


}



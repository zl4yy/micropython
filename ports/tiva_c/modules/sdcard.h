/*
    This file is part of the SD Card module developped for the LM4F port of Micropython
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	26 June 2021
    
    Based on original work found here https://github.com/Javierma/SD-card-TivaC-library
*/

/*
 * sdcard.h
 *
 *  Created on: 15/09/2016
 *  Author: Javier Martínez Arrieta
 *  Version: 1.0
 *  This is part of the sdcard library, with functions that will allow you to read and (in the future) write in an SD card formatted using FAT32 (single partition).
 */

 /*  Copyright (C) 2016 Javier Martínez Arrieta
 *  This project is licensed under Creative Commons Attribution-Non Commercial-Share Alike 4.0 International (CC BY-NC-SA 4.0). According to this license you are free to:
 *  Share & copy and redistribute the material in any medium or format.
 *  Adapt & remix, transform, and build upon the material.
 *  The licensor cannot revoke these freedoms as long as you follow the license terms.
 *	Complete information about this license can be found at: https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
 *
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdint.h>
#include "modules/ssi.h"
#include "modules/time.h"
#include "modules/gpio.h"

#define BUFFERSIZE 4096		// Size of buffer to read files (eg 4KB), printing file has no size limit

/* Definitions for MMC/SDC commands */
#define CMD0     0x40    	/* GO_IDLE_STATE */
#define CMD1     0x41    	/* SEND_OP_COND */
#define CMD8     0x48    	/* SEND_IF_COND */
#define CMD9     0x49    	/* SEND_CSD */
#define CMD10    0x4A   	/* SEND_CID */
#define CMD12    0x4C    	/* STOP_TRANSMISSION */
#define CMD16    0x50    	/* SET_BLOCKLEN */
#define CMD17    0x51    	/* READ_SINGLE_BLOCK */
#define CMD18    0x52    	/* READ_MULTIPLE_BLOCK */
#define CMD23    0x57    	/* SET_BLOCK_COUNT */
#define CMD24    0x58    	/* WRITE_BLOCK */
#define CMD25    0x59    	/* WRITE_MULTIPLE_BLOCK */
#define CMD41    0x69    	/* SEND_OP_COND (ACMD) */
#define CMD55    0x77    	/* APP_CMD */
#define CMD58    0x7A    	/* READ_OCR */

enum typeOfWrite{
  COMMAND,                              // the transmission is an LCD command
  DATA                                  // the transmission is data
};


enum name_type{
	SHORT_NAME,
	LONG_NAME
};

enum get_subdirs{
	GET_SUBDIRS,
	NO_SUBDIRS
};


void Do_SD_SetPins(uint8_t ssinum);
bool Do_SD_initialise();
void Do_SD_tx_SSI();
void Do_SD_write(uint8_t message);
uint8_t Do_SD_read();
uint8_t Do_SD_send_command(uint8_t command, unsigned long argument);
void Do_SD_dummy_clock();
void Do_SD_cs_high();
void Do_SD_cs_low();
void Do_SD_tx_high();
long Do_SD_print_file(long next_cluster);
long Do_SD_print_filebin(long next_cluster);
long Do_SD_read_file(long next_cluster, uint8_t *buffer, uint16_t *offset);
uint8_t Do_SD_find_file_by_name(char *filename);
long Do_SD_get_root_dir_first_cluster(void);
long Do_SD_get_first_cluster(int pos);
long Do_SD_get_files_and_dirs(long next_cluster,enum name_type name, enum get_subdirs subdirs, bool printout);
void Do_SD_read_first_sector();
void Do_SD_read_disk_data();
unsigned int Do_SD_rcvr_datablock(uint8_t *buff, unsigned int btr);
void clean_name(void);
void rcvr_spi_m(uint8_t *dst);

#endif
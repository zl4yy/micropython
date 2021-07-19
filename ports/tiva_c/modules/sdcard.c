/*
    This file is part of the SD Card module developped for the LM4F port of Micropython
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	26 June 2021
    
    Based on original work found here https://github.com/Javierma/SD-card-TivaC-library
*/

/*
 * sdcard.c
 *
 *  Created on: 15/09/2016
 *  Author: Javier Martínez Arrieta
 *  Version: 1.0
 *  This is part of the sdcard library, with functions that will allow you to read and (in the future) write in an SD card formatted using FAT32 (single partition).
 */

 /*  Copyright (C) 2016 Javier Martínez Arrieta
 *
 *  This project is licensed under Creative Commons Attribution-Non Commercial-Share Alike 4.0 International (CC BY-NC-SA 4.0). According to this license you are free to:
 *  Share & copy and redistribute the material in any medium or format.
 *  Adapt & remix, transform, and build upon the material.
 *  The licensor cannot revoke these freedoms as long as you follow the license terms.
 *	Complete information about this license can be found at: https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
 *
 */


/* Part of this example (partially modified functions rcvr_datablock, rcvr_spi_m, disk_timerproc, Timer5A_Handler, Timer5_Init, is_ready, send_command and part of initialize_sd) accompanies the books
   Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers, Volume 3,
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2013

   Volume 3, Program 6.3, section 6.6   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file (concretely the functions mentioned)
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include "sdcard.h"
#include <stdio.h>
#include "modules/ssi.h"
#include "modules/time.h"
#include "modules/gpio.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"
#include "boards/lm4f_pin_map.h"
#include "boards/hw_memmap.h"

uint8_t Timer1,Timer2;
unsigned long lba_begin_address,number_of_sectors,lba_addr,cluster_start,file_size,fat_begin_lba,sectors_per_fat,root_dir_first_cluster;
unsigned long previous_cluster=0;
unsigned long cluster_dir=0;
long volatile file_next_cluster = 0;
uint8_t volatile sectors_per_cluster;
unsigned long volatile cluster_begin_lba;
uint8_t fd_count=0,current_count=0;
uint8_t finish=0;
int row=0,column=0,number=0;

// Global variables to store Pin settings
uint8_t _spiport=0;
uint8_t _cspin=PA3;

typedef struct
{
	uint8_t hour;
	uint8_t minute;
	unsigned int year;
	uint8_t month;
	uint8_t day;
	long size;
	long first_cluster;
}tfile_info;

typedef struct
{
	tfile_info info;
	char file_dir_name[255];
}tfile_name;

typedef struct
{
	tfile_name name;
	enum type_of_file
	{
		IS_NONE,
		IS_DIR,
		IS_FILE
	}type;
}tfile_dir;

tfile_dir file_dir[40];


/**
 * Set Pins and SSI port
 */
void Do_SD_SetPins(uint8_t ssinum) {
	_spiport = ssinum;

	switch(_spiport) {
		case 0:
		{
			_cspin = PA3;
			break;
		}
		case 1:
		{
			_cspin = PF3;
			break;
		}
		case 2:
		{
			_cspin = PB5;
			break;
		}
		case 3:
		{
			_cspin = PD1;
			break;
		}
	}
}

/**
 * Writes to the SD card
 */
void Do_SD_write(uint8_t message)
{
	uint32_t word;
	// wait until transmit FIFO not full
	while(Do_SSI_Busy(_spiport)){};
	Do_SSI_TX(_spiport, (uint32_t)message);
	while(Do_SSI_Busy(_spiport)){};
	Do_SSI_RX_Blocking(_spiport, &word);
}

/*
 * Removes null or other non-printable characters from the file or directory name string
 * Also 'translate' accented characters so they can be printed
 */
void clean_name()
{
    uint8_t temp_name[255] = "";
    uint8_t j=0, k=0;
    //Remove all non-rintable characters
    for(j=0;j<255;j++)
    {
        if(file_dir[fd_count].name.file_dir_name[j]>=0x20 && file_dir[fd_count].name.file_dir_name[j]<=0xFC)
        {
            temp_name[k] = file_dir[fd_count].name.file_dir_name[j];
            k++;
        }
    }
    for(j=0;j<255;j++)
    {
        file_dir[fd_count].name.file_dir_name[j] = temp_name[j];
    }


}

/*
 * Find the specified file by specifying its name (case sensitive)
 * Note: For now it does not find file with accented characters in its name
 */
uint8_t Do_SD_find_file_by_name(char *filename)
{   
	uint8_t index = 0;
	while(index < 40 && strcmp(filename, file_dir[index].name.file_dir_name) != 0)
	{
		index++;
	}
    return index;
}

/*
 * Reads from the SD card
 */
uint8_t Do_SD_read()
{
	uint32_t rcvdata;

	// wait until end of transmit
	while(Do_SSI_Busy(_spiport)){};
	Do_SSI_TX(_spiport,0xFF);

	// wait until end of transmit
	while(Do_SSI_Busy(_spiport)){};
	Do_SSI_RX_Blocking(_spiport, &rcvdata);
	return (uint8_t) rcvdata;
}

/*
 * Wait until sd card is ready
 */
uint8_t is_ready(){
  uint8_t response = 0;
  Timer2 = 100;    /* Wait for ready in timeout of 500ms */
  do {
    response = Do_SD_read();
	Do_SysTick_Waitms(5);
	Timer2 = Timer2 - 1;
  } while ((response != 0xFF) && Timer2);
  return response;
}


/*
 * Sends the command, preparing the packet to be sent
 */
uint8_t Do_SD_send_command(uint8_t command, unsigned long argument)
{
	/* Argument */
	uint8_t crc, response,n;
	if (is_ready() != 0xFF) return 0xFF;
	Do_SysTick_Waitms(10);

    /* Send command packet */
	Do_SD_write(command);                        /* Command */
	Do_SD_write((uint8_t)(argument >> 24));        /* Argument[31..24] */
	Do_SD_write((uint8_t)(argument >> 16));        /* Argument[23..16] */
	Do_SD_write((uint8_t)(argument >> 8));            /* Argument[15..8] */
	Do_SD_write((uint8_t)argument);                /* Argument[7..0] */
	Do_SysTick_Waitms(10);
	
	crc = 0;
	if (command == CMD0)
	{
		crc = 0x95;            /* CRC for CMD0(0) */
	}
	if (command == CMD8)
	{
		crc = 0x87;            /* CRC for CMD8(0x1AA) */
	}
	Do_SD_write(crc);

    /* Receive command response */
	if (command == CMD12) Do_SD_write(0xFF);        /* Skip a stuff byte when stop reading */
	n = 10;                                /* Wait for a valid response in timeout of 10 attempts */
	do {
		response = Do_SD_read();
	} while ((response & 0x80) && --n);
	return response;
}

/*
 * Initialises the SD card
 */
bool Do_SD_initialise()
{
	uint8_t i;
	uint8_t ocr[4];
	uint8_t sd_type;
	bool resetsuccess = false;

	Do_SysTick_Init();
	Do_GPIO_Init();
	Do_GPIO_output(_cspin);
	//Sends a 1 through CS and MOSI lines for at least 74 clock cycles
	Do_SD_cs_high();
	Do_SD_dummy_clock();
	Do_SD_cs_low();
	i=0;
	/*Checks if SD card is in IDLE mode. If so, response will be 1*/
	if(Do_SD_send_command(CMD0, 0) == 1) {
		resetsuccess = true;
	} else {	// Try at high speed if we are trying a warm init (SD Card not fully reset)
		Do_SSI_Init(_spiport,20051,true);
		if(Do_SD_send_command(CMD0, 0) == 1) {
			resetsuccess = true;
		};
	};

	if(resetsuccess) {
		Timer1 = 100; /* Initialization timeout of 1000 msec */
		if(Do_SD_send_command(CMD8, 0x1AA) == 1) {
			/* SDC Ver2+ */
			for(i=0;i<4;i++)
			{
				ocr[i]=Do_SD_read();
			}
			if(ocr[2]==0x01&&ocr[3]==0xAA)
			{
				//sends ACMD41, which is a command sequence of CMD55 and CMD41
				do
				{
					if(Do_SD_send_command(CMD55, 0) <= 1 && Do_SD_send_command(CMD41, 1UL << 30) == 0)
					{
						break; //R1 response is 0x00
					}
				}while(Timer1);
				if(Timer1 && Do_SD_send_command(CMD58, 0) == 0)
				{
					Do_SysTick_Waitms(10);
					Timer1 = Timer1 - 1;
					for(i=0;i<4;i++)
					{
						ocr[i]=Do_SD_read();
						sd_type = (ocr[0] & 0x40) ? 6 : 2;
					}
				}
			}
		}
		else {
			/*It is not SD version 2 or upper*/
			sd_type=(Do_SD_send_command(CMD55, 0)<=1 &&Do_SD_send_command(CMD41, 0) <= 1) ? 2 : 1;    /*Check if SD or MMC*/
			do {
				Do_SysTick_Waitms(10);
				Timer1 = Timer1 - 1;
				if(sd_type==2)
				{
					if(Do_SD_send_command(CMD55, 0)<=1&&Do_SD_send_command(CMD41, 0)==0) /*ACMD41*/
			        {
						break;
			        }
				}
				else
				{
					if (Do_SD_send_command(CMD1, 0) == 0) /*CMD1*/
					{
						break;
					}
			    }
			}while(Timer1);
			if(!Timer1 || Do_SD_send_command(CMD16, 512) != 0)    /*Select R/W block length if timeput not reached*/
			{
				sd_type=0;
			}
		}
		return true;
	}
	else {
	  	mp_printf(&mp_plat_print, "Failure in CMD0.\n");
		return false;
	}
}



/*
 * Makes use of the clock along with CS and MOSI to make the SD card readable using SPI
 */
void Do_SD_dummy_clock()
{
	unsigned int i;
	//In order to initialize and set SPI mode, there should be at least 74 clock cycles with MOSI and CS set to 1
	for ( i = 0; i < 2; i++);
	//CS set high
	Do_SD_cs_high();
	Do_SysTick_Waitms(10);
	//Disables SSI on TX/MOSI pin to send a 1
	Do_SD_tx_high();
	for ( i = 0; i < 100; i++)
	{
		Do_SD_write(0xFF);
	}
	Do_SD_tx_SSI();
}

/*
 * Gets the first cluster of a file or directory
 */
long Do_SD_get_first_cluster(int pos)
{
	return file_dir[pos].name.info.first_cluster;
}

/*
 * Gets the first cluster of the root directory
 */
long Do_SD_get_root_dir_first_cluster(void)
{
	return root_dir_first_cluster;
}

/*
 * Makes chip select line high
 */
void Do_SD_cs_high()
{
	Do_GPIO_up(_cspin);
}

/*
 * Makes chip select line low
 */
void Do_SD_cs_low()
{
	Do_GPIO_down(_cspin);
}

/*
 * Writes a '1' in the transmission line of the SSI that is being used
 */
void Do_SD_tx_high() {
	Do_SSI_Disable(_spiport);
	switch(_spiport) {
		case 0:
		{
			Do_GPIO_output(PA5);
			Do_GPIO_up(PA5);
			break;
		}
		case 1:
		{
			Do_GPIO_output(PF1);
			Do_GPIO_up(PF1);
			break;
		}
		case 2:
		{
			Do_GPIO_output(PB7);
			Do_GPIO_up(PB7);
			break;
		}
		case 3:
		{
			Do_GPIO_output(PD3);
			Do_GPIO_up(PD3);
			break;
		}
	}
	Do_SSI_Enable(_spiport);
}

/*
 * Configure again the transmission line of the SSI that is being used
 */
void Do_SD_tx_SSI() {
	Do_SSI_Disable(_spiport);
	switch(_spiport) {
		case 0:
		{
            ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
            ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, 0x34); // Pins 2 4 5 of Port A used for SSI, CS PA3 is free
			break;
		}
		case 1:
		{
            ROM_GPIOPinConfigure(GPIO_PF1_SSI1TX);
            ROM_GPIOPinTypeSSI(GPIO_PORTF_BASE, 0x07); // Pins 0 1 2 of Port F used for SSI, CS PF3 is free
			break;
		}
		case 2:
		{
            ROM_GPIOPinConfigure(GPIO_PB7_SSI2TX);
            ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, 0xD0);  // Pins 4 6 7 of port B used for SSI, CS PB5 is free
			break;
		}
		case 3:
		{
            ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);
            ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, 0x0d);   // Pins 0 2 3 of Port D used for SSI, CS PD1 is free
			break;
		}
	}
	Do_SSI_Enable(_spiport);
}

void read_csd()
{
	uint8_t csd[16];
	Do_SD_send_command(CMD9,0);
	Do_SD_rcvr_datablock(csd,16);
}

/*
 * Verify if file system is FAT32
 */
void Do_SD_read_first_sector()
{
	uint8_t buffer[512];
	if (Do_SD_send_command(CMD17, 0x00000000) == 0)
	{
		Do_SD_rcvr_datablock(buffer, 512);
	}
	if((buffer[450] == 0x0B || buffer[450] == 0x0C) && buffer[510] == 0x55 && buffer[511] == 0xAA)
	{
	  	mp_printf(&mp_plat_print, "FS is FAT32.\n");
	}
	else{
	  	mp_printf(&mp_plat_print, "Error FAT32.\n");
	}
	lba_begin_address=(unsigned long)buffer[454]+(((unsigned long)buffer[455])<<8)+(((unsigned long)buffer[456])<<16)+(((unsigned long)buffer[457])<<24);
	number_of_sectors=(unsigned long)buffer[458]+(((unsigned long)buffer[459])<<8)+(((unsigned long)buffer[460])<<16)+(((unsigned long)buffer[461])<<24);
}

/*
 * Reads the necessary data so as to be able to access the files and directories
 */
void Do_SD_read_disk_data()
{
	uint8_t buffer[512];
	if (Do_SD_send_command(CMD17, lba_begin_address) == 0)
	{
		Do_SD_rcvr_datablock(buffer, 512);
	}
	fat_begin_lba = lba_begin_address + (unsigned long)buffer[14] + (((unsigned long)buffer[15])<<8); //Partition_LBA_BEGIN + Number of reserved sectors
	sectors_per_fat=((unsigned long)buffer[36]+(((unsigned long)buffer[37])<<8)+(((unsigned long)buffer[38])<<16)+(((unsigned long)buffer[39])<<24));
	cluster_begin_lba = fat_begin_lba + ((unsigned long)buffer[16] * ((unsigned long)buffer[36]+(((unsigned long)buffer[37])<<8)+(((unsigned long)buffer[38])<<16)+(((unsigned long)buffer[39])<<24)));//Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	sectors_per_cluster = (uint8_t) buffer[13];//BPB_SecPerClus;
	root_dir_first_cluster = (unsigned long)buffer[44]+(((unsigned long)buffer[45])<<8)+(((unsigned long)buffer[46])<<16)+(((unsigned long)buffer[47])<<24);//BPB_RootClus;
	lba_addr = cluster_begin_lba + ((root_dir_first_cluster/*cluster_number*/ - 2) * (unsigned long)sectors_per_cluster);
}

/*
 * List directories and files using the long name (if it has) or the short name, listing subdirectories as well if asked by the user
 */
long Do_SD_get_files_and_dirs(long next_cluster,enum name_type name, enum get_subdirs subdirs, bool printout)
{
	uint8_t buffer[512];
    uint8_t filename[255] = "";
	int position=0,filename_position=0;
	int n=0;
	unsigned long count=10,sectors_to_be_read=sectors_per_cluster;//Calculate this
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	if(cluster_dir == next_cluster)
	{
		cluster_dir=0;
	}
	if(Do_SD_send_command(CMD18,address)==0)
	{
		do
		{
			Do_SD_rcvr_datablock(buffer, 512);
			do
			{
				if(position<512 && filename_position<255)
				{//Long filename text - 11th byte is 0x0F
					if(position%32==0)
					{//Check if file has a long filename text, normal record with short filename, unused or end of a directory
						if(buffer[position]==0x00 || buffer[position]==0x2E)
						{//End of directory
							position=position+32;
						}
						else
						{
							if(buffer[position]==0xE5)
							{//Deleted file or directory that is maintained until overriden
								position=position+32;
							}
							else
							{
								if(name==LONG_NAME)//Review this
								{//Review this as there are files which long name are not read, probably because they only have one sequence for the name
									//short keep_counting=1,do_not_continue=0,is_dir=0, keep_reading = 1;
									short keep_reading = 1;
									uint8_t seq_num = 0, filename_read_finished = 0;
									if(buffer[position+11]==0x0F)
									{
									    do
                                        {
                                            //Check if it is last record group of the filename
                                            if(buffer[position+11]!=0x0F)
                                            {
                                                keep_reading = 0;
                                                if(position == 512)
                                                {
                                                    filename_read_finished = 0;
                                                }
                                                else
                                                {
                                                    filename_read_finished = 1;
                                                }
                                            }
                                            //Get the sequence number
                                            seq_num=buffer[position]&0x1F;

                                            uint8_t k=0,l=0;
                                            while(k<32 && keep_reading == 1)
                                            {
                                                if((k>0 && k<11) || (k>13 && k<26) || (k>27 && k<32))
                                                {
                                                    filename[(32*(seq_num-1))+l] = buffer[position];
                                                    l++;
                                                }
                                                k++;
                                                position++;
                                            }
                                        }while(keep_reading == 1);
									}
									else
									{
									    //Filename exactly has a length of 8 bytes and either the base name or the extension have all its characters in capital letters, so we need to check
									    if((buffer[position+12]&0x18)>0)
                                        {
									        //Base
									        if((buffer[position+12]&0x10)>0 && (buffer[position+12]&0x08)>0)
									        {
									            uint8_t k=0;
									            while(k<11)
									            {
									                if(k < 8)
									                {
									                    filename[k] = buffer[position] + 32;
									                }
									                else
									                {
									                    if(k >= 8)
									                    {
									                        filename[k+1] = buffer[position] + 32;
									                    }
									                }
									                k++;
									                position++;
									            }
									            filename[8] = '.';
									        }
									        else
									        {
									            //Extension is in lowercase, basename is in uppercase
									            if((buffer[position+12]&0x10)>0)
									            {
									                uint8_t k=0;
									                while(k<11)
									                {
									                    if(k<8)
									                    {
									                        filename[k] = buffer[position];
									                    }
									                    else
									                    {
									                        filename[k+1] = buffer[position] + 32;
									                    }
									                    k++;
									                    position++;
									                }
									                filename[8] = '.';
									            }
									            else
									            {
									                //Extension in uppercase, basename in lowercase
									                if((buffer[position+12]&0x08)>0)
									                {
									                    uint8_t k=0;
									                    while(k<11)
									                    {
									                        if(k<8)
									                        {
									                            filename[k] = buffer[position] + 32;
									                        }
									                        else
									                        {
									                            filename[k+1] = buffer[position];
									                        }
									                        k++;
									                        position++;
									                    }
									                    filename[8] = '.';
									                }
									            }
									        }
                                            filename_read_finished = 1;
                                            position = position - 11;
                                        }
									    else
									    {
									        //Both basename and extension are uppercase
									        if((buffer[position+12]&0x18)==0)
									        {
									            uint8_t k=0;
									            while(k<11)
									            {
									                if(k < 8)
									                {
									                    filename[k] = buffer[position];
									                }
									                else
									                {
									                    if(k >= 8)
									                    {
									                        filename[k+1] = buffer[position];
									                    }
									                }
									                k++;
									                position++;
									            }
									            filename[8] = '.';
	                                            filename_read_finished = 1;
	                                            position = position - 11;
									        }
									    }
									}
									//Check if filename is a System file and the filename reading was completed
									if((buffer[position+11]&0x0E)==0x00 && filename_read_finished == 1)
									{
									    if((buffer[position+11]&0x30)==0x10)
                                        {
                                            file_dir[fd_count].type=IS_DIR;
                                        }
                                        else
                                        {
                                            if((buffer[position+11]&0x30)==0x20)
                                            {
                                                file_dir[fd_count].type=IS_FILE;
                                            }
                                        }
									    uint8_t k = 0;
									    while(k<255)
									    {
									        file_dir[fd_count].name.file_dir_name[k]=filename[k];
									        k++;
									    }
									    //Reset filename
									    k = 0;
									    while(k<255)
									    {
									        filename[k]=0x00;
									        k++;
									    }
                                        int time=(((int)(buffer[position+23]))<<8) + ((int)buffer[position+22]);
                                        int date=(((int)(buffer[position+25]))<<8) + ((int)buffer[position+24]);
                                        file_dir[fd_count].name.info.minute=(time&0x07E0)>>5;
                                        file_dir[fd_count].name.info.hour=(time&0xF800)>>11;
                                        file_dir[fd_count].name.info.month=((date&0x01E0)>>5);
                                        file_dir[fd_count].name.info.year=((date&0xFE00)>>9)+1980;
                                        file_dir[fd_count].name.info.day=date&0x001F;
                                        file_dir[fd_count].name.info.size=(long)((buffer[position+31])<<24)+(long)((buffer[position+30])<<16)+(long)((buffer[position+29])<<8)+(long)(buffer[position+28]);
                                        file_dir[fd_count].name.info.first_cluster=(long)((buffer[position+21])<<24)+(long)((buffer[position+20])<<16)+(long)((buffer[position+27])<<8)+(long)(buffer[position+26]);
                                        position = position + 32;
									}
									else
									{
									    if(filename_read_finished == 1)
									    {
                                            //Reset filename
                                            uint8_t k = 0;
                                            while(k<255)
                                            {
                                                filename[k]=0x00;
                                                k++;
                                            }
                                            position++;
									    }
									}
								}
								else
								{//Normal record with short filename
									//Check it is not a system file or directory and names to be read are short names
								    if((buffer[position+11]&0x0E)==0x00 && name==SHORT_NAME)
								    {
								        if((buffer[position+11]&0x30)==0x10)
								        {
								            file_dir[fd_count].type=IS_DIR;
								        }
								        else
								        {
								            if((buffer[position+11]&0x30)==0x10)
								            {
								                file_dir[fd_count].type=IS_FILE;
								            }
								        }
                                        for(n=0;n<11;n++)
                                        {
                                            file_dir[fd_count].name.file_dir_name[n]=buffer[position];
                                            position++;
                                        }
                                        int time=(((int)(buffer[position-11+23]))<<8) + ((int)buffer[position-11+22]);
                                        int date=(((int)(buffer[position-11+25]))<<8) + ((int)buffer[position-11+24]);
                                        file_dir[fd_count].name.info.minute=(time&0x07E0)>>5;
                                        file_dir[fd_count].name.info.hour=(time&0xF800)>>11;
                                        file_dir[fd_count].name.info.month=((date&0x01E0)>>5);
                                        file_dir[fd_count].name.info.year=((date&0xFE00)>>9)+1980;
                                        file_dir[fd_count].name.info.day=date&0x001F;
                                        file_dir[fd_count].name.info.size=(long)((buffer[position-11+31])<<24)+(long)((buffer[position-11+30])<<16)+(long)((buffer[position-11+29])<<8)+(long)(buffer[position-11+28]);
                                        file_dir[fd_count].name.info.first_cluster=(long)((buffer[position-11+21])<<24)+(long)((buffer[position-11+20])<<16)+(long)((buffer[position-11+27])<<8)+(long)(buffer[position-11+26]);
								    }
								    else
								    {
								        if(position==512)
								        {
								            //position=0;
								        }
								        else
								        {
								            position++;
								        }
								    }
								}
							}
						}
						clean_name();
						if(file_dir[fd_count].name.file_dir_name[0]!=0xFF && file_dir[fd_count].name.file_dir_name[0]!=0x00)
						{
							if(file_dir[fd_count].type==IS_DIR)
							{
							  	if (printout) mp_printf(&mp_plat_print, "%d. (DIR)\t", number);
							}
							else
							{
							  	if (printout) mp_printf(&mp_plat_print, "%d. (FILE)\t", number);
							}
							uint8_t i;
							for(i=0;i<255;i++)
							{
								if(file_dir[fd_count].name.file_dir_name[i]!=0x00)
								{
								  	if (printout) mp_printf(&mp_plat_print, "%c",file_dir[fd_count].name.file_dir_name[i]);
								}
							}
							if (printout) mp_printf(&mp_plat_print, "\t\t");
							if (printout) mp_printf(&mp_plat_print, "%d/%d/%d	%d:%d\n",file_dir[fd_count].name.info.day,file_dir[fd_count].name.info.month,file_dir[fd_count].name.info.year,file_dir[fd_count].name.info.hour,file_dir[fd_count].name.info.minute);
							fd_count++;
							number++;
						}
					}
					else
					{
						if(position==512)
						{
							//position=0;
						}
						else
						{
							position++;
						}
					}
				}
				else
				{
					if(position==512)
					{
						count--;
					}
					else
					{
						position++;
					}
				}
			} while (position<512);
			position=0;
			sectors_to_be_read--;
		}while(sectors_to_be_read>0);
	}
	Do_SD_send_command(CMD12,0);
	sectors_to_be_read=(next_cluster*4)/512;
	long sector=0;
	if(Do_SD_send_command(CMD18,fat_begin_lba)==0)
	{
		do
		{
			sector++;
			Do_SD_rcvr_datablock(buffer, 512);
		}while(sectors_to_be_read>0);
		sector--;
	}
	Do_SD_send_command(CMD12,0);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);
	if((next_cluster==0x0FFFFFFF || next_cluster==0xFFFFFFFF) && current_count<40 && subdirs==GET_SUBDIRS)
	{
		while(current_count<40&&file_dir[current_count].type!=IS_DIR)
		{
			current_count++;
		}
		if(current_count<40 && file_dir[current_count].type==IS_DIR)
		{
			if (printout) mp_printf(&mp_plat_print, "Content of ");
			uint8_t i;
			for(i=0;i<255;i++)
			{
				if(file_dir[current_count].name.file_dir_name[i]!=0x00)
				{
					if (printout) mp_printf(&mp_plat_print, "%c",file_dir[current_count].name.file_dir_name[i]);
				}
			}
			next_cluster=file_dir[current_count].name.info.first_cluster;
			current_count++;
			if (printout) mp_printf(&mp_plat_print, "\n\t");
		}
	}
	if(current_count==40)
	{
		number=0;
		next_cluster=0x0FFFFFFF;
	}
	return next_cluster;
}

/*
 *Print file content.
 *Please note that this method should be modified if the file to be opened is not a txt file (concretely the content inside the for loop)
 */
long Do_SD_print_file(long next_cluster)
{

	uint8_t buffer[512];
	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);

	sectors_to_be_read=(next_cluster*4)/512;
	if(Do_SD_send_command(CMD18,fat_begin_lba)==0) {
		do
		{
			sector++;
			Do_SD_rcvr_datablock(buffer, 512);
		}while(sectors_to_be_read>=sector); // Bugfix: replaced 0 by sector
		sector--;
	}
	Do_SD_send_command(CMD12,0);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);

	if(Do_SD_send_command(CMD18,address)==0) {
		do
		{
			Do_SD_rcvr_datablock(buffer, 512);
			int c=0;
			for(c=0;c<512;c++)
			{
				if(buffer[c]!=0x00)
				{
					mp_printf(&mp_plat_print, "%c", buffer[c]);
				}
				else
				{
					c=512;
					finish=1;
				}
			}
			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1);
	}
	Do_SD_send_command(CMD12,0);

	if(next_cluster==0x0FFFFFFF || next_cluster==0xFFFFFFFF) // Bugfix: second test was wrong with 0x0FFFFFFF
	{
		finish=0;
	}
	return next_cluster;
}

/*
 *Print file content.
 *Please note that this method should be modified if the file to be opened is not a txt file (concretely the content inside the for loop)
 */
long Do_SD_print_filebin(long next_cluster)
{

	uint8_t buffer[512];
	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);

	sectors_to_be_read=(next_cluster*4)/512;
	if(Do_SD_send_command(CMD18,fat_begin_lba)==0) {
		do
		{
			sector++;
			Do_SD_rcvr_datablock(buffer, 512);
		}while(sectors_to_be_read>=sector); // Bugfix: replaced 0 by sector
		sector--;
	}
	Do_SD_send_command(CMD12,0);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);

	if(Do_SD_send_command(CMD18,address)==0) {
		do
		{
			Do_SD_rcvr_datablock(buffer, 512);
			int c=0;
			for(c=0;c<512;c++)
			{
				mp_printf(&mp_plat_print, "%x", buffer[c]);
			}
			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1);
	}
	Do_SD_send_command(CMD12,0);

	if(next_cluster==0x0FFFFFFF || next_cluster==0xFFFFFFFF) // Bugfix: second test was wrong with 0x0FFFFFFF
	{
		finish=0;
	}
	return next_cluster;
}

/*
 *Reads file content and returns it in struct
 *Note: order or loops inverted compared to Do_SD_open_file to save one buffer
 */

long Do_SD_read_file(long next_cluster, uint8_t *buffer, uint16_t *offset) {

	// We need to use the offset so we keep same position in buffer even if jumping to next cluster

	long sector=0;
	long sectors_to_be_read=sectors_per_cluster;
	long address=cluster_begin_lba + ((next_cluster - 2) * (unsigned long)sectors_per_cluster);
	
	sectors_to_be_read=(next_cluster*4)/512;
	if(Do_SD_send_command(CMD18,fat_begin_lba)==0) {
		do
		{
			sector++;
			Do_SD_rcvr_datablock(buffer, 512);
		}while(sectors_to_be_read>=sector); // Bugfix: replaced 0 by sector
		sector--;
	}
	Do_SD_send_command(CMD12,0);
	next_cluster=(((long)(buffer[((next_cluster*4)-(sector*512))+3]))<<24)+(((long)(buffer[((next_cluster*4)-(sector*512))+2]))<<16)+(((long)(buffer[((next_cluster*4)-(sector*512))+1]))<<8)+(long)(buffer[((next_cluster*4)-(sector*512))]);

	if(Do_SD_send_command(CMD18,address)==0) {
		do
		{
			Do_SD_rcvr_datablock(&buffer[*offset], 512);
			*offset = *offset + 512;

			sectors_to_be_read--;
		}while(sectors_to_be_read>0 && finish!=1 && *offset<BUFFERSIZE);
	}
	Do_SD_send_command(CMD12,0);

	if(next_cluster==0x0FFFFFFF || next_cluster==0xFFFFFFFF) // Bugfix: second test was wrong with 0x0FFFFFFF
	{
		finish=0;
	}
	return next_cluster;
}

/*
 * Receives a block from a read of an SD card
 */
unsigned int Do_SD_rcvr_datablock (
    uint8_t *buff,         /* Data buffer to store received data */
    unsigned int btr) {          /* Byte count (must be even number) */

	uint8_t token;
	Timer1 = 100;
	do {                            /* Wait for data packet in timeout of 100ms */
		token = Do_SD_read();
		Do_SysTick_Waitms(5);
		Timer1 = Timer1 - 1;
	} while ((token == 0xFF) && Timer1);

	if(token != 0xFE) return 0;    /* If not valid data token, retutn with error */

	do {                            /* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 2);

	Do_SD_write(0xFF);                        /* Discard CRC */
	Do_SD_write(0xFF);

	return 1;                    /* Return with success */
}

void rcvr_spi_m(uint8_t *dst){
  *dst = Do_SD_read();
}

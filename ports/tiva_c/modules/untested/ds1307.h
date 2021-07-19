/*

	Basic DS1307 RTC Clock support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	30 June 2021
	
	Software provided under MIT License
  Based on DS1307new library by Peter Schmelzer and Oliver Kraus

*/

#ifndef DS1307_H_
#define DS1307_H_

#include <stdbool.h>
#include "py/objstr.h"
#include "modules/i2c.h"


/*
    Functions for use in other libraries
*/

// Initialise the DS1307 and I2C
void Do_RTC_Init(uint8_t port);

// Aquire time from the RTC chip in BCD format and convert it to DEC
bool Do_RTC_readtime(void);

#endif
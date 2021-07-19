/*

	Basic BMP085 Pressure sensor support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	4 July 2021
	
	Software provided under MIT License

*/

#ifndef BMP085_H_
#define BMP085_H_

#include <stdbool.h>
//#include "py/objstr.h"
#include "modules/i2c.h"


/*
    Functions for use in other libraries
*/

// Initialise the BMP085 and I2C
bool Do_BMP085_Init(uint8_t port);

// Acquire data from the BMP085
bool Do_BMP085_readdata(void);

#endif
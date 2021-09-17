/*

	Headers
	Basic XPT2046 Touch Screen support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	3 September 2021
	
	Software provided under MIT License

	Based on the following library:
	https://github.com/ImpulseAdventure/Arduino-TFT-Library-ILI9486
	
*/

#ifndef XPT2046_H_
#define XPT2046_H_

#include <stdbool.h>


void Do_XPT_endWrite();

void Do_XPT_Init(uint8_t spiport, uint8_t pinChipSelect);

int16_t Do_XPT_transfer(uint16_t command);

int16_t Do_XPT_besttwoavg( int16_t x , int16_t y , int16_t z );

void Do_XPT_update();

#endif
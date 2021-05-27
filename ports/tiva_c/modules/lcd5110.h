/*

	Headers
	Basic LCD 5110 (PCD8544) for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	26 May 2021
	
	Software provided under MIT License

  Project LCD 5110
  Based on work by Rei VILO on 28/05/12 (http://embeddedcomputing.weebly.com)

*/
#ifndef LCD5110_H_
#define LCD5110_H_

#include <stdbool.h>

#define COMMANDLCD  0     // D/C bit
#define DATALCD     1     // D/C bit

#define SML     0     // Small font
#define BIG     1     // Large font


void Do_LCD_5110(uint8_t pinChipSelect, uint8_t pinSerialClock, uint8_t pinSerialData, uint8_t pinDataCommand, uint8_t pinReset, uint8_t pinBacklight, uint8_t pinPushButton);


void Do_LCD_write(uint8_t dataCommand, uint8_t c);

void Do_LCD_setXY(uint8_t x, uint8_t y);

void Do_LCD_Init();

void Do_LCD_clear(bool bl);

void Do_LCD_setBacklight(bool b);

void Do_LCD_setFont(uint8_t font);

void Do_LCD_plot(uint8_t x, uint8_t y);

void Do_LCD_text(uint8_t x, uint8_t y, char* s);

bool Do_LCD_getButton();

#endif
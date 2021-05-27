/*

	Headers
	Basic GPIO control for Texas Instruments LM4F Microcontrollers
	Limited to pins that are exposed on the Stellaris LaunchPad board
	Includes LEDs and Switches (see boards/pins_def.h)
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	27 May 2021
	
	Software provided under MIT License

*/

#ifndef GPIO_H_
#define GPIO_H_

#define MSBFIRST 0
#define LSBFIRST 1

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "boards/pins_def.h"

/*

    Internal C functions (For use in other C modules)   

*/

// Enable the GPIO port that is used for the on-board LEDs and Switches.
void Do_GPIO_Init();

// Enable the GPIO pins for the LEDs.  Set the direction as output, and
// enable the GPIO pin for digital function.
void Do_GPIO_output(int pin);

// Enable the GPIO pins for the Switches.  Set the direction as input, and
// enable the GPIO pin for digital function.
void Do_GPIO_input(int pin);

// Set pin value to High
void Do_GPIO_up(int pin);

// Set pin value to Low
void Do_GPIO_down(int pin);

// Wrapper to mimic Arduino/Energia digitalWrite 
void Do_GPIO_write(int pin, bool value);

// Read value from input pin
int Do_GPIO_read(int pin);

// Shift out function from Arduino source code
void Do_GPIO_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

#endif
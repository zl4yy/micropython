/*

    Headers
	Basic Time and delay for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	12 May 2021
	
	Software provided under MIT License

*/

#ifndef TIME_H_
#define TIME_H_

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"


/*

    Internal C functions (For use in other C modules)   

*/

// Initialise PLL and registers to use SysTick
void Do_SysTick_Init();

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void Do_SysTick_Wait(uint32_t delay);

// The delay parameter is in milliseconds
void Do_SysTick_Waitms(uint32_t delay);

// The delay parameter is in micro-seconds
void Do_SysTick_Waitus(uint32_t delay);

// Return the current tick value in ms
uint32_t Do_SysTick_Ticksms();

// Return the diff in ticks in ms
int32_t Do_SysTick_TicksDiff(uint32_t tick_a, uint32_t tick_b);

#endif
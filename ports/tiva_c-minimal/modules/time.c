/*

	Basic Time and delay for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	12 May 2021
	
	Software provided under MIT License

*/
#include <stdbool.h>
#include <math.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"

bool SysTick_InitDone = false;

/*

    Internal C functions (For use in other C modules)   

*/

// Initialise PLL and registers to use SysTick
void Do_SysTick_Init() {
    //
    // Initialise the Registers for PLL.
    //
    // 0) Use RCC2
    SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
    // 1) bypass PLL while initializing
    SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
    // 2) select the crystal value and oscillator source
    SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                    + 0x00000540;   // 10101, configure for 16 MHz crystal
    SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
    // 3) activate PLL by clearing PWRDN
    SYSCTL_RCC2_R &= ~0x00002000;
    // 4) set the desired system divider
    SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                    + (4<<22);      // configure for 80 MHz clock
    // 5) wait for the PLL to lock by polling PLLLRIS
    while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
    // 6) enable use of PLL by clearing BYPASS
    SYSCTL_RCC2_R &= ~0x00000800;
    //
    // Initialise Systick.
    //
    // 1) Disable SysTick during initialization.
    NVIC_ST_CTRL_R &= ~(NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN | NVIC_ST_CTRL_ENABLE);    
    // 2) Set the RELOAD register to establish a modulo RELOAD + 1 decrement counter
    NVIC_ST_RELOAD_R = 0x00FFFFFF;    
    // 3) Clear the accumulator
    NVIC_ST_CURRENT_R = 0;
    // 4) Set clock source to core clock and enable 
    NVIC_ST_CTRL_R = NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_ENABLE;
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void Do_SysTick_Wait(uint32_t delay) {
    if (SysTick_InitDone == false) {
	    mp_printf(&mp_plat_print, "Time not initialised.\n");
    } else{
        NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
        NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
        while ((NVIC_ST_CTRL_R&0x00010000)==0) {} // wait for count flag
    }
}

// The delay parameter is in milliseconds
void Do_SysTick_Waitms(uint32_t delay) {
	uint32_t i;
	
	for(i=0; i<delay; i++){
    	Do_SysTick_Wait(80000);  // wait 1ms
		// 80000*12.5ns equals 1ms
	}
}

// The delay parameter is in micro-seconds
void Do_SysTick_Waitus(uint32_t delay) {
	uint32_t i;
	
	for(i=0; i<delay; i++){
    	Do_SysTick_Wait(80);  // wait 1ms
		// 80*12.5ns equals 1ms
	}
}

// Return the current tick
void Do_SysTick_Ticksms() {
	
}


/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t time_info(void) {
    mp_printf(&mp_plat_print, "Functions are time.init() time.sleep_ms(<int>) time.sleep_us(<int>) time.ticks_ms() time.ticks_diff(<ini>,<int>)\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_info_obj, time_info);

// Initialise registers for SysTick
STATIC mp_obj_t time_init(void) {

    Do_SysTick_Init();
	SysTick_InitDone = true;
	mp_printf(&mp_plat_print, "Time initialised.\n");

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_init_obj, time_init);

// Delay per milli seconds
STATIC mp_obj_t time_sleep_ms(mp_obj_t delay_obj) {
    uint32_t delay = mp_obj_get_int(delay_obj);
	
    if (SysTick_InitDone == false) {
	    mp_printf(&mp_plat_print, "Time not initialised.\n");
    } else {
        Do_SysTick_Waitms(delay);
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_ms_obj, time_sleep_ms);

// Delay per micro seconds
STATIC mp_obj_t time_sleep_us(mp_obj_t delay_obj) {
    uint32_t delay = mp_obj_get_int(delay_obj);

    if (SysTick_InitDone == false) {
	    mp_printf(&mp_plat_print, "Time not initialised.\n");
    } else {
        Do_SysTick_Waitus(delay);
    }
	
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_us_obj, time_sleep_us);

// Return current Systick (WIP).
STATIC mp_obj_t time_ticks_ms(void) {
    uint32_t value = 0;
    if (SysTick_InitDone == false) {
	    mp_printf(&mp_plat_print, "Time not initialised.\n");
    } else {
        value = ROM_SysTickValueGet();
// This part of the code does not work
//        value = (uint32_t)floor(value/80000);
    }

    return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_ticks_ms_obj, time_ticks_ms);

// Return difference between ticks.
STATIC mp_obj_t time_ticks_diff(mp_obj_t tick_a_obj, mp_obj_t tick_b_obj) {
    uint32_t tick_a = mp_obj_get_int(tick_a_obj);
    uint32_t tick_b = mp_obj_get_int(tick_b_obj);
    uint32_t period = ROM_SysTickPeriodGet();
    int32_t value = ((int32_t)(((tick_b - tick_a + period / 2) & (period - 1)) - period / 2));

    return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(time_ticks_diff_obj, time_ticks_diff);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t time_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_time) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&time_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep_ms), MP_ROM_PTR(&time_sleep_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep_us), MP_ROM_PTR(&time_sleep_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_ms), MP_ROM_PTR(&time_ticks_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_diff), MP_ROM_PTR(&time_ticks_diff_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&time_init_obj) },
};
STATIC MP_DEFINE_CONST_DICT(time_module_globals, time_module_globals_table);

const mp_obj_module_t time_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&time_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_time, time_module, MICROPY_MODULE_TIME);
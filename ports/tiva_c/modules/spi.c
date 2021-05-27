/*

	Basic SPI control for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	12 May 2021
	
	Software provided under MIT License

*/
#include <stdbool.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"

bool SPI_InitDone = false;

/*

    Internal C functions (For use in other C modules)   

*/

// Initialise registers to SPI
void Do_SPI_Init() {
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



/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t spi_info(void) {
    mp_printf(&mp_plat_print, "Functions are spi.init()\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(spi_info_obj, spi_info);

// Initialise registers for SysTick
STATIC mp_obj_t spi_init(void) {

    Do_SPI_Init();
	SPI_InitDone = true;
	mp_printf(&mp_plat_print, "SPI initialised.\n");

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(spi_init_obj, spi_init);

// Return current Systick in ms.
STATIC mp_obj_t spi_ticks_ms(void) {
    uint32_t value = 0;
    if (SysTick_InitDone == false) {
	    mp_printf(&mp_plat_print, "Time not initialised.\n");
    } else {
        value = Do_SysTick_Ticksms();
    }

    return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(spi_ticks_ms_obj, spi_ticks_ms);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t spi_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_spi) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&spi_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_ticks_ms), MP_ROM_PTR(&spi_ticks_ms_obj) },
};
STATIC MP_DEFINE_CONST_DICT(spi_module_globals, spi_module_globals_table);

const mp_obj_module_t spi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&spi_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_spi, spi_module, MICROPY_MODULE_SPI);
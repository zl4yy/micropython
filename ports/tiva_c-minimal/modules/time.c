/*

	Basic Time and delay for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	12 May 2021
	
	Software provided under MIT License

*/
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"

STATIC mp_obj_t time_info(void) {
	// Display basic help
    mp_printf(&mp_plat_print, "Functions are time.init() time.sleep_ms(<int>) time.sleep_us(<int>)\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_info_obj, time_info);


STATIC mp_obj_t time_init(void) {
    //
    // Enable the Systick.
    //
	NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
	NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(time_init_obj, time_init);

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay) {
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while ((NVIC_ST_CTRL_R&0x00010000)==0) {} // wait for count flag
}

// Delay per milli seconds
STATIC mp_obj_t time_sleep_ms(mp_obj_t delay_obj) {
    unsigned long delay = mp_obj_get_int(delay_obj);
	unsigned long i;
	
	for(i=0; i<delay; i++){
    	SysTick_Wait(80000);  // wait 1ms
		// 80000*12.5ns equals 1ms
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_ms_obj, time_sleep_ms);

// Delay per micro seconds
STATIC mp_obj_t time_sleep_us(mp_obj_t delay_obj) {
    unsigned long delay = mp_obj_get_int(delay_obj);
	unsigned long i;
	
	for (i=0; i<delay; i++) {
    	SysTick_Wait(80);  // wait 1us
		// 80*12.5ns equals 1us
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(time_sleep_us_obj, time_sleep_us);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t time_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_time) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&time_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep_ms), MP_ROM_PTR(&time_sleep_ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_sleep_us), MP_ROM_PTR(&time_sleep_us_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&time_init_obj) },
};
STATIC MP_DEFINE_CONST_DICT(time_module_globals, time_module_globals_table);

const mp_obj_module_t time_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&time_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_time, time_module, MICROPY_MODULE_TIME);
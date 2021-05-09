/*

	Basic GPIO control for Texas Instruments LM4F Microcontrollers
	Limited to LEDs and Switches on the Stellaris LaunchPad board
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	10 May 2021
	
	Software provided under MIT License

*/
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"

#define PF0 0
#define PF1 1
#define PF2 2
#define PF3 3
#define PF4 4

STATIC mp_obj_t gpio_info(void) {
	// Display basic help
    mp_printf(&mp_plat_print, "Basic GPIO Functions to control LEDs. Functions are:\n gpio.init() gpio.output(<pin>) gpio.input(<pin>) gpio.up(<pin>) gpio.down(<pin>) gpio.read(<pin>)\n <pin> can be 0 1 2 3 4 or gpio.pf0/1/2/3/4 or gpio.red/blue/green/sw1/sw2\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpio_info_obj, gpio_info);

STATIC mp_obj_t gpio_output(mp_obj_t pin_obj) {
    // Enable the GPIO pins for the LEDs.  Set the direction as output, and
    // enable the GPIO pin for digital function.
    int pin = mp_obj_get_int(pin_obj);

	switch (pin) {
	case 1:
		GPIO_PORTF_DIR_R |= 0x02;
		GPIO_PORTF_DEN_R |= 0x02;
	    mp_printf(&mp_plat_print, "Pin PF1 set as output.\n");
		break;
	case 2:
		GPIO_PORTF_DIR_R |= 0x04;
		GPIO_PORTF_DEN_R |= 0x04;
	    mp_printf(&mp_plat_print, "Pin PF2 set as output.\n");
		break;
	case 3:
		GPIO_PORTF_DIR_R |= 0x08;
		GPIO_PORTF_DEN_R |= 0x08;
	    mp_printf(&mp_plat_print, "Pin PF3 set as output.\n");
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable for this function.\n");
		break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_output_obj, gpio_output);

STATIC mp_obj_t gpio_input(mp_obj_t pin_obj) {
    // Enable the GPIO pins for the Switches.  Set the direction as input, and
    // enable the GPIO pin for digital function.
    int pin = mp_obj_get_int(pin_obj);

	switch (pin) {
	case 0:
		// SW2 is connected to PORTF0, which is an NMI pin.
		// In order to use this pin for any function other than NMI, the pin needs be unlocked first.
		GPIO_PORTF_LOCK_R = 0x4C4F434B;    // unlock commit register
	    GPIO_PORTF_CR_R |= 0x01;           // make PORTF0 configurable
		GPIO_PORTF_DIR_R &= ~0x01;
		GPIO_PORTF_DEN_R |= 0x01;
		GPIO_PORTF_PUR_R |= 0x01;         // enable pull up for pin 0
	    mp_printf(&mp_plat_print, "Pin PF0 set as input and Pull up enabled.\n");
		break;
	case 4:
		GPIO_PORTF_DIR_R &= ~0x10;
		GPIO_PORTF_DEN_R |= 0x10;
		GPIO_PORTF_PUR_R |= 0x10;         // enable pull up for pin 4
	    mp_printf(&mp_plat_print, "Pin PF4 set as input and Pull up enabled.\n");
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable for this function.\n");
		break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_input_obj, gpio_input);


STATIC mp_obj_t gpio_up(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);

	switch (pin) {
	case 1:
		GPIO_PORTF_DATA_R |= 0x02;	// Set pin 1 (bit 1) to 1
		break;
	case 2:
		GPIO_PORTF_DATA_R |= 0x04;	// Set pin 2 (bit 2) to 1
		break;
	case 3:
		GPIO_PORTF_DATA_R |= 0x08;	// Set pin 3 (bit 3) to 1
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable for this function.\n");
		break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_up_obj, gpio_up);

STATIC mp_obj_t gpio_down(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);

	switch (pin) {
	case 1:
		GPIO_PORTF_DATA_R &= ~0x02; // Set pin 1 (bit 1) to 0
		break;
	case 2:
		GPIO_PORTF_DATA_R &= ~0x04; // Set pin 2 (bit 2) to 0
		break;
	case 3:
		GPIO_PORTF_DATA_R &= ~0x08;	// Set pin 3 (bit 3) to 10
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable for this function.\n");
		break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_down_obj, gpio_down);

STATIC mp_obj_t gpio_read(mp_obj_t pin_obj) {
	// Reading a value from the GPIO port
    int pin = mp_obj_get_int(pin_obj);
    int value = -1;

	switch (pin) {
	case 0:
	    value = GPIO_PORTF_DATA_R & 0x1;  // read data from PORTF pin 0
		break;
	case 4:
	    value = (GPIO_PORTF_DATA_R & 0x10) >> 4;  // read data from PORTF pin 4
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable for this function.\n");
		break;
    }
    
    return mp_obj_new_int(value);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_read_obj, gpio_read);


STATIC mp_obj_t gpio_init(void) {
    //
    // Enable the GPIO port that is used for the on-board LEDs and Switches.
    //
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpio_init_obj, gpio_init);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t gpio_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_gpio) },
    { MP_ROM_QSTR(MP_QSTR_pf0), MP_ROM_INT(PF0) },
    { MP_ROM_QSTR(MP_QSTR_pf1), MP_ROM_INT(PF1) },
    { MP_ROM_QSTR(MP_QSTR_pf2), MP_ROM_INT(PF2) },
    { MP_ROM_QSTR(MP_QSTR_pf3), MP_ROM_INT(PF3) },
    { MP_ROM_QSTR(MP_QSTR_pf4), MP_ROM_INT(PF4) },
    { MP_ROM_QSTR(MP_QSTR_sw2), MP_ROM_INT(PF0) },
    { MP_ROM_QSTR(MP_QSTR_red), MP_ROM_INT(PF1) },
    { MP_ROM_QSTR(MP_QSTR_blue), MP_ROM_INT(PF2) },
    { MP_ROM_QSTR(MP_QSTR_green), MP_ROM_INT(PF3) },
    { MP_ROM_QSTR(MP_QSTR_sw1), MP_ROM_INT(PF4) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&gpio_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_output), MP_ROM_PTR(&gpio_output_obj) },
    { MP_ROM_QSTR(MP_QSTR_input), MP_ROM_PTR(&gpio_input_obj) },
    { MP_ROM_QSTR(MP_QSTR_up), MP_ROM_PTR(&gpio_up_obj) },
    { MP_ROM_QSTR(MP_QSTR_down), MP_ROM_PTR(&gpio_down_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&gpio_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&gpio_init_obj) },
};
STATIC MP_DEFINE_CONST_DICT(gpio_module_globals, gpio_module_globals_table);

const mp_obj_module_t gpio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&gpio_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_gpio, gpio_module, MICROPY_MODULE_GPIO);
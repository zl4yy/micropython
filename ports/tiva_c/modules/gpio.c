/*

	Basic GPIO control for Texas Instruments LM4F Microcontrollers
	Limited to pins that are exposed on the Stellaris LaunchPad board
	Includes LEDs and Switches (see boards/pins_def.h)
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	5 June 2021
	
	Software provided under MIT License

*/
#include "modules/gpio.h"
#include "modules/time.h"
#include <stdbool.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"
#include "boards/pins_def.h"

bool GPIO_InitDone = false;

/*

    Internal C functions (For use in other C modules)   

*/

void gpio_error_notinitialised () {
  // Send generic error
	  mp_printf(&mp_plat_print, "GPIO not initialised.\n");
}

// Enable the GPIO port that is used for the on-board LEDs and Switches.
void Do_GPIO_Init() {
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOB | SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD | SYSCTL_RCGC2_GPIOE | SYSCTL_RCGC2_GPIOF;
	GPIO_InitDone = true;
}

// Enable the GPIO pins for the LEDs.  Set the direction as output, and
// enable the GPIO pin for digital function.
void Do_GPIO_output(int pin) {
	uint8_t value=0x00;

	switch (pin%10) {	// Select Mask / value
	case 0:
		value = 0x01;
		break;
	case 1:
		value = 0x02;
		break;
	case 2:
		value = 0x04;
		break;
	case 3:
		value = 0x08;
		break;
	case 4:
		value = 0x10;
		break;
	case 5:
		value = 0x20;
		break;
	case 6:
		value = 0x40;
		break;
	case 7:
		value = 0x80;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable.\n");
		break;
	}

	switch (pin/10) {	// Set GPIO port as output for the selected pin
	case 1:
		GPIO_PORTA_DIR_R |= value;
		GPIO_PORTA_DEN_R |= value;
		break;
	case 2:
		GPIO_PORTB_DIR_R |= value;
		GPIO_PORTB_DEN_R |= value;
		break;
	case 3:
		GPIO_PORTC_DIR_R |= value;
		GPIO_PORTC_DEN_R |= value;
		break;
	case 4:
		GPIO_PORTD_DIR_R |= value;
		GPIO_PORTD_DEN_R |= value;
		break;
	case 5:
		GPIO_PORTE_DIR_R |= value;
		GPIO_PORTE_DEN_R |= value;
		break;
	case 6:
		GPIO_PORTF_DIR_R |= value;
		GPIO_PORTF_DEN_R |= value;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Port unavailable.\n");
		break;
    }

}

// Enable the GPIO pins for the Switches.  Set the direction as input, and
// enable the GPIO pin for digital function.
void Do_GPIO_input(int pin) {
	// TODO: As above for output but first special case for 60 with "LOCK and CR commands"
	uint8_t value=0x00;

	switch (pin%10) {	// Select Mask / value
	case 0:
		value = 0x01;
		break;
	case 1:
		value = 0x02;
		break;
	case 2:
		value = 0x04;
		break;
	case 3:
		value = 0x08;
		break;
	case 4:
		value = 0x10;
		break;
	case 5:
		value = 0x20;
		break;
	case 6:
		value = 0x40;
		break;
	case 7:
		value = 0x80;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable.\n");
		break;
	}

	switch (pin) {
		// Dealing with special cases
		// PD7 and PF0 (Switch 2) are NMI pins.
		// In order to use them for any function other than NMI, the pins need be unlocked first.

	case 47:
		GPIO_PORTD_LOCK_R = 0x4C4F434B;    // unlock commit register
	    GPIO_PORTD_CR_R |= 0x80;           // make PORTD7 configurable
		break;
	case 60:
		GPIO_PORTF_LOCK_R = 0x4C4F434B;    // unlock commit register
	    GPIO_PORTF_CR_R |= 0x01;           // make PORTF0 configurable
		break;
    }

	switch (pin/10) {	// Set GPIO port as input for the selected pin
	case 1:
		GPIO_PORTA_DIR_R &= ~value;
		GPIO_PORTA_DEN_R |= value;
		GPIO_PORTA_PUR_R |= value;         // enable pull up
		break;
	case 2:
		GPIO_PORTB_DIR_R &= ~value;
		GPIO_PORTB_DEN_R |= value;
		GPIO_PORTB_PUR_R |= value;         // enable pull up
		break;
	case 3:
		GPIO_PORTC_DIR_R &= ~value;
		GPIO_PORTC_DEN_R |= value;
		GPIO_PORTC_PUR_R |= value;         // enable pull up
		break;
	case 4:
		GPIO_PORTD_DIR_R &= ~value;
		GPIO_PORTD_DEN_R |= value;
		GPIO_PORTD_PUR_R |= value;         // enable pull up
		break;
	case 5:
		GPIO_PORTE_DIR_R &= ~value;
		GPIO_PORTE_DEN_R |= value;
		GPIO_PORTE_PUR_R |= value;         // enable pull up
		break;
	case 6:
		GPIO_PORTF_DIR_R &= ~value;
		GPIO_PORTF_DEN_R |= value;
		GPIO_PORTF_PUR_R |= value;         // enable pull up
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Port unavailable.\n");
		break;
    }
}

// Set pin value to High
void Do_GPIO_up(int pin) {
	uint8_t value=0x00;

	switch (pin%10) {	// Select Mask / value
	case 0:
		value = 0x01;
		break;
	case 1:
		value = 0x02;
		break;
	case 2:
		value = 0x04;
		break;
	case 3:
		value = 0x08;
		break;
	case 4:
		value = 0x10;
		break;
	case 5:
		value = 0x20;
		break;
	case 6:
		value = 0x40;
		break;
	case 7:
		value = 0x80;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable.\n");
		break;
	}

	switch (pin/10) {	// Set value to the GPIO port
	case 1:
		GPIO_PORTA_DATA_R |= value;
		break;
	case 2:
		GPIO_PORTB_DATA_R |= value;
		break;
	case 3:
		GPIO_PORTC_DATA_R |= value;
		break;
	case 4:
		GPIO_PORTD_DATA_R |= value;
		break;
	case 5:
		GPIO_PORTE_DATA_R |= value;
		break;
	case 6:
		GPIO_PORTF_DATA_R |= value;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Port unavailable.\n");
		break;
	}
}

// Set pin value to Low
void Do_GPIO_down(int pin) {
	uint8_t value = 0x00;

	switch (pin%10) {	// Select Mask / value
	case 0:
		value = 0x01;
		break;
	case 1:
		value = 0x02;
		break;
	case 2:
		value = 0x04;
		break;
	case 3:
		value = 0x08;
		break;
	case 4:
		value = 0x10;
		break;
	case 5:
		value = 0x20;
		break;
	case 6:
		value = 0x40;
		break;
	case 7:
		value = 0x80;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable.\n");
		break;
	}

	switch (pin/10) {	// Set value to the GPIO port
	case 1:
		GPIO_PORTA_DATA_R &= ~value;
		break;
	case 2:
		GPIO_PORTB_DATA_R &= ~value;
		break;
	case 3:
		GPIO_PORTC_DATA_R &= ~value;
		break;
	case 4:
		GPIO_PORTD_DATA_R &= ~value;
		break;
	case 5:
		GPIO_PORTE_DATA_R &= ~value;
		break;
	case 6:
		GPIO_PORTF_DATA_R &= ~value;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Port unavailable.\n");
		break;
    }
}

// Wrapper to mimic Arduino/Energia digitalWrite 
void Do_GPIO_write(int pin, bool value) {
	if (value == true) {
		Do_GPIO_up(pin);
	} else {
		Do_GPIO_down(pin);
	}
}

// Read value from input pin
int Do_GPIO_read(int pin) {
	uint8_t value = 0x00;

	switch (pin%10) {	// Select Mask / value
	case 0:
		value = 0x01;
		break;
	case 1:
		value = 0x02;
		break;
	case 2:
		value = 0x04;
		break;
	case 3:
		value = 0x08;
		break;
	case 4:
		value = 0x10;
		break;
	case 5:
		value = 0x20;
		break;
	case 6:
		value = 0x40;
		break;
	case 7:
		value = 0x80;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Pin unavailable.\n");
		break;
	}

	switch (pin/10) {	// Read value from the GPIO port
						// with mask coming from "value" as set above
	case 1:
		value = GPIO_PORTA_DATA_R & value;  
		break;
	case 2:
		value = GPIO_PORTB_DATA_R & value; 
		break;
	case 3:
		value = GPIO_PORTC_DATA_R & value; 
		break;
	case 4:
		value = GPIO_PORTD_DATA_R & value;  
		break;
	case 5:
		value = GPIO_PORTE_DATA_R & value;  
		break;
	case 6:
		value = GPIO_PORTF_DATA_R & value;
		break;
	case 9:
		// 99 is NULL value
		break;
	default:
	    mp_printf(&mp_plat_print, "Port unavailable.\n");
		break;
    }

	// Bit read will be returned at the "mask" position.
	// It needs to be shifted right by the number of bits equal to "pin number"
	value = value >> (pin%10); 

	return value;
}

// Shift out function from Arduino source code
void Do_GPIO_shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
      uint8_t i;

      for (i = 0; i < 8; i++)  {
            if (bitOrder == LSBFIRST)
                  Do_GPIO_write(dataPin, !!(val & (1 << i)));
            else      
                  Do_GPIO_write(dataPin, !!(val & (1 << (7 - i))));
                  
			Do_SysTick_Waitus(10);	// Wait times were required for PD8544
            Do_GPIO_write(clockPin, true);
			Do_SysTick_Waitus(10);
            Do_GPIO_write(clockPin, false);            
      }
}
/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t gpio_info(void) {
    mp_printf(&mp_plat_print, "Basic GPIO Functions to control LEDs. Initialise by gpio.init()\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpio_info_obj, gpio_info);

// Initialise GPIO pins
STATIC mp_obj_t gpio_init(void) {
	Do_GPIO_Init();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpio_init_obj, gpio_init);

// Set pin as output
STATIC mp_obj_t gpio_output(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);
    if (GPIO_InitDone == false) {
		gpio_error_notinitialised();
	} else{
		Do_GPIO_output(pin);
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_output_obj, gpio_output);

// Set pin as input
STATIC mp_obj_t gpio_input(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);
    if (GPIO_InitDone == false) {
		gpio_error_notinitialised();
	} else{
		Do_GPIO_input(pin);
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_input_obj, gpio_input);

// Set output pin value to High
STATIC mp_obj_t gpio_up(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);
    if (GPIO_InitDone == false) {
		gpio_error_notinitialised();
	} else{
		Do_GPIO_up(pin);
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_up_obj, gpio_up);

// Set output pin value to Low
STATIC mp_obj_t gpio_down(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);
    if (GPIO_InitDone == false) {
		gpio_error_notinitialised();
	} else{
		Do_GPIO_down(pin);
	}
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_down_obj, gpio_down);

// Reading a value from the GPIO port
STATIC mp_obj_t gpio_read(mp_obj_t pin_obj) {
    int pin = mp_obj_get_int(pin_obj);
    int value = -1;

    if (GPIO_InitDone == false) {
		gpio_error_notinitialised();
    } else {
		value = Do_GPIO_read(pin);
	}    
    return mp_obj_new_int(value);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(gpio_read_obj, gpio_read);

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
/*

	Basic XPT2046 Touch Screen support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	3 September 2021
	
	Software provided under MIT License

	Based on the following library:
	https://github.com/ImpulseAdventure/Arduino-TFT-Library-ILI9486
	
*/

#include <stdbool.h>
#include "py/objstr.h"
#include "modules/gpio.h"
#include "modules/time.h"

#include "xpt2046.h"
#include "modules/ssi.h"
#include "modules/lcd_ili9486.h"

uint8_t _spiport_xpt2046;
uint8_t _pinChipSelect_xpt;
uint8_t _touchrotation=3;
int16_t _zthreshold=400;
int16_t _xmincal = 4095, _ymincal = 4095, _xmaxcal = 0, _ymaxcal = 0;

int16_t xraw=0, yraw=0, zraw=0;

bool XPT2046_InitDone = false;


void xpt2046_error_notinitialised () {
	// Send generic error
	mp_printf(&mp_plat_print, "XPT2046 not initialised.\n");
}

void Do_XPT_startWrite() {
	Do_GPIO_down(_pinChipSelect_xpt);
	Do_SysTick_Waitus(100);
}

void Do_XPT_endWrite() {
	Do_SysTick_Waitus(100);
	Do_GPIO_up(_pinChipSelect_xpt);
}

void Do_XPT_Init(uint8_t spiport, uint8_t pinChipSelect) {

	// Set GPIO Pin values for global variables
	_spiport_xpt2046    = spiport;
	_pinChipSelect_xpt  = pinChipSelect;

	// Initialise MCU hardware
	Do_GPIO_Init();

	Do_GPIO_output(_pinChipSelect_xpt);
	Do_GPIO_up(_pinChipSelect_xpt);

	Do_SSI_Init(_spiport_xpt2046,10031,false);

	_xmincal = 4095, _ymincal = 4095, _xmaxcal = 0, _ymaxcal = 0;
	XPT2046_InitDone = true;
}

int16_t Do_XPT_transfer(uint16_t command) {
	uint32_t word;
	uint8_t msb,lsb;
	Do_SSI_TX_FIFO(_spiport_xpt2046, (uint32_t)(command>>8)&0xff);
	Do_SSI_RX(_spiport_xpt2046, &word);
	msb = (uint8_t)word;
	Do_SSI_TX_FIFO(_spiport_xpt2046, (uint32_t)(command)&0xff);
	Do_SSI_RX(_spiport_xpt2046, &word);
	lsb = (uint8_t)word;

	return (int16_t)(((msb << 8) | lsb) >> 3);
}

int16_t Do_XPT_besttwoavg( int16_t x , int16_t y , int16_t z ) {
	int16_t da, db, dc;
	int16_t reta = 0;
	if ( x > y ) {
		da = x - y;
	} else {
		da = y - x;
	};
	if ( x > z ) {
		db = x - z;
	} else {
		db = z - x;
	};
	if ( z > y ) {
		dc = z - y;
	} else {
		dc = y - z;
	};

	if ( da <= db && da <= dc ) {
		reta = (x + y) >> 1;
	} else if ( db <= da && db <= dc ) {
		reta = (x + z) >> 1;
	} else {
		reta = (y + z) >> 1;
	}   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

	return (reta);
}

void Do_XPT_update() {
	int16_t data[6];

	Do_XPT_startWrite();

	Do_XPT_transfer(0xB3); // Requesting Z1
	int16_t z1 = Do_XPT_transfer(0xC3); // Requesting Z2, receiving Z1
	int16_t z2 = Do_XPT_transfer(0x93); // Requesting X (dummy measure), receiving Z2
	int16_t z = (4095 - z2) + z1;

	if (z >= _zthreshold) {
		Do_XPT_transfer(0x93);  // Requesting X
		data[0] = Do_XPT_transfer(0xD3); // Requesting Y (dummy measure), receiving X
		data[1] = Do_XPT_transfer(0x93); // X, make 3 x-y measurements
		data[2] = Do_XPT_transfer(0xD3); // Y
		data[3] = Do_XPT_transfer(0x93); // X
	}
	else data[0] = data[1] = data[2] = data[3] = 0;	// Compiler warns these values may be used unset on early exit.
	data[4] = Do_XPT_transfer(0xD3);	// Last Y touch 
	data[5] = Do_XPT_transfer(0xD0);	// Dummy read to obtain data and 2 last bits 0 to power down

	Do_XPT_endWrite();

	//mp_printf(&mp_plat_print, "z=%d  ::  z1=%d,  z2=%d  \n", z, z1, z2);
	if (z < 0) z = 0;
	if (z < _zthreshold) { //	if ( !touched ) {
		zraw = 0;
		return;
	} else {
		zraw = z;
		// Average pair with least distance between each measured x then y
		//mp_printf(&mp_plat_print, "p=%d,  %d,%d  %d,%d  %d,%d ++ ", zraw, data[0], data[1], data[2], data[3], data[4], data[5]);

		int16_t x = Do_XPT_besttwoavg( data[0], data[2], data[4] );
		int16_t y = Do_XPT_besttwoavg( data[1], data[3], data[5] );
	
		//mp_printf(&mp_plat_print, "    %d,%d \n", x, y);

		switch (_touchrotation) {
		  case 0:
			xraw = 4095 - y;
			yraw = x;
			break;
		  case 1:
			xraw = x;
			yraw = y;
			break;
		  case 2:
			xraw = y;
			yraw = 4095 - x;
			break;
		  default: // 3
			xraw = 4095 - x;
			yraw = 4095 - y;
		}

		// Auto calibration routine, saving maximu values read
		if (xraw > _xmaxcal) _xmaxcal = xraw;
		if (yraw > _ymaxcal) _ymaxcal = yraw;

		if (xraw < _xmincal) _xmincal = xraw;
		if (yraw < _ymincal) _ymincal = yraw;
	}
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t xpt_info(void) {
	mp_printf(&mp_plat_print, "XPT2046 Touchscreen Driver.\n");
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(xpt_info_obj, xpt_info);

// Initialise Touchscreen
STATIC mp_obj_t xpt_init(mp_obj_t spi_obj, mp_obj_t cs_obj) {
	uint8_t spiport = mp_obj_get_int(spi_obj);
	uint8_t pinChipSelect = mp_obj_get_int(cs_obj);
	Do_XPT_Init(spiport, pinChipSelect);
	// Do two dummy reads
	_xmaxcal = _ymaxcal = 0;
	_xmincal = _ymincal = 4095;

	Do_XPT_update();
	Do_SysTick_Waitms(50);
	Do_XPT_update();

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(xpt_init_obj, xpt_init);

// Set rotation
STATIC mp_obj_t xpt_setRotation(mp_obj_t r_obj) {

	_touchrotation =  mp_obj_get_int(r_obj)%4;

	// If changing orientation we need to reset calibration
	_xmincal = 4095, _ymincal = 4095, _xmaxcal = 0, _ymaxcal = 0;

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(xpt_setRotation_obj, xpt_setRotation);

// Set threshold
STATIC mp_obj_t xpt_setThreshold(mp_obj_t zt_obj) {

	_zthreshold =  mp_obj_get_int(zt_obj)%4095;

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(xpt_setThreshold_obj, xpt_setThreshold);


// Print Values
STATIC mp_obj_t xpt_printRaw(void) {
	if (!XPT2046_InitDone) {
		xpt2046_error_notinitialised();
	} else {
		Do_XPT_update();
		mp_printf(&mp_plat_print, "X=%d Y=%d Z=%d \n", xraw, yraw, zraw);
	}
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(xpt_printRaw_obj, xpt_printRaw);


// Take measurements and Return Z
STATIC mp_obj_t xpt_getZ(void) {
	if (!XPT2046_InitDone) {
		xpt2046_error_notinitialised();
	} else {
		Do_XPT_update();
	}
	return mp_obj_new_int(zraw);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(xpt_getZ_obj, xpt_getZ);

// Return X for 320x480 TFT screen resolution after calibration
STATIC mp_obj_t xpt_getX(void) {
	if (!XPT2046_InitDone) {
		xpt2046_error_notinitialised();
	}
	uint32_t x = ((uint32_t)xraw-(uint32_t)_xmincal)*TFT_WIDTH/(uint32_t)_xmaxcal;
	return mp_obj_new_int(x);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(xpt_getX_obj, xpt_getX);

// Return Y for 320x480 TFT screen resolution after calibration
STATIC mp_obj_t xpt_getY(void) {
	if (!XPT2046_InitDone) {
		xpt2046_error_notinitialised();
	}
	uint32_t y = ((uint32_t)yraw-(uint32_t)_ymincal)*TFT_HEIGHT/(uint32_t)_ymaxcal;
	return mp_obj_new_int(y);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(xpt_getY_obj, xpt_getY);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t xpt2046_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_xpt2046) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&xpt_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&xpt_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_setThreshold), MP_ROM_PTR(&xpt_setThreshold_obj) },
    { MP_ROM_QSTR(MP_QSTR_setRotation), MP_ROM_PTR(&xpt_setRotation_obj) },
    { MP_ROM_QSTR(MP_QSTR_printRaw), MP_ROM_PTR(&xpt_printRaw_obj) },
    { MP_ROM_QSTR(MP_QSTR_getZ), MP_ROM_PTR(&xpt_getZ_obj) },
    { MP_ROM_QSTR(MP_QSTR_getX), MP_ROM_PTR(&xpt_getX_obj) },
    { MP_ROM_QSTR(MP_QSTR_getY), MP_ROM_PTR(&xpt_getY_obj) },
};
STATIC MP_DEFINE_CONST_DICT(xpt2046_module_globals, xpt2046_module_globals_table);

const mp_obj_module_t xpt2046_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&xpt2046_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_xpt2046, xpt2046_module, MICROPY_MODULE_XPT2046);
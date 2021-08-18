/*

	Basic BMP085 Pressure sensor support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	4 July 2021
	
	Software provided under MIT License

*/

#include <stdbool.h>
#include <float.h>
#include "modules/time.h"
#include "modules/i2c.h"
#include "bmp085.h"

#define SLAVEADDR (0x77)  // Last bit (LSB bit) is read mode

uint8_t _i2cPort_bmp085 = 0;
int16_t ac1;          // Calibration parameters
int16_t ac2;
int16_t ac3;
uint16_t ac4;
uint16_t ac5;
uint16_t ac6;
int16_t b1;
int16_t b2;
int16_t mb;
int16_t mc;
int16_t md;
int8_t oss=0;         // Oversampling factor
uint32_t utemp=0;      // Uncompensated temperature 
uint32_t upressure=0;  // Uncompensated pressure
int16_t temp=0;      // Temperature in 1/10 of deg C
int32_t pressure=0;  // Pressure in Pa

bool BMP085_InitDone = false;

/*
    Internal formatting / housekeeping functions
*/


// Send generic error
void bmp085_error_notinitialised () {
	  mp_printf(&mp_plat_print, "BMP085 not initialised.\n");
}

/*
    Functions for use in other libraries
*/

// Initialise the BMP085 and I2C
bool Do_BMP085_Init(uint8_t port) {
  
  _i2cPort_bmp085 = port;
  bool err = false;
  uint8_t data[3];

  // Initialise MCU hardware
  Do_SysTick_Init();
  Do_I2C_MasterInit(_i2cPort_bmp085, 0);   // Low speed I2C only is supported
  Do_SysTick_Waitms(5);   // Wait for initialisation
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xd0);  // Chip ID register
  err |= Do_I2C_MasterRX(_i2cPort_bmp085, SLAVEADDR, data);
  if (data[0] != 0x55) {
	  mp_printf(&mp_plat_print, "BMP085 not detected.\n");
    err = true;
    return err;  
  } else {
    BMP085_InitDone = true;
  }
  Do_SysTick_Waitms(5);   // Wait for initialisation

  // Read Calibration Data
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xaa);  // AC1 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac1 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xac);  // AC2 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac2 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xae);  // AC3 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac3 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xb0);  // AC4 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac4 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xb2);  // AC5 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac5 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xb4);  // AC6 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  ac6 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xb6);  // B1 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  b1 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xb8);  // B2 register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  b2 = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xba);  // mb register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  mb = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xbc);  // mc register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  mc = (data[0]<<8) | data[1];
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xbe);  // md register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  md = (data[0]<<8) | data[1];

  return err;
}

// Acquire data from the BMP085 chip
bool Do_BMP085_readdata(void) {

  bool err = false;
  uint8_t data[3];

  data[0] = 0xf4; // Register
  data[1] = 0x2e; // Value to get temperature
  err |= Do_I2C_MasterTXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  Do_SysTick_Waitms(10);   // Wait for sensor to convert temperature
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xf6);  // Read value register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  utemp = (data[0]<<8) | data[1];
  // Do temperature calculations
  int32_t X1 = ((int32_t)utemp - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
  int32_t B5 = X1+X2;
  temp = (B5 + 8) >> 4;

  data[0] = 0xf4; // Register
  data[1] = 0x34+(oss<<6); // Value to get pressure
  err |= Do_I2C_MasterTXBurst(_i2cPort_bmp085, SLAVEADDR, 2, data);
  Do_SysTick_Waitms(30);   // Wait for sensor to convert pressure, maximum conversion time is 25ms
  err |= Do_I2C_MasterTX(_i2cPort_bmp085, SLAVEADDR, 0xf6);  // Read value register
  err |= Do_I2C_MasterRXBurst(_i2cPort_bmp085, SLAVEADDR, 3, data);
  upressure = ((data[0]<<16) | (data[1]<<8) | data[2]) >> (8-oss);

  // Do pressure calculations
  int32_t B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  int32_t X3 = X1 + X2;
  int32_t B3 = ((((int32_t)ac1 * 4 + X3) << oss) + 2) / 4;
  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  uint32_t B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  uint32_t B7 = ((uint32_t)upressure - B3) * (50000UL >> oss);  
  if (B7 < 0x80000000) {
    pressure = (B7 * 2) / B4;
  } else {
    pressure = (B7 / B4) * 2;
  };
  X1 = (pressure >> 8) * (pressure >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * pressure) >> 16;
  pressure = pressure + ((X1 + X2 + 3791L) >> 4);

  return err;
}


/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t bmp085_info(void) {
  mp_printf(&mp_plat_print, "Control BMP085 pressure sensor. Initialise by bmp085.init().\n");
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bmp085_info_obj, bmp085_info);

// Initialise BMP085
STATIC mp_obj_t bmp085_init(mp_obj_t i2cnum_obj) {
  uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
	if (!Do_BMP085_Init(i2cnum)) {
    mp_printf(&mp_plat_print, "BMP085 initialised.\n");
  };
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(bmp085_init_obj, bmp085_init);

// Set overssampling factor
STATIC mp_obj_t bmp085_setoss(mp_obj_t oss_obj) {
  uint8_t oss = mp_obj_get_int(oss_obj);
  if (oss<0) oss=0;
  if (oss>3) oss=3;
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(bmp085_setoss_obj, bmp085_setoss);

// Print Pressure and temperature
STATIC mp_obj_t bmp085_print() {
  if (BMP085_InitDone == false) {
    bmp085_error_notinitialised();
	} else {
    if (Do_BMP085_readdata()) {
      mp_printf(&mp_plat_print, "BMP085 not ready.");
    } else {
      mp_printf(&mp_plat_print, "Temperature: %d.%d deg. C \n", temp/10, temp%10);
      mp_printf(&mp_plat_print, "Pressure: %d.%d%d hPa \n", pressure/100, (pressure%100)/10, pressure%10);
    }
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bmp085_print_obj, bmp085_print);

// Return temperature
STATIC mp_obj_t bmp085_get_temp() {
  if (BMP085_InitDone == false) {
    bmp085_error_notinitialised();
	} else {
    if (Do_BMP085_readdata()) {
      mp_printf(&mp_plat_print, "BMP085 not ready.");
      return mp_const_none;
    }
	}
  return mp_obj_new_int(temp);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bmp085_get_temp_obj, bmp085_get_temp);

// Return pressure
STATIC mp_obj_t bmp085_get_pressure() {
  if (BMP085_InitDone == false) {
    bmp085_error_notinitialised();
	} else {
    if (Do_BMP085_readdata()) {
      mp_printf(&mp_plat_print, "BMP085 not ready.");
      return mp_const_none;
    }
	}
  return mp_obj_new_int(pressure);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bmp085_get_pressure_obj, bmp085_get_pressure);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t bmp085_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_bmp085) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&bmp085_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&bmp085_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_setoss), MP_ROM_PTR(&bmp085_setoss_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&bmp085_print_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_temp), MP_ROM_PTR(&bmp085_get_temp_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_pressure), MP_ROM_PTR(&bmp085_get_pressure_obj) },
};
STATIC MP_DEFINE_CONST_DICT(bmp085_module_globals, bmp085_module_globals_table);

const mp_obj_module_t bmp085_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&bmp085_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_bmp085, bmp085_module, MICROPY_MODULE_BMP085);
/*

	Basic DS1307 RTC Clock support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	30 June 2021
	
	Software provided under MIT License
  Based on DS1307new library by Peter Schmelzer and Oliver Kraus

*/

#include <stdbool.h>
#include "py/objstr.h"
#include "modules/i2c.h"
#include "ds1307.h"

#define SLAVEADDR (0b01101000)  // Last bit (LSB bit) is read mode

uint8_t _i2cPort = 0;
uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t dow;
uint8_t day;
uint8_t month;
uint8_t year;

bool RTC_InitDone = false;

/*
    Internal formatting / housekeeping functions
*/


// Send generic error
void rtc_error_notinitialised () {
	  mp_printf(&mp_plat_print, "DS1307 not initialised.\n");
}

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t dec2bcd(uint8_t num) {
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t bcd2dec(uint8_t num) {
  return ((num/16 * 10) + (num % 16));
}
/*
uint8_t is_leap_year(uint16_t y)
{
  //uint16_t y = year;
   if ( 
          ((y % 4 == 0) && (y % 100 != 0)) || 
          (y % 400 == 0) 
      )
      return 1;
   return 0;
}

void calculate_ydn(void)
{
  uint8_t tmp1; 
  uint16_t tmp2;
  tmp1 = 0;
  if ( month >= 3 )
    tmp1++;
  tmp2 = month;
  tmp2 +=2;
  tmp2 *=611;
  tmp2 /= 20;
  tmp2 += day;
  tmp2 -= 91;
  tmp1 <<=1;
  tmp2 -= tmp1;
  if ( tmp1 != 0 )
    tmp2 += is_leap_year(year);
  ydn = tmp2;
}

void calculate_cdn(void)
{
  uint16_t y = year;
  cdn = ydn;
  cdn--;
  while( y > 2000 )
  {
    y--;
    cdn += 365;
    cdn += is_leap_year(y);
  }
}

void calculate_dow(void)
{
  uint16_t tmp;
  tmp = cdn;
  tmp += 6;
  tmp %= 7;
  dow = tmp;
}

void calculate_time2000(void)
{
  uint32_t t;
  t = cdn;
  t *= 24;
  t += hour;
  t *= 60;
  t += minute;
  t *= 60;
  t += second;
  time2000 = t;
}
*/
/*
    Functions for use in other libraries
*/

// Initialise the DS1307 and I2C
void Do_RTC_Init(uint8_t port) {
  
  _i2cPort = port;

  // Initialise MCU hardware
  Do_I2C_MasterInit(_i2cPort, 0);   // Low speed I2C only is supported
  if (Do_I2C_MasterTX(_i2cPort, SLAVEADDR, 0x00) != false) {
	  mp_printf(&mp_plat_print, "DS1307 not detected.\n");    
  } else {
    RTC_InitDone = true;
  }

  // Set default values

}

// Aquire time from the RTC chip in BCD format and convert it to DEC
bool Do_RTC_readtime(void) {
  Do_I2C_MasterTX(_i2cPort, SLAVEADDR, 0x00);

  bool err = false;
  uint8_t data[7];

  err = Do_I2C_MasterRXBurst(_i2cPort, SLAVEADDR, 7, data);
  second = bcd2dec(data[0] & 0x7f);      // aquire seconds...
  minute = bcd2dec(data[1]);             // aquire minutes
  hour = bcd2dec(data[2]);               // aquire hours
  dow = bcd2dec(data[3]);                // aquire dow (Day Of Week)
  dow--;	        						        //  correction from RTC format (1..7) to lib format (0..6). Useless, because it will be overwritten
  day = bcd2dec(data[4]);                // aquire day
  month = bcd2dec(data[5]);      // aquire month
  year = bcd2dec(data[6]);       // aquire year...
  year = year + 2000;                   // ...and assume that we are in 21st century!
  
  // recalculate all other values
  //calculate_ydn();
  //calculate_cdn();
  //calculate_dow();
  //calculate_time2000();

  return err;
}


/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t rtc_info(void) {
  mp_printf(&mp_plat_print, "Control DS1307 RTC Clock. Initialise by rtc.init().\n");
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(rtc_info_obj, rtc_info);

// Initialise DS1307
STATIC mp_obj_t rtc_init(mp_obj_t i2cnum_obj) {
  uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
	Do_RTC_Init(i2cnum);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(rtc_init_obj, rtc_init);

// Print Time and Date
STATIC mp_obj_t rtc_print() {
  if (RTC_InitDone == false) {
    rtc_error_notinitialised();
	} else {
    if (Do_RTC_readtime()) {
      mp_printf(&mp_plat_print, "DS1307 not ready.");
    } else {
      mp_printf(&mp_plat_print, "%d-%d_%d %d:%d:%d", year, month, day, hour, minute, second);
    }
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(rtc_print_obj, rtc_print);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t rtc_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_rtc) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&rtc_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&rtc_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&rtc_print_obj) },
};
STATIC MP_DEFINE_CONST_DICT(rtc_module_globals, rtc_module_globals_table);

const mp_obj_module_t rtc_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&rtc_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_rtc, rtc_module, MICROPY_MODULE_DS1307);
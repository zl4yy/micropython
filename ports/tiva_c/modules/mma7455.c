/*

	Basic MMA7455 accelerometer sensor support for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	29 July 2021
	
	Software provided under MIT License

*/

#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include "modules/time.h"
#include "modules/i2c.h"
#include "mma7455.h"

// Declaring a union type to do the conversion between 2 uint8 and 1 uint16 as it shares same memory
struct val {
  uint16_t x;      // Acceleration on X axis
  uint16_t y;      // Acceleration on X axis
  uint16_t z;      // Acceleration on X axis
};

struct val acc;
uint8_t _i2cPort_mma7455 = 0;
uint8_t _Gratio = 64; // Multiplying ratio for senstivity (Default 1G = 64)

bool MMA7455_InitDone = false;

/*
    Internal formatting / housekeeping functions
*/


// Send generic error messages
void mma7455_error_notinitialised () {
	mp_printf(&mp_plat_print, "MMA7455 not initialised.\n");
}

void mma7455_error_notready () {
  mp_printf(&mp_plat_print, "MMA7455 not ready.");
}
/*
    Functions for use in other libraries
*/

// Initialise the MMA7455 and I2C
bool Do_MMA7455_Init(uint8_t port, uint8_t sensitivity) {
  
  _i2cPort_mma7455 = port;
  bool err = false;
  uint8_t i;
  uint8_t data[7];

  struct val offsets;

  // Initialise MCU hardware
  Do_I2C_MasterInit(_i2cPort_mma7455, 0);   // Low speed I2C only is supported
  Do_SysTick_Waitms(5);   // Wait for initialisation
  err |= Do_I2C_MasterTX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, MMA7455_I2CAD);  // Read it back to verify
  err |= Do_I2C_MasterRX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, data);
  if ((data[0] != 0x1d) || err) {
	  mma7455_error_notready();
    err = true;
    return err;  
  } else {
    data[0] = MMA7455_MCTL;   // Control register
    data[1] = 1<<MMA7455_STON;    // Self-Test Mode
    err |= Do_I2C_MasterTXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 2, data);  // Set control register
    Do_SysTick_Waitms(10);   // Wait for self test
    err |= Do_I2C_MasterTX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, MMA7455_STATUS);  // Read status
    err |= Do_I2C_MasterRX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, data);
    if ((data[0] & 0x04) || err) { // Self-test returned error
  	  mp_printf(&mp_plat_print, "Self-test error.\n");
      err = true;
      return err;  
    }

    // Prepare data to configure control register 
    data[0] = MMA7455_MCTL;   // Control register
    switch (sensitivity)
    {
    case 8:
      /* 8G sensitivity */
      data[1] = 1<<MMA7455_MODE0;    // 8G sensitivity and Measurement mode
      _Gratio = 16;
      break;
    case 4:
      /* 4G sensitivity */
      data[1] = 1<<MMA7455_GLVL1 | 1<<MMA7455_MODE0;    // 4G sensitivity and Measurement mode
      _Gratio = 32;
      break;
    case 2:
    default:
      /* Default is 2G max sensitivity */
      data[1] = 1<<MMA7455_GLVL0 | 1<<MMA7455_MODE0;    // 4G sensitivity and Measurement mode
      _Gratio = 64;    
      break;
    }

    err |= Do_I2C_MasterTXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 2, data);  // Set control register
    Do_SysTick_Waitms(5);   // Wait for initialisation

    // Clear the offset registers.
    // If the Arduino was reset or with a warm-boot,
    // there still could be offset written in the sensor.
    // Only with power-up the offset values of the sensor
    // are zero.
    for (i=1;i<7;i++){
      data[i]=0;
    }
    data[0] = MMA7455_XOFFL;   // Control register
    err |= Do_I2C_MasterTXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 7, data);  // Set offset registers for X Y Z in one pass
    Do_SysTick_Waitms(100);   // Wait for stabilisation

    // Calcuate the offset.
    //
    // The values are 16-bits signed integers, but the sensor
    // uses offsets of 11-bits signed integers.
    // However that is not a problem,
    // as long as the value is within the range.

    // Assuming that the sensor is flat horizontal,
    // the 'z'-axis should be 1 'g'. And 1 'g' is
    // a value of 64 (if the 2g most sensitive setting
    // is used).  
    // Note that the actual written value should be doubled
    // for this sensor.

    err |= Do_MMA7455_readdata(); // get the x,y,z values

    offsets.x = -2 * acc.x;        // The sensor wants double values.
    offsets.y = -2 * acc.y;
    offsets.z = -2 * (acc.z-_Gratio);   // Gratio is for 1 'g' for z-axis.

    // X
    data[1] = (uint8_t)(offsets.x & 0xFF);
    if ((offsets.x>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[2] = (uint8_t)((offsets.x>>8) | 0x02); // Negative
    } else {
      data[2] = (uint8_t)((offsets.x>>8) & 0x01); // Positive
    }
    // Y
    data[3] = (uint8_t)(offsets.y & 0xFF);
    if ((offsets.y>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[4] = (uint8_t)((offsets.y>>8) | 0x02); // Negative
    } else {
      data[4] = (uint8_t)((offsets.y>>8) & 0x01); // Positive
    }
    // Z
    data[5] = (uint8_t)(offsets.z & 0xFF);
    if ((offsets.z>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[6] = (uint8_t)((offsets.z>>8) | 0x02); // Negative
    } else {
      data[6] = (uint8_t)((offsets.z>>8) & 0x01); // Positive
    }

    data[0] = MMA7455_XOFFL;   // Control register
    err |= Do_I2C_MasterTXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 7, data);  // Set offset registers for X Y Z in one pass
  
    // The offset has been set, and everything should be okay.
    // But by setting the offset, the offset of the sensor
    // changes.
    // A second offset calculation has to be done after
    // a short delay, to compensate for that.
    Do_SysTick_Waitms(200);

    err |= Do_MMA7455_readdata(); // get the x,y,z values

    offsets.x += -2 * acc.x;        // add to previous value
    offsets.y += -2 * acc.y;
    offsets.z += -2 * (acc.z-_Gratio);   // Gratio is for 1 'g' for z-axis at 2G sensitivity.

    // X
    data[1] = (uint8_t)(offsets.x & 0xFF);
    if ((offsets.x>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[2] = (uint8_t)((offsets.x>>8) | 0x02); // Negative
    } else {
      data[2] = (uint8_t)((offsets.x>>8) & 0x01); // Positive
    }
    // Y
    data[3] = (uint8_t)(offsets.y & 0xFF);
    if ((offsets.y>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[4] = (uint8_t)((offsets.y>>8) | 0x02); // Negative
    } else {
      data[4] = (uint8_t)((offsets.y>>8) & 0x01); // Positive
    }
    // Z
    data[5] = (uint8_t)(offsets.z & 0xFF);
    if ((offsets.z>>8) & 0x80) {  // Check sign at bit 8 at move it to bit 2 (Making 10 bit coding)
      data[6] = (uint8_t)((offsets.z>>8) | 0x02); // Negative
    } else {
      data[6] = (uint8_t)((offsets.z>>8) & 0x01); // Positive
    }

    data[0] = MMA7455_XOFFL;   // Control register
    err |= Do_I2C_MasterTXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 7, data);  // Set offset registers for X Y Z in one pass

    MMA7455_InitDone = true;
  }

  return err;
}

// Acquire data from the mma7455 chip
bool Do_MMA7455_readdata(void) {

  bool err = false;

  uint8_t data[6];

  err |= Do_I2C_MasterTX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, MMA7455_STATUS);  // Read status
  err |= Do_I2C_MasterRX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, data);
  if ((data[0] & 0x01) && !err) { // Data is ready and no error
    err |= Do_I2C_MasterTX(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, MMA7455_XOUTL);  // Start from X data register
    // Read values for X, Y and Z at once
    err |= Do_I2C_MasterRXBurst(_i2cPort_mma7455, MMA7455_I2C_ADDRESS, 6, data);  // Need to skip first array slot of union

    if (data[1] & 0x02) {  // Contains MSB for X
      data[1] |= 0xFC;     // Stretch bit 9 over other bits
    } else {
      data[1] &= 0x03;
    }
    acc.x = data[0] | (data[1]<<8);

    if (data[3] & 0x02) {  // Contains MSB for Y
      data[3] |= 0xFC;     // Stretch bit 9 over other bits
    } else {
      data[3] &= 0x03;
    }
    acc.y = data[2] | (data[3]<<8);

    if (data[5] & 0x02) {  // Contains MSB for Z
      data[5] |= 0xFC;     // Stretch bit 9 over other bits
    } else {
      data[5] &= 0x03;
    }
    acc.z = data[4] | (data[5]<<8);

  } else {
	  mma7455_error_notready();
    err = true;
    return err;  
  }

  return err;
}


/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t mma7455_info(void) {
  mp_printf(&mp_plat_print, "Control MMA7455 acceleromter sensor. Initialise by mma7455.init(<i2c_port>, <sensitivity>).\n");
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mma7455_info_obj, mma7455_info);

// Initialise MMA7455
STATIC mp_obj_t mma7455_init(mp_obj_t i2cnum_obj, mp_obj_t sensitivity_obj) {
  uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
  uint8_t sensitivity = mp_obj_get_int(sensitivity_obj);
	if (!Do_MMA7455_Init(i2cnum, sensitivity)) {
    mp_printf(&mp_plat_print, "MMA7455 initialised.\n");
  };
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mma7455_init_obj, mma7455_init);

// Print Axis values
STATIC mp_obj_t mma7455_print() {
  if (MMA7455_InitDone == false) {
    mma7455_error_notinitialised();
	} else {
    if (Do_MMA7455_readdata()) {
  	  mma7455_error_notready();
    } else {
      mp_printf(&mp_plat_print, "X: %s%d.%d G \n", (((int16_t)acc.x < 0)?"-":"+"),abs((int16_t)acc.x/_Gratio),abs((int16_t)acc.x%_Gratio)*100/_Gratio);
      mp_printf(&mp_plat_print, "Y: %s%d.%d G \n", (((int16_t)acc.y < 0)?"-":"+"),abs((int16_t)acc.y/_Gratio),abs((int16_t)acc.y%_Gratio)*100/_Gratio);
      mp_printf(&mp_plat_print, "Z: %s%d.%d G \n", (((int16_t)acc.z < 0)?"-":"+"),abs((int16_t)acc.z/_Gratio),abs((int16_t)acc.z%_Gratio)*100/_Gratio);
    }
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mma7455_print_obj, mma7455_print);

// Return acceleration on X
STATIC mp_obj_t mma7455_get_x() {
  if (MMA7455_InitDone == false) {
    mma7455_error_notinitialised();
	} else {
    if (Do_MMA7455_readdata()) {
  	  mma7455_error_notready();
      return mp_const_none;
    }
	}
  return mp_obj_new_int((((int16_t)acc.x < 0)?-1:+1)*(abs(((int16_t)acc.x)/_Gratio)*100+abs((int16_t)acc.x%_Gratio)*100/_Gratio));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mma7455_get_x_obj, mma7455_get_x);

// Return acceleration on Y
STATIC mp_obj_t mma7455_get_y() {
  if (MMA7455_InitDone == false) {
    mma7455_error_notinitialised();
	} else {
    if (Do_MMA7455_readdata()) {
  	  mma7455_error_notready();
      return mp_const_none;
    }
	}
  return mp_obj_new_int((((int16_t)acc.y < 0)?-1:+1)*(abs(((int16_t)acc.y)/_Gratio)*100+abs((int16_t)acc.y%_Gratio)*100/_Gratio));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mma7455_get_y_obj, mma7455_get_y);

// Return acceleration on Z
STATIC mp_obj_t mma7455_get_z() {
  if (MMA7455_InitDone == false) {
    mma7455_error_notinitialised();
	} else {
    if (Do_MMA7455_readdata()) {
  	  mma7455_error_notready();
      return mp_const_none;
    }
	}
  return mp_obj_new_int((((int16_t)acc.z < 0)?-1:+1)*(abs(((int16_t)acc.z)/_Gratio)*100+abs((int16_t)acc.z%_Gratio)*100/_Gratio));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mma7455_get_z_obj, mma7455_get_z);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t mma7455_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_mma7455) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&mma7455_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mma7455_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&mma7455_print_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_x), MP_ROM_PTR(&mma7455_get_x_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_y), MP_ROM_PTR(&mma7455_get_y_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_z), MP_ROM_PTR(&mma7455_get_z_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mma7455_module_globals, mma7455_module_globals_table);

const mp_obj_module_t mma7455_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&mma7455_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mma7455, mma7455_module, MICROPY_MODULE_MMA7455);
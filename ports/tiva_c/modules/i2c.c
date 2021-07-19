/*

	I2C control for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	27 June 2021
	
	Software provided under MIT License

*/
#include "modules/i2c.h"

#include <stdbool.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/hw_memmap.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"
#include "boards/lm4f_pin_map.h"
#include "boards/pins_def.h"
#include "modules/time.h"
#include "modules/time.h"

bool I2C_InitDone[4] = {[0]=false,[1]=false,[2]=false,[3]=false};   // Array with port initialisation status

/*

    Internal C functions (For use in other C modules)   

*/

void i2c_err_notinitialised() {
    // Send generic error
	mp_printf(&mp_plat_print, "I2C port not initialised.\n");
}
void i2c_err_portnotavailable() {
    // Send generic error
    mp_printf(&mp_plat_print, "I2C port not available.\n");
}

// Initialise registers for I2C
void Do_I2C_MasterInit(uint8_t i2cnum, bool i2cbfast) {
    i2cbfast = false;
    switch (i2cnum) {
        case 0:
            // enable I2C0 and GPIOB peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
            ROM_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_I2C0);
            ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
            while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C0)) {};

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
            ROM_GPIOPinConfigure(GPIO_PB3_I2C0SDA);
            ROM_GPIOPinTypeI2C(GPIO_PORTB_BASE, 0x08); // Pins 3 of Port B used for I2C
            ROM_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, 0x04);    // Pin 2 of port B is SCL
		    ROM_GPIOPadConfigSet(GPIO_PORTB_BASE, 0x08, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            ROM_GPIOPadConfigSet(GPIO_PORTB_BASE, 0x04, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

            //Configure and enable I2C port
            ROM_I2CMasterInitExpClk(I2C0_BASE,80000000,i2cbfast);
            ROM_SysCtlDelay(10);    // Wait 30 cycles for the hardware to start

            // Set timeout
            ROM_I2CMasterTimeoutSet(I2C0_BASE,0xfa); // 40ms at 100KHz, 10ms at 400KHz

            ROM_I2CMasterEnable(I2C0_BASE);

            I2C_InitDone[i2cnum] = true;
            break;
        case 1:
            // enable I2C1 and GPIOA peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
            ROM_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_I2C1);
            ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
            while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)) {};

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PA6_I2C1SCL);
            ROM_GPIOPinConfigure(GPIO_PA7_I2C1SDA);
            ROM_GPIOPinTypeI2C(GPIO_PORTA_BASE, 0x80); // Pins 7 of Port A used for I2C
            ROM_GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, 0x40);    // Pin 6 of port A is SCL
		    ROM_GPIOPadConfigSet(GPIO_PORTA_BASE, 0x80, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            ROM_GPIOPadConfigSet(GPIO_PORTA_BASE, 0x40, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

            //Configure and enable I2C port
            ROM_I2CMasterInitExpClk(I2C1_BASE,80000000,i2cbfast);
            ROM_SysCtlDelay(10);    // Wait 30 cycles for the hardware to start

            // Set timeout
            ROM_I2CMasterTimeoutSet(I2C1_BASE,0xfa); // 40ms at 100KHz, 10ms at 400KHz

            ROM_I2CMasterEnable(I2C1_BASE);

            I2C_InitDone[i2cnum] = true;
            break;
        case 2:
            // enable I2C2 and GPIOE peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
            ROM_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_I2C2);
            ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
            while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C2)) {};

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PE4_I2C2SCL);
            ROM_GPIOPinConfigure(GPIO_PE5_I2C2SDA);
            ROM_GPIOPinTypeI2C(GPIO_PORTE_BASE, 0x20); // Pin 5 of Port E used for I2C
            ROM_GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, 0x10);    // Pin 4 of port E is SCL
		    ROM_GPIOPadConfigSet(GPIO_PORTE_BASE, 0x20, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            ROM_GPIOPadConfigSet(GPIO_PORTE_BASE, 0x10, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

            //Configure and enable I2C port
            ROM_I2CMasterInitExpClk(I2C2_BASE,80000000,i2cbfast);
            ROM_SysCtlDelay(10);    // Wait 30 cycles for the hardware to start

            // Set timeout
            ROM_I2CMasterTimeoutSet(I2C2_BASE,0xfa); // 40ms at 100KHz, 10ms at 400KHz

            ROM_I2CMasterEnable(I2C2_BASE);

            I2C_InitDone[i2cnum] = true;
            break;
        case 3:
            // enable I2C3 and GPIOD peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            ROM_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_I2C3);
            ROM_SysCtlPeripheralReset(SYSCTL_PERIPH_I2C3);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3);
            while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_I2C3)) {};

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PD0_I2C3SCL);
            ROM_GPIOPinConfigure(GPIO_PD1_I2C3SDA);
            ROM_GPIOPinTypeI2C(GPIO_PORTD_BASE, 0x02); // Pin 1 of Port E used for I2C
            ROM_GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, 0x01);    // Pin 0 of port E is SCL
		    ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, 0x02, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);
            ROM_GPIOPadConfigSet(GPIO_PORTD_BASE, 0x01, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

            //Configure and enable I2C port
            ROM_I2CMasterInitExpClk(I2C3_BASE,80000000,i2cbfast);
            ROM_SysCtlDelay(10);    // Wait 30 cycles for the hardware to start

            // Set timeout
            ROM_I2CMasterTimeoutSet(I2C3_BASE,0xfa); // 40ms at 100KHz, 10ms at 400KHz

            ROM_I2CMasterEnable(I2C3_BASE);

            I2C_InitDone[i2cnum] = true;
            break;
        default:
            i2c_err_portnotavailable();
            break;
    }
}

// Send data via I2C Single Byte
bool Do_I2C_MasterTX(uint8_t i2cnum, uint8_t slaveaddr, uint8_t word) {
    unsigned long i2c_base = 0;
    bool err = true;
    if (!I2C_InitDone[i2cnum]) {
        i2c_err_notinitialised();
    } else {
        switch (i2cnum) {
        case 0:
            i2c_base = I2C0_BASE;
            break;  
        case 1:
            i2c_base = I2C1_BASE;
            break;
        case 2:
            i2c_base = I2C2_BASE;
            break;
        case 3:
            i2c_base = I2C3_BASE;
            break;
        default:
            i2c_err_portnotavailable();
            break;
        }
        if (i2c_base != 0) {
            // Transmit data
            while(ROM_I2CMasterBusBusy(i2c_base)){};   // Waiting for the bus to free if multiple masters
            ROM_I2CMasterSlaveAddrSet(i2c_base, slaveaddr, false);     // Setting slave address and transmit mode
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
            ROM_I2CMasterDataPut(i2c_base, word);      // Data is readu to be transmitted
            ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_SEND);    // Simple single byte transmission (Not burst)
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
            if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                err = false;
            }
        }
    }
    return err;
}

// Send data via I2C Burts
bool Do_I2C_MasterTXBurst(uint8_t i2cnum, uint8_t slaveaddr, uint8_t bytes, uint8_t *word) {
    bool err = true;
    unsigned long i2c_base = 0;
    uint8_t i=0;

    if (!I2C_InitDone[i2cnum]) {
        i2c_err_notinitialised();
    } else {
        switch (i2cnum) {
        case 0:
            i2c_base = I2C0_BASE;
            break;  
        case 1:
            i2c_base = I2C1_BASE;
            break;
        case 2:
            i2c_base = I2C2_BASE;
            break;
        case 3:
            i2c_base = I2C3_BASE;
            break;
        default:
            i2c_err_portnotavailable();
            break;
            return err;
        }

        if (bytes<2) {
        	mp_printf(&mp_plat_print, "Not enough data.\n");
            return err;
        }

        if (i2c_base != 0) {
            // Transmit data
            while(ROM_I2CMasterBusBusy(i2c_base)){};   // Waiting for the bus to free if multiple masters
            ROM_I2CMasterSlaveAddrSet(i2c_base, slaveaddr, false);     // Setting slave address and transmit mode
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
            ROM_I2CMasterDataPut(i2c_base, word[0]);      // First byte is ready to be transmitted
            ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);    // Burst first byte transmission
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
            if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                err = false;
            } else {
                err = true;                
            }

            // Loop except for the first (already sent) and last (will be sent after) bytes
            for (i=1;i<bytes-1;i++) {
                if(!err) {  // If no error occurred
                    ROM_I2CMasterDataPut(i2c_base, word[i]);      // Data is readu to be transmitted
                    ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_CONT);    // Send next
                    while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
                    if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                        err = false;
                    } else {
                        err = true;                
                    }
                } else {
                    ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);    // Stop with error
                    break;
                }
            }
            if (!err) {     // If no error occured, then we send the last byte, else the transmission was already stopped
                ROM_I2CMasterDataPut(i2c_base, word[(bytes-1)]);      // Data is ready to be transmitted
                ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);    // Send last
                while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
                if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                    err = false;
                }
            }
        }
    }
    return err;
}


// Receive data via I2C 
bool Do_I2C_MasterRX(uint8_t i2cnum, uint8_t slaveaddr, uint8_t *word) {
    bool err = true;
    unsigned long i2c_base = 0;

    if (!I2C_InitDone[i2cnum]) {
        i2c_err_notinitialised();
    } else {
        switch (i2cnum) {
        case 0:
            i2c_base = I2C0_BASE;
            break;  
        case 1:
            i2c_base = I2C1_BASE;
            break;
        case 2:
            i2c_base = I2C2_BASE;
            break;
        case 3:
            i2c_base = I2C3_BASE;
            break;
        default:
            i2c_err_portnotavailable();
            break;
        }
        if (i2c_base != 0) {
            // Receive data
            while(ROM_I2CMasterBusBusy(i2c_base)){};   // Waiting for the bus to free if multiple masters
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the I2C to be available
            ROM_I2CMasterSlaveAddrSet(i2c_base, slaveaddr, true);     // Setting slave address and receive mode
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the transmission to end
            ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_RECEIVE);    // Simple single byte transmission (Not burst)
            while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the transmission to end
            if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                word[0] = (uint8_t)ROM_I2CMasterDataGet(i2c_base);
                err = false;
            }
        }
    }
    return err;
}


// Receive data via I2C Burst mode
bool Do_I2C_MasterRXBurst(uint8_t i2cnum, uint8_t slaveaddr, uint8_t bytes, uint8_t *word) {
    bool err = true;
    unsigned long i2c_base = 0;
    uint8_t i=0;

    if (!I2C_InitDone[i2cnum]) {
        i2c_err_notinitialised();
    } else {
        switch (i2cnum) {
        case 0:
            i2c_base = I2C0_BASE;
            break;  
        case 1:
            i2c_base = I2C1_BASE;
            break;
        case 2:
            i2c_base = I2C2_BASE;
            break;
        case 3:
            i2c_base = I2C3_BASE;
            break;
        default:
            i2c_err_portnotavailable();
            return err;
            break;
        }
        if (bytes<2) {
        	mp_printf(&mp_plat_print, "Not enough data.\n");
            return err;
        }
        if (i2c_base != 0) {
            // Receive data
            bytes--;    // As the count goes from 0 to X, we need to decrease the counter's limit

            while(ROM_I2CMasterBusBusy(i2c_base)){};   // Waiting for the bus to free if multiple masters
            ROM_I2CMasterSlaveAddrSet(i2c_base, slaveaddr, true);     // Setting slave address and receive mode
            while(ROM_I2CMasterBusy(i2c_base)){};       // Waiting for the I2C to be available
            ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_START);    // Start first byte receiving
            // Loop except for the last byte that will be received after
            for (i=0;i<=bytes;i++) {
                while(ROM_I2CMasterBusy(i2c_base)){};   // Waiting for the transmission to end
                if(ROM_I2CMasterErr(i2c_base)==I2C_MASTER_ERR_NONE) {  // If no error occurred, return false
                    word[i] = (uint8_t)ROM_I2CMasterDataGet(i2c_base);
                    err = false;
                } else {
                    ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);    // Stop with error
                    err = true;
                    break;
                }
                if (i != bytes) {
                    ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);    // Next byte receiving
                } else {
                    ROM_I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);    // Stop receiving
                }
            }
        }
    }
    return err;
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t i2c_info(void) {
    mp_printf(&mp_plat_print, "Initialise with i2c.init()\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(i2c_info_obj, i2c_info);

// Initialise I2C port (0 to 3)
STATIC mp_obj_t i2c_init(mp_obj_t i2cnum_obj, mp_obj_t speed_obj) {
    uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
    bool speed = mp_obj_get_int(speed_obj);

    Do_I2C_MasterInit(i2cnum,speed);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(i2c_init_obj, i2c_init);

// Send data to I2C port
STATIC mp_obj_t i2c_write(mp_obj_t i2cnum_obj, mp_obj_t slave_obj, mp_obj_t word_obj) {
    uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
    uint8_t slave = mp_obj_get_int(slave_obj);
    uint8_t word = mp_obj_get_int(word_obj);

    if (Do_I2C_MasterTX(i2cnum, slave, word)) {
        mp_printf(&mp_plat_print, "Transmission error.\n");
    };

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(i2c_write_obj, i2c_write);

// Read data from I2C Port
STATIC mp_obj_t i2c_read(mp_obj_t i2cnum_obj, mp_obj_t slave_obj) {
    uint8_t i2cnum = mp_obj_get_int(i2cnum_obj);
    uint8_t slave = mp_obj_get_int(slave_obj);
    uint8_t word;

    if (Do_I2C_MasterRX(i2cnum, slave, &word)) {
        mp_printf(&mp_plat_print, "Receiving error.\n");
    };

    return mp_obj_new_int(word);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(i2c_read_obj, i2c_read);

// Request data from I2C Port
STATIC mp_obj_t i2c_request(size_t n_args, const mp_obj_t *args) {
    uint8_t i2cnum = mp_obj_get_int(args[0]);
    uint8_t slave = mp_obj_get_int(args[1]);
    uint8_t word = mp_obj_get_int(args[2]);
    uint8_t bytes = mp_obj_get_int(args[3]);
    uint8_t word_rx[4]  = {[0]=0,[1]=0,[2]=0,[3]=0};
    uint32_t word_result = 0;

    if (bytes > 4) {
        mp_printf(&mp_plat_print, "4 bytes max.\n");
        return mp_const_none;
    }

    if (Do_I2C_MasterTX(i2cnum, slave, word)) {
        mp_printf(&mp_plat_print, "Transmission error.\n");
    } else {
        if (bytes == 1) {
            if (Do_I2C_MasterRX(i2cnum, slave, word_rx)) {
                mp_printf(&mp_plat_print, "Receiving error.\n");
            };
        } else {
            if (Do_I2C_MasterRXBurst(i2cnum, slave, bytes, word_rx)) {
                mp_printf(&mp_plat_print, "Receiving error.\n");
            };
        }
    }
    for (uint8_t i=0; i<bytes; i++) {
        word_result = word_result | ((word_rx[(bytes-i-1)])<<(8*i));
    }
    return mp_obj_new_int(word_result);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(i2c_request_obj, 4, 4, i2c_request);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t i2c_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_i2c) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&i2c_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&i2c_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&i2c_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_request), MP_ROM_PTR(&i2c_request_obj) },
};
STATIC MP_DEFINE_CONST_DICT(i2c_module_globals, i2c_module_globals_table);

const mp_obj_module_t i2c_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&i2c_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_i2c, i2c_module, MICROPY_MODULE_I2C);
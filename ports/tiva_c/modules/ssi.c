/*

	Basic SSI control for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	5 June 2021
	
	Software provided under MIT License

*/
#include "modules/ssi.h"

#include <stdbool.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"
#include "boards/lm4f_pin_map.h"
#include "boards/hw_memmap.h"
#include "modules/time.h"

int8_t SSI_InitDone = -1;   // Currently this only supports one port to be initialised and used at time

/*

    Internal C functions (For use in other C modules)   

*/

void ssi_err_notinitialised() {
    // Send generic error
	mp_printf(&mp_plat_print, "SSI port not initialised.\n");
}
void ssi_err_portnotavailable() {
    // Send generic error
    mp_printf(&mp_plat_print, "SSI port not available.\n");
}

// Initialise registers to SSI
void Do_SSI_Init(uint8_t ssinum) {
    uint32_t initialData = 0;
    switch (ssinum)
    {
    case 0:
        // enable SSI0 and GPIOD peripherals
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
        ROM_SSIDisable(SSI0_BASE);

        // Configure GPIO pins for special functions
        ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
        ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
        ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
        ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, 0x2c); // Pins 2 3 5 of Port A used for SSI

        //Configure and enable SSI port
        // Use internal 16Mhz RC oscillator as SSI clock source
        ROM_SSIClockSourceSet(SSI0_BASE, SSI_CLOCK_PIOSC);
        ROM_SSIConfigSetExpClk(SSI0_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
                SSI_MODE_MASTER, 8000000, 8);
        ROM_SSIEnable(SSI0_BASE);

        //clear out any initial data that might be present in the RX FIFO
        while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &initialData));
        
        // Load default configuration parameters
        ROM_SSIDataPut(SSI0_BASE, 0x00);

        SSI_InitDone = ssinum;
        break;
    case 1:
        // enable SSI1 and GPIOD peripherals
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

        // Configure GPIO pins for special functions
        ROM_GPIOPinConfigure(GPIO_PF2_SSI1CLK);
        ROM_GPIOPinConfigure(GPIO_PF1_SSI1TX);
        ROM_GPIOPinConfigure(GPIO_PF3_SSI1FSS);
        ROM_GPIOPinTypeSSI(GPIO_PORTF_BASE, 0x0e); // Pins 1 2 3 of Port F used for SSI

        //Configure and enable SSI port
        // Use internal 16Mhz RC oscillator as SSI clock source
        ROM_SSIClockSourceSet(SSI1_BASE, SSI_CLOCK_PIOSC);
        ROM_SSIConfigSetExpClk(SSI1_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
                SSI_MODE_MASTER, 8000000, 8);
        ROM_SSIEnable(SSI1_BASE);

        //clear out any initial data that might be present in the RX FIFO
        while(ROM_SSIDataGetNonBlocking(SSI1_BASE, &initialData));

        // Load default configuration parameters
        ROM_SSIDataPut(SSI1_BASE, 0x00);

        SSI_InitDone = ssinum;
        break;
    case 2:
        // enable SSI2 and GPIOD peripherals
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

        // Configure GPIO pins for special functions
        ROM_GPIOPinConfigure(GPIO_PB4_SSI2CLK);
        ROM_GPIOPinConfigure(GPIO_PB7_SSI2TX);
        ROM_GPIOPinConfigure(GPIO_PB5_SSI2FSS);
        ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, 0xB0);  // Pins 4 5 7 of port B used for SSI

        //Configure and enable SSI port
        // Use internal 16Mhz RC oscillator as SSI clock source
        ROM_SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_PIOSC);
        ROM_SSIConfigSetExpClk(SSI2_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
                SSI_MODE_MASTER, 8000000, 8);
        ROM_SSIEnable(SSI2_BASE);

        //clear out any initial data that might be present in the RX FIFO
        while(ROM_SSIDataGetNonBlocking(SSI2_BASE, &initialData));

        // Load default configuration parameters
        ROM_SSIDataPut(SSI2_BASE, 0x00);

        SSI_InitDone = ssinum;
        break;
    case 3:
        // enable SSI3 and GPIOD peripherals
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
        ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

        // Configure GPIO pins for special functions
        ROM_GPIOPinConfigure(GPIO_PD0_SSI3CLK);
        ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);
        ROM_GPIOPinConfigure(GPIO_PD1_SSI3FSS);
        ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, 0xB);   // Pins 0 1 3 of Port D used for SSI

        //Configure and enable SSI port
        // Use internal 16Mhz RC oscillator as SSI clock source
        ROM_SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_PIOSC);
        ROM_SSIConfigSetExpClk(SSI2_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
                SSI_MODE_MASTER, 8000000, 8);
        ROM_SSIEnable(SSI2_BASE);

        //clear out any initial data that might be present in the RX FIFO
        while(ROM_SSIDataGetNonBlocking(SSI2_BASE, &initialData));

        // Load default configuration parameters
        ROM_SSIDataPut(SSI2_BASE, 0x00);

        SSI_InitDone = ssinum;
        break;
    default:
        ssi_err_portnotavailable();
        break;
    }
}

// Send data via SSI 
void Do_SSI_TX(uint8_t ssinum, uint32_t word) {
    if (SSI_InitDone != ssinum) {
        ssi_err_portnotavailable();
    } else {
        switch (ssinum) {
        case 0:
            ROM_SSIDataPut(SSI0_BASE, word);
            break;  
        case 1:
            ROM_SSIDataPut(SSI1_BASE, word);
            break;
        case 2:
            ROM_SSIDataPut(SSI2_BASE, word);
            break;
        case 3:
            ROM_SSIDataPut(SSI3_BASE, word);
            break;
        default:
            ssi_err_portnotavailable();
            break;
        }
    }
}

// Send data via SSI using the FIFO (non blocking)
void Do_SSI_TX_FIFO(uint8_t ssinum, uint32_t word) {
    if (SSI_InitDone != ssinum) {
        ssi_err_portnotavailable();
    } else {
        switch (ssinum) {
        case 0:
            while(ROM_SSIDataPutNonBlocking(SSI0_BASE, word)==0);  // Keep retrying until data goes into the FIFO
            break;  
        case 1:
            while(ROM_SSIDataPutNonBlocking(SSI1_BASE, word)==0);  // Keep retrying until data goes into the FIFO
            break;
        case 2:
            while(ROM_SSIDataPutNonBlocking(SSI2_BASE, word)==0);  // Keep retrying until data goes into the FIFO
            break;
        case 3:
            while(ROM_SSIDataPutNonBlocking(SSI3_BASE, word)==0);  // Keep retrying until data goes into the FIFO
            break;
        default:
            ssi_err_portnotavailable();
            break;
        }
    }
}

// Check if SSI port is still sending data
bool Do_SSI_Busy(uint8_t ssinum) {
    bool value = false;
    if (SSI_InitDone != ssinum) {
        ssi_err_notinitialised();
    } else {
        switch (ssinum) {
        case 0:
            value = ROM_SSIBusy(SSI0_BASE);
            break;  
        case 1:
            value = ROM_SSIBusy(SSI1_BASE);
            break;
        case 2:
            value = ROM_SSIBusy(SSI2_BASE);
            break;
        case 3:
            value = ROM_SSIBusy(SSI3_BASE);
            break;
        default:
            ssi_err_portnotavailable();
            break;
        }
    }
    return value;
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t ssi_info(void) {
    mp_printf(&mp_plat_print, "Initialise with ssi.init()\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ssi_info_obj, ssi_info);

// Initialise SSI port (0 to 3)
STATIC mp_obj_t ssi_init(mp_obj_t ssinum_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);

    Do_SSI_Init(ssinum);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ssi_init_obj, ssi_init);

// Send data to SSI port
STATIC mp_obj_t ssi_tx(mp_obj_t ssinum_obj, mp_obj_t word_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint32_t word = mp_obj_get_int(word_obj);

    Do_SSI_TX(ssinum, word);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ssi_tx_obj, ssi_tx);

// Send data to SSI port using FIFO
STATIC mp_obj_t ssi_tx_fifo(mp_obj_t ssinum_obj, mp_obj_t word_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint32_t word = mp_obj_get_int(word_obj);

    Do_SSI_TX_FIFO(ssinum, word);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ssi_tx_fifo_obj, ssi_tx_fifo);

// Check if SSI port is busy
STATIC mp_obj_t ssi_busy(mp_obj_t ssinum_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    bool is_busy;

    is_busy = Do_SSI_Busy(ssinum);

    return mp_obj_new_bool(is_busy);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ssi_busy_obj, ssi_busy);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t ssi_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ssi) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&ssi_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&ssi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_tx), MP_ROM_PTR(&ssi_tx_obj) },
    { MP_ROM_QSTR(MP_QSTR_tx_fifo), MP_ROM_PTR(&ssi_tx_fifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_busy), MP_ROM_PTR(&ssi_busy_obj) },
};
STATIC MP_DEFINE_CONST_DICT(ssi_module_globals, ssi_module_globals_table);

const mp_obj_module_t ssi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&ssi_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ssi, ssi_module, MICROPY_MODULE_SSI);
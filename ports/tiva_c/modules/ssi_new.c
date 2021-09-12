/*

	SSI control for Texas Instruments LM4F Microcontrollers
	
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

uint16_t _ssicfg[4] = {[0]=0,[1]=0,[2]=0,[3]=0}; // Array with port initialisation and config value
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

// Initialise registers for SSI
void Do_SSI_Init(uint8_t ssinum, uint16_t ssicfg, bool sdcard) {
    uint32_t initialData = 0;

    // Default settings
    uint32_t ssiproto = SSI_FRF_MOTO_MODE_0;
    uint32_t ssimode = SSI_MODE_MASTER;
    uint32_t ssibitrate = 2000000;
    uint32_t ssidata = 8;
    uint32_t ssiclock = SSI_CLOCK_PIOSC;
    uint32_t clkvalue = 16000000;

    // Select Clock source
    switch (ssicfg/10000) {
        case 1:
            ssiclock = SSI_CLOCK_PIOSC;
            clkvalue = 16000000;
            break;
        case 2:
            ssiclock = SSI_CLOCK_SYSTEM;
            clkvalue = 80000000;
            break;
    }

    // Select Master / Slave
    switch (ssicfg/1000) {
        case 0:
            ssimode = SSI_MODE_MASTER;
            break;
        case 1:
            ssimode = SSI_MODE_SLAVE;
            break;
        case 2:
            ssimode = SSI_MODE_SLAVE_OD;
            break;
    }

    // Select Frame Format
    switch (ssicfg/100) {
        case 0:
            ssiproto = SSI_FRF_MOTO_MODE_0;
            break;
        case 1:
            ssiproto = SSI_FRF_MOTO_MODE_1;
            break;
        case 2:
            ssiproto = SSI_FRF_MOTO_MODE_2;
            break;
        case 3:
            ssiproto = SSI_FRF_MOTO_MODE_3;
            break;
        case 4:
            ssiproto = SSI_FRF_NMW;
            break;
        case 5:
            ssiproto = SSI_FRF_TI;
            break;
    }

    // Select Bitrate
    switch (ssicfg/10) {
        case 0:
            ssibitrate = 250000;
            break;
        case 1:
            ssibitrate = 500000;
            break;
        case 2:
            ssibitrate = 1000000;
            break;
        case 3:
            ssibitrate = 2000000;
            break;
        case 4:
            ssibitrate = 4000000;
            break;
        case 5:
            ssibitrate = 8000000;
            break;
        case 6:
            if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 12000000;
            break;
        case 7:
            if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 16000000;
            break;
        case 8:
            if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 20000000;
            break;
        case 9:
            if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 25000000;
            break;
    }

    // Select Data Segment Size (length)
    switch (ssicfg%10) {
        case 0:
            ssidata = 4;
            break;
        case 1:
            ssidata = 8;
            break;
        case 2:
            ssidata = 16;
            break;
    }

    switch (ssinum) {
        case 0:
            // enable SSI0 and GPIOD peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
            ROM_SSIDisable(SSI0_BASE);

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
            ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
            if (!sdcard) {
                ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
                ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
                ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, 0x3c); // Pins 2 3 4 5 of Port A used for SSI
            } else {
                ROM_GPIOPinTypeSSI(GPIO_PORTF_BASE, 0x05); // Pins 0 2 of Port F used for SSI, CS PF3 and MOSI PF1 is freed
            };

            //Configure and enable SSI port
            // Use internal 16Mhz RC oscillator as SSI clock source
            ROM_SSIClockSourceSet(SSI0_BASE, ssiclock);
            ROM_SSIConfigSetExpClk(SSI0_BASE, clkvalue, ssiproto,
                    ssimode, ssibitrate, ssidata);
            ROM_SSIEnable(SSI0_BASE);

            //clear out any initial data that might be present in the RX FIFO
            while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &initialData));
            
            // Load default configuration parameters
            ROM_SSIDataPut(SSI0_BASE, 0x00);

            _ssicfg[ssinum] = ssicfg;

            break;
        case 1:
            // enable SSI1 and GPIOD peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
            ROM_SSIDisable(SSI1_BASE);

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PF2_SSI1CLK);
            ROM_GPIOPinConfigure(GPIO_PF0_SSI1RX);
            if (!sdcard) {
                ROM_GPIOPinConfigure(GPIO_PF3_SSI1FSS);
                ROM_GPIOPinConfigure(GPIO_PF1_SSI1TX);
                ROM_GPIOPinTypeSSI(GPIO_PORTF_BASE, 0x0f); // Pins 0 1 2 3 of Port F used for SSI
            } else {
                ROM_GPIOPinTypeSSI(GPIO_PORTF_BASE, 0x05); // Pins 0 2 of Port F used for SSI, CS PF3 and MOSI PF1 is freed
            }

            //Configure and enable SSI port
            // Use internal 16Mhz RC oscillator as SSI clock source
            ROM_SSIClockSourceSet(SSI1_BASE, ssiclock);
            ROM_SSIConfigSetExpClk(SSI1_BASE, clkvalue, ssiproto,
                    ssimode, ssibitrate, ssidata);
            ROM_SSIEnable(SSI1_BASE);

            //clear out any initial data that might be present in the RX FIFO
            while(ROM_SSIDataGetNonBlocking(SSI1_BASE, &initialData));

            // Load default configuration parameters
            ROM_SSIDataPut(SSI1_BASE, 0x00);

            _ssicfg[ssinum] = ssicfg;

            break;
        case 2:
            // enable SSI2 and GPIOD peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
            ROM_SSIDisable(SSI2_BASE);

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PB4_SSI2CLK);
            ROM_GPIOPinConfigure(GPIO_PB6_SSI2RX);
            if (!sdcard) {
                ROM_GPIOPinConfigure(GPIO_PB7_SSI2TX);
                ROM_GPIOPinConfigure(GPIO_PB5_SSI2FSS);
                ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, 0xF0);  // Pins 4 5 6 7 of port B used for SSI
            } else {
                ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, 0x50);  // Pins 4 6 of port B used for SSI, CS PB5 and MOSI PB7 is freed
            }

            //Configure and enable SSI port
            // Use internal 16Mhz RC oscillator as SSI clock source
            ROM_SSIClockSourceSet(SSI2_BASE, ssiclock);
            ROM_SSIConfigSetExpClk(SSI2_BASE, clkvalue, ssiproto,
                    ssimode, ssibitrate, ssidata);
            ROM_SSIEnable(SSI2_BASE);

            //clear out any initial data that might be present in the RX FIFO
            while(ROM_SSIDataGetNonBlocking(SSI2_BASE, &initialData));

            // Load default configuration parameters
            ROM_SSIDataPut(SSI2_BASE, 0x00);

            _ssicfg[ssinum] = ssicfg;

            break;
        case 3:
            // enable SSI3 and GPIOD peripherals
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
            ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
            ROM_SSIDisable(SSI3_BASE);

            // Configure GPIO pins for special functions
            ROM_GPIOPinConfigure(GPIO_PD0_SSI3CLK);
            ROM_GPIOPinConfigure(GPIO_PD2_SSI3RX);
            if (!sdcard) {
                ROM_GPIOPinConfigure(GPIO_PD3_SSI3TX);
                ROM_GPIOPinConfigure(GPIO_PD1_SSI3FSS);
                ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, 0x0f);   // Pins 0 2 1 3 of Port D used for SSI
            } else {
                ROM_GPIOPinTypeSSI(GPIO_PORTD_BASE, 0x05);   // Pins 0 2 of Port D used for SSI, CS PD1 and MOSI PD3 is freed
            }


            //Configure and enable SSI port
            // Use internal 16Mhz RC oscillator as SSI clock source
            ROM_SSIClockSourceSet(SSI3_BASE, ssiclock);
            ROM_SSIConfigSetExpClk(SSI3_BASE, clkvalue, ssiproto,
                    ssimode, ssibitrate, ssidata);
            ROM_SSIEnable(SSI3_BASE);

            //clear out any initial data that might be present in the RX FIFO
            while(ROM_SSIDataGetNonBlocking(SSI3_BASE, &initialData));

            // Load default configuration parameters
            ROM_SSIDataPut(SSI3_BASE, 0x00);

            _ssicfg[ssinum] = ssicfg;

            break;
        default:
            ssi_err_portnotavailable();
            break;
    }
}

// Perform fast configuration change without complete reinitisalisation
uint16_t Do_SSI_FastConfig(uint8_t ssinum, uint16_t ssicfg) {
    uint16_t oldconfig = 0;

    if (_ssicfg[ssinum]==0) {
        ssi_err_notinitialised();
    } else {
        uint32_t initialData = 0;

        // Default settings
        uint32_t ssiproto = SSI_FRF_MOTO_MODE_0;
        uint32_t ssimode = SSI_MODE_MASTER;
        uint32_t ssibitrate = 2000000;
        uint32_t ssidata = 8;
        uint32_t ssiclock = SSI_CLOCK_PIOSC;
        uint32_t clkvalue = 16000000;

        // Select Clock source
        switch (ssicfg/10000) {
            case 1:
                ssiclock = SSI_CLOCK_PIOSC;
                clkvalue = 16000000;
                break;
            case 2:
                ssiclock = SSI_CLOCK_SYSTEM;
                clkvalue = 80000000;
                break;
        }

        // Select Master / Slave
        switch (ssicfg/1000) {
            case 0:
                ssimode = SSI_MODE_MASTER;
                break;
            case 1:
                ssimode = SSI_MODE_SLAVE;
                break;
            case 2:
                ssimode = SSI_MODE_SLAVE_OD;
                break;
        }

        // Select Frame Format
        switch (ssicfg/100) {
            case 0:
                ssiproto = SSI_FRF_MOTO_MODE_0;
                break;
            case 1:
                ssiproto = SSI_FRF_MOTO_MODE_1;
                break;
            case 2:
                ssiproto = SSI_FRF_MOTO_MODE_2;
                break;
            case 3:
                ssiproto = SSI_FRF_MOTO_MODE_3;
                break;
            case 4:
                ssiproto = SSI_FRF_NMW;
                break;
            case 5:
                ssiproto = SSI_FRF_TI;
                break;
        }

        // Select Bitrate
        switch (ssicfg/10) {
            case 0:
                ssibitrate = 250000;
                break;
            case 1:
                ssibitrate = 500000;
                break;
            case 2:
                ssibitrate = 1000000;
                break;
            case 3:
                ssibitrate = 2000000;
                break;
            case 4:
                ssibitrate = 4000000;
                break;
            case 5:
                ssibitrate = 8000000;
                break;
            case 6:
                if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 12000000;
                break;
            case 7:
                if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 16000000;
                break;
            case 8:
                if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 20000000;
                break;
            case 9:
                if (ssiclock==SSI_CLOCK_SYSTEM) ssibitrate = 25000000;
                break;
        }

        // Select Data Segment Size (length)
        switch (ssicfg%10) {
            case 0:
                ssidata = 4;
                break;
            case 1:
                ssidata = 8;
                break;
            case 2:
                ssidata = 16;
                break;
        }

        switch (ssinum) {
            case 0:
                // Disable SSI peripherals
                ROM_SSIDisable(SSI0_BASE);

                //Configure and enable SSI port
                // Use internal 16Mhz RC oscillator as SSI clock source
                ROM_SSIClockSourceSet(SSI0_BASE, ssiclock);
                ROM_SSIConfigSetExpClk(SSI0_BASE, clkvalue, ssiproto,
                        ssimode, ssibitrate, ssidata);
                ROM_SSIEnable(SSI0_BASE);

                //clear out any initial data that might be present in the RX FIFO
                while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &initialData));
                
                // Load default configuration parameters
                ROM_SSIDataPut(SSI0_BASE, 0x00);

                oldconfig = _ssicfg[ssinum];
                _ssicfg[ssinum] = ssicfg;

                break;
            case 1:
                // Disable SSI peripherals
                ROM_SSIDisable(SSI1_BASE);

                //Configure and enable SSI port
                // Use internal 16Mhz RC oscillator as SSI clock source
                ROM_SSIClockSourceSet(SSI1_BASE, ssiclock);
                ROM_SSIConfigSetExpClk(SSI1_BASE, clkvalue, ssiproto,
                        ssimode, ssibitrate, ssidata);
                ROM_SSIEnable(SSI1_BASE);

                //clear out any initial data that might be present in the RX FIFO
                while(ROM_SSIDataGetNonBlocking(SSI1_BASE, &initialData));

                // Load default configuration parameters
                ROM_SSIDataPut(SSI1_BASE, 0x00);

                oldconfig = _ssicfg[ssinum];
                _ssicfg[ssinum] = ssicfg;

                break;
            case 2:
                // Disable SSI peripherals
                ROM_SSIDisable(SSI2_BASE);

                //Configure and enable SSI port
                // Use internal 16Mhz RC oscillator as SSI clock source
                ROM_SSIClockSourceSet(SSI2_BASE, ssiclock);
                ROM_SSIConfigSetExpClk(SSI2_BASE, clkvalue, ssiproto,
                        ssimode, ssibitrate, ssidata);
                ROM_SSIEnable(SSI2_BASE);

                //clear out any initial data that might be present in the RX FIFO
                while(ROM_SSIDataGetNonBlocking(SSI2_BASE, &initialData));

                // Load default configuration parameters
                ROM_SSIDataPut(SSI2_BASE, 0x00);

                oldconfig = _ssicfg[ssinum];
                _ssicfg[ssinum] = ssicfg;

                break;
            case 3:
                // Disable SSI peripherals
                ROM_SSIDisable(SSI3_BASE);

                //Configure and enable SSI port
                // Use internal 16Mhz RC oscillator as SSI clock source
                ROM_SSIClockSourceSet(SSI3_BASE, ssiclock);
                ROM_SSIConfigSetExpClk(SSI3_BASE, clkvalue, ssiproto,
                        ssimode, ssibitrate, ssidata);
                ROM_SSIEnable(SSI3_BASE);

                //clear out any initial data that might be present in the RX FIFO
                while(ROM_SSIDataGetNonBlocking(SSI3_BASE, &initialData));

                // Load default configuration parameters
                ROM_SSIDataPut(SSI3_BASE, 0x00);

                oldconfig = _ssicfg[ssinum];
                _ssicfg[ssinum] = ssicfg;

                break;
            default:
                ssi_err_portnotavailable();
                break;
        }
    }
    return oldconfig;
}


// Disable SSI
void Do_SSI_Disable(uint8_t ssinum) {
    if (_ssicfg[ssinum]==0) {
        ssi_err_notinitialised();
    } else {
        switch (ssinum) {
            case 0:
                ROM_SSIDisable(SSI0_BASE);
                break;
            case 1:
                ROM_SSIDisable(SSI1_BASE);
                break;
            case 2:
                ROM_SSIDisable(SSI2_BASE);
                break;
            case 3:
                ROM_SSIDisable(SSI3_BASE);
                break;
            default:
                ssi_err_portnotavailable();
                break;
        }
    }
}

// Enable SSI
void Do_SSI_Enable(uint8_t ssinum) {
    if (_ssicfg[ssinum]==0) {
        ssi_err_notinitialised();
    } else {
        switch (ssinum) {
            case 0:
                ROM_SSIEnable(SSI0_BASE);
                break;
            case 1:
                ROM_SSIEnable(SSI1_BASE);
                break;
            case 2:
                ROM_SSIEnable(SSI2_BASE);
                break;
            case 3:
                ROM_SSIEnable(SSI3_BASE);
                break;
            default:
                ssi_err_portnotavailable();
                break;
        }
    }
}

// Send data via SSI 
void Do_SSI_TX(uint8_t ssinum, uint32_t word) {
    if (_ssicfg[ssinum]==0) {
        ssi_err_notinitialised();
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
    if (_ssicfg[ssinum]==0) {
        ssi_err_notinitialised();
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

// Send 2 words data via SSI using the FIFO (non blocking)
void Do_SSI_TX16_FIFO(uint8_t ssinum, uint16_t word, bool islsb) {
    union {
            uint16_t val;
            struct {
                    uint8_t lsb;    // ARM is little endian
                    uint8_t msb;
            };
    } in;
    in.val = word & 0xFFFF;

    if(islsb) {
        //LSBFIRST
        Do_SSI_TX_FIFO(ssinum,(uint32_t)in.lsb);
        Do_SSI_TX_FIFO(ssinum,(uint32_t)in.msb);
    } else {
        //MSBFIRST
        Do_SSI_TX_FIFO(ssinum,(uint32_t)in.msb);
        Do_SSI_TX_FIFO(ssinum,(uint32_t)in.lsb);
    }
}

// Send 2 words data via SSI (blocking)
void Do_SSI_TX16(uint8_t ssinum, uint16_t word, bool islsb) {
    union {
            uint16_t val;
            struct {
                    uint8_t lsb;    // ARM is little endian
                    uint8_t msb;
            };
    } in;
    in.val = word & 0xFFFF;

    if(islsb) {
        //LSBFIRST
        Do_SSI_TX(ssinum,(uint32_t)in.lsb);
        Do_SSI_TX(ssinum,(uint32_t)in.msb);
    } else {
        //MSBFIRST
        Do_SSI_TX(ssinum,(uint32_t)in.msb);
        Do_SSI_TX(ssinum,(uint32_t)in.lsb);
    }
}

// Check if SSI port is still sending data
bool Do_SSI_Busy(uint8_t ssinum) {
    bool value = false;
    if (_ssicfg[ssinum]==0) {
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

// Receive data via SSI 
uint8_t Do_SSI_RX(uint8_t ssinum, uint32_t *word) {
    uint8_t length = 0;

    if (_ssicfg[ssinum]==0) {
        ssi_err_portnotavailable();
    } else {
        switch (ssinum) {
        case 0:
            ROM_SSIDataGetNonBlocking(SSI0_BASE, word);
            break;  
        case 1:
            ROM_SSIDataGetNonBlocking(SSI1_BASE, word);
            break;
        case 2:
            ROM_SSIDataGetNonBlocking(SSI2_BASE, word);
            break;
        case 3:
            ROM_SSIDataGetNonBlocking(SSI3_BASE, word);
            break;
        default:
            ssi_err_portnotavailable();
            break;
        }
    }
    return length;
}

// Receive data via SSI 
void Do_SSI_RX_Blocking(uint8_t ssinum, uint32_t *word) {
    if (_ssicfg[ssinum]==0) {
        ssi_err_portnotavailable();
    } else {
        switch (ssinum) {
        case 0:
            ROM_SSIDataGet(SSI0_BASE, word);
            break;  
        case 1:
            ROM_SSIDataGet(SSI1_BASE, word);
            break;
        case 2:
            ROM_SSIDataGet(SSI2_BASE, word);
            break;
        case 3:
            ROM_SSIDataGet(SSI3_BASE, word);
            break;
        default:
            ssi_err_portnotavailable();
            break;
        }
    }
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t ssi_info(void) {
    mp_printf(&mp_plat_print, "Initialise with ssi.init(<ssiport>,<config>)\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(ssi_info_obj, ssi_info);

// Initialise SSI port (0 to 3)
STATIC mp_obj_t ssi_init(mp_obj_t ssinum_obj, mp_obj_t ssicfg_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint8_t ssicfg = mp_obj_get_int(ssicfg_obj);

    Do_SSI_Init(ssinum,ssicfg,false);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ssi_init_obj, ssi_init);

// Send data to SSI port
STATIC mp_obj_t ssi_write(mp_obj_t ssinum_obj, mp_obj_t word_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint32_t word = mp_obj_get_int(word_obj);

    Do_SSI_TX(ssinum, word);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ssi_write_obj, ssi_write);

// Send data to SSI port using FIFO
STATIC mp_obj_t ssi_write_fifo(mp_obj_t ssinum_obj, mp_obj_t word_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint32_t word = mp_obj_get_int(word_obj);

    Do_SSI_TX_FIFO(ssinum, word);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(ssi_write_fifo_obj, ssi_write_fifo);

// Check if SSI port is busy
STATIC mp_obj_t ssi_isbusy(mp_obj_t ssinum_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    bool is_busy;

    is_busy = Do_SSI_Busy(ssinum);

    return mp_obj_new_bool(is_busy);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ssi_isbusy_obj, ssi_isbusy);

// Read data from SSI Port
STATIC mp_obj_t ssi_read(mp_obj_t ssinum_obj) {
    uint8_t ssinum = mp_obj_get_int(ssinum_obj);
    uint32_t word;

    Do_SSI_RX_Blocking(ssinum, &word);

    return mp_obj_new_int(word);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ssi_read_obj, ssi_read);


// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t ssi_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_ssi) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&ssi_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&ssi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&ssi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_fifo), MP_ROM_PTR(&ssi_write_fifo_obj) },
    { MP_ROM_QSTR(MP_QSTR_isbusy), MP_ROM_PTR(&ssi_isbusy_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&ssi_read_obj) },
};
STATIC MP_DEFINE_CONST_DICT(ssi_module_globals, ssi_module_globals_table);

const mp_obj_module_t ssi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&ssi_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_ssi, ssi_module, MICROPY_MODULE_SSI);
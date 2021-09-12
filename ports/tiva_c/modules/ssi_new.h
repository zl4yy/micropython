/*

    Headers
	SSI control for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	5 June 2021
	
	Software provided under MIT License

*/
#ifndef SSI_H_
#define SSI_H_

#include <stdbool.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"


//*****************************************************************************
//
// Values that can be passed to SSIIntEnable, SSIIntDisable, and SSIIntClear
// as the ulIntFlags parameter, and returned by SSIIntStatus.
//
//*****************************************************************************
#define SSI_TXFF                0x00000008  // TX FIFO half full or less
#define SSI_RXFF                0x00000004  // RX FIFO half full or more
#define SSI_RXTO                0x00000002  // RX timeout
#define SSI_RXOR                0x00000001  // RX overrun

//*****************************************************************************
//
// Values that can be passed to SSIConfigSetExpClk.
//
//*****************************************************************************
#define SSI_FRF_MOTO_MODE_0     0x00000000  // Moto fmt, polarity 0, phase 0
#define SSI_FRF_MOTO_MODE_1     0x00000002  // Moto fmt, polarity 0, phase 1
#define SSI_FRF_MOTO_MODE_2     0x00000001  // Moto fmt, polarity 1, phase 0
#define SSI_FRF_MOTO_MODE_3     0x00000003  // Moto fmt, polarity 1, phase 1
#define SSI_FRF_TI              0x00000010  // TI frame format
#define SSI_FRF_NMW             0x00000020  // National MicroWire frame format

#define SSI_MODE_MASTER         0x00000000  // SSI master
#define SSI_MODE_SLAVE          0x00000001  // SSI slave
#define SSI_MODE_SLAVE_OD       0x00000002  // SSI slave with output disabled

//*****************************************************************************
//
// Values that can be passed to SSIDMAEnable() and SSIDMADisable().
//
//*****************************************************************************
#define SSI_DMA_TX              0x00000002  // Enable DMA for transmit
#define SSI_DMA_RX              0x00000001  // Enable DMA for receive

//*****************************************************************************
//
// Values that can be passed to SSIClockSourceSet() or returned from
// SSIClockSourceGet().
//
//*****************************************************************************
#define SSI_CLOCK_SYSTEM        0x00000000
#define SSI_CLOCK_PIOSC         0x00000005

/*

    Internal C functions (For use in other C modules)   

*/

// Initialise registers for SSI
void Do_SSI_Init(uint8_t ssinum, uint16_t ssicfg, bool sdcard);

// Perform fast configuration change without complete reinitisalisation
uint16_t Do_SSI_FastConfig(uint8_t ssinum, uint16_t ssicfg);

// Disable SSI
void Do_SSI_Disable(uint8_t ssinum);

// Enable SSI
void Do_SSI_Enable(uint8_t ssinum);

// Send data via SSI
void Do_SSI_TX(uint8_t ssinum, uint32_t word);

// Send data via SSI using the FIFO (non blocking)
void Do_SSI_TX_FIFO(uint8_t ssinum, uint32_t word);

// Send 2 words data via SSI using the FIFO (non blocking)
void Do_SSI_TX16_FIFO(uint8_t ssinum, uint16_t word, bool lsb);

// Send 2 words data via SSI (blocking)
void Do_SSI_TX16(uint8_t ssinum, uint16_t word, bool lsb);

// Check if SSI port is still sending data
bool Do_SSI_Busy(uint8_t ssinum);

// Receive data via SSI 
uint8_t Do_SSI_RX(uint8_t ssinum, uint32_t *word);
void Do_SSI_RX_Blocking(uint8_t ssinum, uint32_t *word);

#endif
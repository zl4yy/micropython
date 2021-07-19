/*

    Headers
	I2C control for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	27 June 2021
	
	Software provided under MIT License

*/
#ifndef I2C_H_
#define I2C_H_

#include <stdbool.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"


//*****************************************************************************
//
// I2C Master Commands.
//
//*****************************************************************************

#define I2C_MASTER_CMD_SINGLE_SEND              0x00000007
#define I2C_MASTER_CMD_SINGLE_RECEIVE           0x00000007
#define I2C_MASTER_CMD_BURST_SEND_START         0x00000003
#define I2C_MASTER_CMD_BURST_SEND_CONT          0x00000001
#define I2C_MASTER_CMD_BURST_SEND_FINISH        0x00000005
#define I2C_MASTER_CMD_BURST_SEND_ERROR_STOP    0x00000004
#define I2C_MASTER_CMD_BURST_RECEIVE_START      0x0000000b
#define I2C_MASTER_CMD_BURST_RECEIVE_CONT       0x00000009
#define I2C_MASTER_CMD_BURST_RECEIVE_FINISH     0x00000005
#define I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP 0x00000004

//*****************************************************************************
//
// I2C Error flags.
//
//*****************************************************************************
#define I2C_MASTER_ERR_NONE          0
#define I2CM_STAT_BUSY      0x00000001
#define I2CM_STAT_ERROR     0x00000002
#define I2CM_STAT_ADRACK    0x00000004
#define I2CM_STAT_DATACK    0x00000008
#define I2CM_STAT_ARBLST    0x00000010
#define I2CM_STAT_IDLE      0x00000020
#define I2CM_STAT_BUSBSY    0x00000040
#define I2CM_STAT_INVALID   0x00000080

/*

    Internal C functions (For use in other C modules)   

*/

// Initialise registers for I2C
void Do_I2C_MasterInit(uint8_t i2cnum, bool i2cbfast);

// Send data via I2C Single Byte
bool Do_I2C_MasterTX(uint8_t i2cnum, uint8_t slaveaddr, uint8_t word);

// Send data via I2C Burst
bool Do_I2C_MasterTXBurst(uint8_t i2cnum, uint8_t slaveaddr, uint8_t bytes, uint8_t *word);

// Receive data via I2C Single Byte
bool Do_I2C_MasterRX(uint8_t i2cnum, uint8_t slaveaddr, uint8_t *word);

// Receive data via I2C Single Byte
bool Do_I2C_MasterRXBurst(uint8_t i2cnum, uint8_t slaveaddr, uint8_t bytes, uint8_t *word);

#endif
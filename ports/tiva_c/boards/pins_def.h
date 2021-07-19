/*

	Headers
	Pin Definitions and GPIO values for Texas Instruments LM4F Microcontrollers
	Based on the Stellaris LaunchPad board
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	5 June 2021
	
	Software provided under MIT License

*/

// Pin coding on two digits. First digit is port (A=1) and second digit is bit (PA0=10)

#define NOPIN 99		// If we need to send a command but without actual action

// Pins that are connected to hardware components on the board
#define PF0 60      // Switch SW2
#define PF1 61      // Red LED
#define PF2 62      // Blue LED
#define PF3 63      // Green LED
#define PF4 64      // Switch SW1

// Pins connected to J1
// Vcc 3.3V            J1.1
#define PB5 25      // J1.2
#define PB0 20      // J1.3
#define PB1 21      // J1.4
#define PE4 54      // J1.5
#define PE5 55      // J1.6
#define PB4 24      // J1.7
#define PA5 15      // J1.8
#define PA6 16      // J1.9
#define PA7 17      // J1.10

// Pins connected to J2
#define PA2 12      // J2.11
#define PA3 13      // J2.12
#define PA4 14      // J2.13
#define PB6 26      // J2.14
#define PB7 27      // J2.15
// Reset               J2.16
// PF0 SW2             J2.17
#define PE0 50      // J2.18
#define PB2 22      // J2.19
// Ground              J2.20

// Pins connected to J3
// +5V                 J3.21
// n                   J3.22
#define PD0 40      // J3.23
#define PD1 41      // J3.24
#define PD2 42      // J3.25
#define PD3 43      // J3.26
#define PE1 51      // J3.27
#define PE2 52      // J3.28
#define PE3 53      // J3.29
// PF1 Red LED         J3.30

// Pins connected to J4
// PF4 SW1             J4.31
#define PD7 47      // J4.32
#define PD6 46      // J4.33
#define PC7 37      // J4.34
#define PC6 36      // J4.35
#define PC5 35      // J4.36
#define PC4 34      // J4.37
#define PB3 33      // J4.38
// PF3 Green LED       J4.39
// PF2 Blue LED        J4.40

//*****************************************************************************
//
// Values that can be passed to GPIODirModeSet as the ui32PinIO parameter, and
// returned from GPIODirModeGet.
//
//*****************************************************************************
#define GPIO_DIR_MODE_IN        0x00000000  // Pin is a GPIO input
#define GPIO_DIR_MODE_OUT       0x00000001  // Pin is a GPIO output
#define GPIO_DIR_MODE_HW        0x00000002  // Pin is a peripheral function

//*****************************************************************************
//
// Values that can be passed to GPIOIntTypeSet as the ui32IntType parameter,
// and returned from GPIOIntTypeGet.
//
//*****************************************************************************
#define GPIO_FALLING_EDGE       0x00000000  // Interrupt on falling edge
#define GPIO_RISING_EDGE        0x00000004  // Interrupt on rising edge
#define GPIO_BOTH_EDGES         0x00000001  // Interrupt on both edges
#define GPIO_LOW_LEVEL          0x00000002  // Interrupt on low level
#define GPIO_HIGH_LEVEL         0x00000006  // Interrupt on high level
#define GPIO_DISCRETE_INT       0x00010000  // Interrupt for individual pins

//*****************************************************************************
//
// Values that can be passed to GPIOPadConfigSet as the ui32Strength parameter,
// and returned by GPIOPadConfigGet in the *pui32Strength parameter.
//
//*****************************************************************************
#define GPIO_STRENGTH_2MA       0x00000001  // 2mA drive strength
#define GPIO_STRENGTH_4MA       0x00000002  // 4mA drive strength
#define GPIO_STRENGTH_6MA       0x00000065  // 6mA drive strength
#define GPIO_STRENGTH_8MA       0x00000066  // 8mA drive strength
#define GPIO_STRENGTH_8MA_SC    0x0000006E  // 8mA drive with slew rate control
#define GPIO_STRENGTH_10MA      0x00000075  // 10mA drive strength
#define GPIO_STRENGTH_12MA      0x00000077  // 12mA drive strength

//*****************************************************************************
//
// Values that can be passed to GPIOPadConfigSet as the ui32PadType parameter,
// and returned by GPIOPadConfigGet in the *pui32PadType parameter.
//
//*****************************************************************************
#define GPIO_PIN_TYPE_STD       0x00000008  // Push-pull
#define GPIO_PIN_TYPE_STD_WPU   0x0000000A  // Push-pull with weak pull-up
#define GPIO_PIN_TYPE_STD_WPD   0x0000000C  // Push-pull with weak pull-down
#define GPIO_PIN_TYPE_OD        0x00000009  // Open-drain
#define GPIO_PIN_TYPE_ANALOG    0x00000000  // Analog comparator
#define GPIO_PIN_TYPE_WAKE_HIGH 0x00000208  // Hibernate wake, high
#define GPIO_PIN_TYPE_WAKE_LOW  0x00000108  // Hibernate wake, low

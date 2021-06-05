/*

	Headers
	Pin Definitions for Texas Instruments LM4F Microcontrollers
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
#define PC6 37      // J4.35
#define PC5 35      // J4.36
#define PC4 34      // J4.37
#define PB3 38      // J4.38
// PF3 Green LED       J4.39
// PF2 Blue LED        J4.40

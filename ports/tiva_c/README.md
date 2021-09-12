# Texas Instruments Tiva C Micropython port (based on minimal port to STM32)

Yannick Devos (ZL4YY), 15 August 2021

This port is intended to be a minimal MicroPython port that actually runs on the Tiva C
platform. It was developed and tested on the Stellaris Launchpad (EK-LM4F120XL) board but
should run on other similar MCUs from Texas Instruments with reasonable effort.

The LM4F120XL is build around the LM4F120H5QR micro-controller which is Cortex-M4 based
and runs at 80 MHz with 256KB of Flash, 32KB of SRAM and a wide range of peripherals
including GPIOs, I2C, SPI, ADCs and USB.

For more information on Texas Instruments' Stellaris and Tiva C Launchpad
https://www.ti.com/tool/EK-LM4F120XL
https://www.ti.com/tool/EK-TM4C123GXL


## What hardware / software feature are supported

This is a minimalist port and only provides:
- REPL interface to MicroPython
- UART0 at 115kbps (via USB debug port)
- Onboard GPIO control
- Time and delay using SysTick
- Nokia 5110 LCD clones (PD8544 based)
- ILI9486 and ILI9488 based TFT LCDs (WIP)
- XPT2046 based touchscreen (Experimental)
- SSI (SPI) communication
- SD Card reader in SPI mode (Read only)
- I2C Master (without IRQ)
- Bosch BMP085 temperature and pressure sensor
- Freescale MMA7455 accelerometer sensor

ADC, I2C slave, DMA or hardware FPU are not supported.

## Time and delay
SysTick support, initialised by default.
Example usage:

	import time
	time.sleep(2)
	time.sleep_ms(1000)
	time.sleep_us(100)
	start = time.ticks_ms() # get value of millisecond counter
	delta = time.ticks_diff(time.ticks_ms(), start) # compute time difference


## GPIO and onboard LED and switch control
GPIO pin basic support with aliases for onboard LED and switches.
Pins are encoded by Port and Pin number. Ex: PC5 is 35 (Port C =3 and pin 5), PA2 is 12, PE7 is 57...
Example usage:

	import gpio
	gpio.init()
	gpio.output(gpio.red)
	gpio.up(gpio.red)
	gpio.down(gpio.red)

	gpio.input(gpio.sw1)
	gpio.read(gpio.sw1)


## LCD 5110 Control (PD8544)
Interfacing settings (hard or soft SPI, pins) need to be set in the source code. Example usage:

	import lcd
	lcd.init()
	lcd.setfont(lcd.large)
	lcd.text(0,0,'TIVA C')
	lcd.setfont(lcd.small)
	lcd.text(0,4,'MicroPython')
	lcd.setXY(3,3)
	lcd.write(lcd.data,0xFF)
	lcd.write(lcd.data,0xF0)
	lcd.write(lcd.data,0x0F)
	lcd.write(lcd.data,0xFF)
	lcd.plot(48,24)
	lcd.plot(49,24)
	lcd.plot(50,24)
	lcd.backlight(1)
	lcd.inversevideo(1)
	lcd.clear(0)
	lcd.clear(1)

Font can be lcd.small or lcd.large and lcd.write first's parameter is either lcd.data or lcd.cmnd.
NOTE: The PD8544 chip must be initialised very shortly after power up or it does not work properly.
Initialisation at boot time is configurable in mpconfigport.h.
You may need to start the MCU first and apply power to the LCD only before the lcd.init().


## TFT based on ILI9486/88 (WIP)
There is a functional but limited support for ILI9486 and ILI9488 based TFT panels. Text or advanced drawing
is not yet implemented. This has not been tested on ILI9486 but works on ILI9488.
The parameters for init are spi port, pin for ChipSelect, pin for DataCommand and pin for Reset.
Fast drawing is possible using C code in your modules (look at the Fractals module for examples).

	tft.init(0, 34, 35, 36)
	tft.clear(<colour>)
	tft.setOrientation(0)
	tft.fill(<x>,<y>,<w>,<h>,<colour>)
	tft.plot(<x>,<y>,<colour>)
	tft.get565colour(40,70,128)
	lcd.write(1,0x01)

The first parameter of write is 0 for data, 1 for command to be sent to the LCD.
Colours are 565 packed. The get565colour takes 8-bit R G B values and returns 16-bit 565 values.

## Touchscreen based on XPT2046 (Experimental)
There is an experimentatl support for XPT2046 chips. It works but still returns unpredictable results.
The parameters for init are spi port and pin for ChipSelect.

	xpt2046.init(0,37)
	xpt2046.setOrientation(3)
	xpt2046.setTreshold(300)
	xpt2046.printRaw()
	xpt2046.getZ()
	xpt2046.getX()
	xpt2046.getY()

Only getZ and printRaw actually do an updated with the chip. getX and getY only return previously obtained values.


## SSI Support
Example usage:

	import ssi
	ssi.init(0,10041)
	ssi.write_fifo(0,0x21)
	print(ssi.isbusy(0))
	ssi.write(0,0xFF)

NOTE: 4 SSI ports are available but SSI 1 may conflict with LaunchPad LED and switch hardware.
The first value of the init command is the SSI port, second is configuration under the form CMFBD
	- C is for clock source, 1 for PIO at 16 MHz or 2 for CPU clock at 80MHz)
	- M is 0 for master, 1 for slave, 2 for slave OD
	- F is Frame Format with 0/1/2/3 being Freescale frame format, 4 is Microwire, 5 is TI SFF
	- B is bitrate with 0 = 250Kbps, 1 = 500 Kbps, 2 = 1 Mbps, 3 = 2 Mbps, 4 = 4 Mbps, 5 = 8 Mbps,
		6 = 12 Mbps, 7 = 16 Mbps, 8 = 20 Mbps, 9 = 25 Mbps (C =2 required above 8 Mbps)
	- D is data size (0 = 4, 1 = 8, 2 = 16)
If configuration is incorrect,  default is SPI Master Motorola/Freescale mode SPO=0 SPH=0, MSBFIRST, 8 Mbps. DMA operation is not available.
All other commands take port number as first value.

## SD Card support
Basic SD Card support is available, Read-Only at the moment.
The module can list files in the root dir, print text files to screen, read files and return the content as a string, run .py files (Python text code) and run .mpy files (Python Object code).
File size for readfile and execfile is limited by a Buffer which size is set in modules/sdcard.h (default is 4KB)

There is an option to configure the main.c code to initialise SD Card at MCU start and boot from a file on the SD Card

	import sdcard
	sdcard.init(3)				Use SSI port 3
	sdcard.listdir()
	sdcard.printfile(0)			Print first file of the list (#0) to screen
	exec(sdcard.readfile(0))	Execute first file
	sdcard.execfile(0)			Execute firtt file, but uses less memory than previous command

## I2C Support
I2C is supported. Master only without IRQ at the moment. Port 0 to 3 are accessible at either 100kbps or 400 kbps.
Python functions are available to send individual bytes, and receive either individual bytes or burst of bytes (up to 4). More functions are available in C for internal module use.

Here are the available Python functions

	import i2c
	i2c.init(0,0)				Use port 0 at 100kbps
	i2c.write(0,0x77,0xd0)		Send 0xd0 to slave address 0x77 on I2C port 0
	i2c.read(0,0x77)			Read one byte of data from slave 0x77 on port 0
	i2c.request(0,0x77,0xaa,2)	Send 0xaa and then read two bytes of data from slave 0x77 on port 0

## Bosch BMP085 pressure sensor Support
The BMP085 can be connected via I2C. BMP180 should be supported with only minor modifications but has not been tested. The following functions are supported:

	import bmp085
	bmp085.init(0)				Initialise BMP085 on I2C port 0
	bmp085.print()				Print temperature and pressure
	bmp085.get_temp()			Return temperature (in tenth of degrees Celcius)
	bmp086.get_pressure()		Return pressure (in Pascal)

## FreeScale MMA7455 accelerometer sensor Support
The MMA7455 can be connected via I2C. Similar sensors should be supported but have not been tested. The following functions are supported:
	
	import mma7455
	mma7455.init(0,2)			Initialise MMA7455 on I2C port 0 with 2G sensitivity (other values are 4 and 8)
	mma7455.print()				Print temperature and pressure
	mma7455.get_x()				Return G force on x axis (unit???)
	mma7455.get_y()				Return G force on y axis (unit???)
	mma7455.get_z()				Return G force on z axis (unit???)

## Compute and display Mandelbrot set
Uses the FPU unit of the micro controller to compute and display the Mandelbrot set.
Currently display only works to the UART (serial) and LCD Nokia 5110. Output settings can be
changed in the source code.
Parameters for the tracing functions print_mandel and plot_mandel are:
	- x start and stop for region (integer only)
	- y start and stop for region (integer only)
	- maximum number of iterations (Numbers higher that 5000 use significant compute time)
	- x resolution (fixed with plot_mandel for LCD to 84)
	
	import fractals
	fractals.print_mandel(-2,1,-1,1,40,100)
	fractals.print_mandel(-1,0,0,1,40,100)
	fractals.plot_mandel(-1,0,0,1,40)

Only integers can be used as inputs for the above functions, but a separate functions can make them
act as fractions, with the parameters then being the denominators. If calling again the tracing functions,
this can be used as a "zoom" function:

	fractals.print_mandel(-2,1,-1,1,40,100)
	fractals.set_denominators(2,2,2,2)
	fractals.print_mandel()

Setting precise regions of the fractal set to draw will require some skillfull computations to give the
right values.
	fractals.set_denominators(3,22,5,4)
	fractals.plot_mandel(1,10,1,1,100)

	fractals.set_denominators(3,23,5,4)
	fractals.plot_mandel(1,9,-1,-1,100)

	fractals.set_denominators(30,22,46,41)
	fractals.plot_mandel(11,9,-10,-10,100)

	fractals.set_denominators(3,26,11,9)
	fractals.plot_mandel(1,11,-2,-2,100)


## Running the Frozen bytecode to test GPIO
An example of frozen bytecode is provided in gpiotest.py to demonstrate GPIO usage. It
should be included in your build by default. To execute it, just run:

	import gpiotest


## Installing a cross-compiler toolchain

The cross-compiler toolchain used to develop this port is the one supplied with the
Energia IDE (https://energia.nu/) which is a fork of Arduino for Texas Instruments MCUs.
With Energia 15 on MacOS, the gcc suite and DSlite (debug / flashing tool) can be found
at the following locations:

	/Users/*yourusername*/Library/Energia15/packages/energia/tools/dslite
	/Users/*yourusername*/Library/Energia15/packages/energia/tools/arm-none-eabi-gcc/

To simplify their use I added the following commands to my .zshrc (or .bashrc):

	alias FlashTiva="DSlite flash -e --config=/Users/*yourusername*/Library/Energia15/packages/energia/tools/dslite/9.3.0.1863/EK-TM4C123GXL.ccxml"
	export PATH=$PATH:/Users/*yourusername*/Library/Energia15/packages/energia/tools/arm-none-eabi-gcc/8.3.1-20190703/bin
	export PATH=$PATH:/Users/*yourusername*/Library/Energia15/packages/energia/tools/dslite/9.3.0.1863/DebugServer/bin

In order to find the right paths and parameters for your setup or Energia or OS, you can
configure Energia's preferences to provide verbose compiler outputs and the full commands
will be displayed when compiling and uploading one of the examples.


## Building for an LM4F120H5QR MCU

The Makefile has the ability to build for a Cortex-M CPU, and by default
includes some start-up code for an LM4F120H5QR MCU and also enables a UART
for communication.  To build:

    $ make CROSS=1

If you previously built another version, you will need to first run
`make clean` to get rid of incompatible object files.

Building will produce the build/firmware.dfu file which can be programmed
to an MCU using:

    $ make CROSS=1 deploy


This version of the build will work out-of-the-box on the Stellaris LaunchPad (and
anything similar such as the Tiva C Launchpad) and will give you a MicroPython REPL
on UART0 at 115200 baud (accessible via USB debug connection).
The GPIO module allows you to do a minimal control of LEDs and switches.


## Building without the built-in MicroPython compiler

This minimal port can be built with the built-in MicroPython compiler
disabled.  This will reduce the firmware by about 20k on a Thumb2 machine,
and by about 40k on 32-bit x86.  Without the compiler the REPL will be
disabled, but pre-compiled scripts can still be executed.

To test out this feature, change the `MICROPY_ENABLE_COMPILER` config
option to "0" in the mpconfigport.h file in this directory.  Then
recompile and run the firmware and it will execute the gpiotest.py
file.

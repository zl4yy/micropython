# Texas Instruments Tiva C Micropython port (based on minimal port to STM32)

Yannick Devos (ZL4YY), 29 May 2021

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
- Onboard GPIO control to use LEDs and Switches

Full GPIO control, SPI, I2C, DMA or hardware FPU are not supported.


## Onboard LED control
Example usage:
	import gpio
	gpio.init()
	gpio.output(gpio.red)
	gpio.up(gpio.red)
	gpio.down(gpio.red)

	gpio.input(gpio.sw1)
	gpio.read(gpio.sw1)


The values for the pin can be 0/1/2/3/4 or gpio.red/blue/green/sw1/sw2 or gpio.pf0/pf1/pf2/pf3/pf4


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

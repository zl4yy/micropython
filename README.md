[![CI badge](https://github.com/micropython/micropython/workflows/unix%20port/badge.svg)](https://github.com/micropython/micropython/actions?query=branch%3Amaster+event%3Apush) [![Coverage badge](https://coveralls.io/repos/micropython/micropython/badge.png?branch=master)](https://coveralls.io/r/micropython/micropython?branch=master)

The MicroPython project (ZL4YY's fork)
======================================
<p align="center">
  <img src="https://raw.githubusercontent.com/micropython/micropython/master/logo/upython-with-micro.jpg" alt="MicroPython Logo"/>
</p>

This is a fork of the MicroPython project to work on a port to the
Texas Instruments LM4F120 microcontroller (Stellaris Launchpad).
Micropython aims to put an implementation of Python 3.x on microcontrollers and small
embedded systems. You can find the official website at [micropython.org](http://www.micropython.org).

This port gas been developed on the LM4F120 microcontroller but should work
on other similar microcontrollers from the Tiva C range (e.g. TM4C123).

Objectives of this fork
-----------------------
This repo and fork has been setup mainly as a sandpit for me to play with the MicroPython
code on a Stellaris LaunchPad board. If you are looking for other platforms or more
general MicroPython code, please go to the origin repo.

General principles underpinning my work:
- This is more about the LM4F120 / TM4C123 (Tiva C) microcontrollers than MicroPython
- Code to control with the MCU or Hardware modules will preferably be written in C
- MicroPython's REPL is mainly seen as a front-end to the MCU
- I miss the days of the simple home computers with BASIC interpreters, and this was a
way to recreate a substitute retro-computer.

MicroPython's general intro
---------------------------
WARNING: this project is in beta stage and is subject to changes of the
code-base, including project-wide name changes and API changes.

MicroPython implements the entire Python 3.4 syntax (including exceptions,
`with`, `yield from`, etc., and additionally `async`/`await` keywords from
Python 3.5). The following core datatypes are provided: `str` (including
basic Unicode support), `bytes`, `bytearray`, `tuple`, `list`, `dict`, `set`,
`frozenset`, `array.array`, `collections.namedtuple`, classes and instances.
Builtin modules include `sys`, `time`, and `struct`, etc. Select ports have
support for `_thread` module (multithreading). Note that only a subset of
Python 3 functionality is implemented for the data types and modules.

MicroPython can execute scripts in textual source form or from precompiled
bytecode, in both cases either from an on-device filesystem or "frozen" into
the MicroPython executable.

See the repository http://github.com/micropython/pyboard for the MicroPython
board (PyBoard), the officially supported reference electronic circuit board.

Major components in this repository:
- py/ -- the core Python implementation, including compiler, runtime, and
  core library.
- mpy-cross/ -- the MicroPython cross-compiler which is used to turn scripts
  into precompiled bytecode.
- ports/tiva_c -- a version intending to have a relatively complete feature set
- ports/tiva_c-minimal -- a version including minimal functionality (UART, REPL and LEDs)
- tests/ -- test framework and test scripts.
- docs/ -- user documentation in Sphinx reStructuredText format. Rendered
  HTML documentation is available at http://docs.micropython.org.

The subdirectories above may include READMEs with additional info. For more details
about the other components and sub-directories, go to the main repo.

"make" is used to build the components, or "gmake" on BSD-based systems.
You will also need bash, gcc, and Python 3.3+ available as the command `python3`
(if your system only has Python 2.7 then invoke make with the additional option
`PYTHON=python2`).

The MicroPython cross-compiler, mpy-cross
-----------------------------------------

Most ports require the MicroPython cross-compiler to be built first.  This
program, called mpy-cross, is used to pre-compile Python scripts to .mpy
files which can then be included (frozen) into the firmware/executable for
a port.  To build mpy-cross use:

    $ cd mpy-cross
    $ make


The Tiva C (LM4F120) version
-------------------

There is a functional minimal port to the Texas Instruments Stellaris LaunchPad.
UART and basic GPIO control are implemented.
Another port is still under development but brings support of GPIO, SSI, I2C as well as
SD Card and other peripherals.

More details available in the ports/tiva_c-minimal and ports/tiva_c directories.

Once the compiler toolchain is installed, to build:

    $ cd ports/tiva_c-minimal
    $ make

To Flash it to the Launchpad:
	
	$ FlashTiva build/firmware.elf

Connect to the UART0 via the USB debug port. Example on MacOS (device address may vary):

	$screen /dev/cu.usbmodem0E104ED61

Then to give it a try:

    $ ./micropython
    >>> list(5 * x + y for x in range(10) for y in [4, 2, 1])

Browse available modules on
[PyPI](https://pypi.python.org/pypi?%3Aaction=search&term=micropython).
Standard library modules come from
[micropython-lib](https://github.com/micropython/micropython-lib) project.

Contributing
------------

MicroPython is an open-source project and welcomes contributions. To be
productive, please be sure to follow the
[Contributors' Guidelines](https://github.com/micropython/micropython/wiki/ContributorGuidelines)
and the [Code Conventions](https://github.com/micropython/micropython/blob/master/CODECONVENTIONS.md).
Note that MicroPython is licenced under the MIT license, and all contributions
should follow this license.

include ../../py/mkenv.mk

CROSS = 1

V = 1

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# include py core make definitions
include $(TOP)/py/py.mk

ifeq ($(CROSS), 1)
CROSS_COMPILE ?= arm-none-eabi-
endif

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

ifeq ($(CROSS), 1)
DFU = $(TOP)/tools/dfu.py
PYDFU = $(TOP)/tools/pydfu.py
MCUTARGET = -DTARGET_IS_TM4C123_RA3 -DPART_LM4F120H5QR #Forcing target for lm4f_rom.h and lm4f_pin_map.h
CFLAGS_CORTEX_M4 = -mthumb -mtune=cortex-m4 -mcpu=cortex-m4 -fsingle-precision-constant -Wdouble-promotion -Wfloat-conversion -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS = $(INC) -Wall -Werror -std=c99 -nostdlib $(CFLAGS_CORTEX_M4) $(COPT) $(MCUTARGET)

LDFLAGS = -nostdlib -T boards/lm4f120xl.ld -Map=$@.map --cref --gc-sections
else
LD = gcc
CFLAGS = -m32 $(INC) -Wall -Werror -Wdouble-promotion -Wfloat-conversion -std=c99 $(COPT)
CFLAGS += -DMICROPY_FLOAT_IMPL=MICROPY_FLOAT_IMPL_FLOAT
CFLAGS += -fsingle-precision-constant
LDFLAGS = -m32 -Wl,-Map=$@.map,--cref -Wl,--gc-sections
endif

CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Sources
SRC_C = \
	main.c \
	uart_core.c \
	shared/libc/printf.c \
	shared/runtime/stdout_helpers.c \
	shared/runtime/pyexec.c \
	shared/readline/readline.c \
	modules/gpio.c \
	modules/time.c \
	modules/ssi.c \
	modules/lcd5110.c \
	modules/lcd_ili9486.c \
	modules/xpt2046.c \
	modules/sdcard.c \
	modules/sdcard_mod.c \
	modules/i2c.c \
	modules/bmp085.c \
	modules/mma7455.c \
	modules/fractals.c \
#	modules/ds1307.c \

ifeq ($(CROSS), 1)
SRC_C += shared/libc/string0.c
endif

# Frozen MPY code
SRC_C += \
	$(BUILD)/_gpiotest_mpy.c \


SRC_QSTR += modules/gpio.c
SRC_QSTR += modules/time.c 
SRC_QSTR += modules/lcd5110.c 
SRC_QSTR += modules/lcd_ili9486.c 
SRC_QSTR += modules/xpt2046.c 
SRC_QSTR += modules/ssi.c 
SRC_QSTR += modules/sdcard_mod.c 
SRC_QSTR += modules/i2c.c 
SRC_QSTR += modules/bmp085.c 
SRC_QSTR += modules/mma7455.c 
#SRC_QSTR += modules/ds1307.c 
SRC_QSTR += modules/fractals.c 

OBJ = $(PY_CORE_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

ifeq ($(CROSS), 1)
all: $(BUILD)/firmware.dfu
else
all: $(BUILD)/firmware.elf
endif

$(BUILD)/_gpiotest_mpy.c: code_py/gpiotest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "Freezing GPIO test bytecode"
	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin

$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$(BUILD)/firmware.bin $@

deploy: $(BUILD)/firmware.elf
	$(ECHO) "Writing $< to the board"
#	$(Q)$(PYTHON) $(PYDFU) -u $<
	FlashTiva build/firmware.elf
	
# Run on target MCU
run:
	@echo Connecting to UART terminal...
	screen /dev/cu.usbmodem0E104ED61
# This sleep is useful to spot segfaults
	sleep 1
	reset

test: $(BUILD)/firmware.elf
	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

include $(TOP)/py/mkrules.mk

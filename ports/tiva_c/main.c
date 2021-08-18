/*

	Basic Port for Texas Instruments LM4F Microcontrollers
	Based on micropython's minimal port code
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
	10 May 2021
	
	Software provided under MIT License

*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "boards/lm4f_rom.h"
#include "boards/lm4f_sysctl.h"
#include "boards/lm4f_registers.h"

#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/pyexec.h"

#ifdef INIT_SDCARD
#include "modules/sdcard.h"
#include "modules/time.h"
void SDCARD_boot (void);    // Declaration only
# endif

#ifdef INIT_LCD5110
#include "modules/lcd5110.h"
# endif

static char *stack_top;
#if MICROPY_ENABLE_GC
static char heap[MICROPY_HEAPSIZE];
#endif

#if MICROPY_MIN_USE_LM4F_MCU
// Prototype
void lm4f_reset(void);
#endif


int main(int argc, char **argv) {
    int stack_dummy;
    stack_top = (char *)&stack_dummy;

    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();
    #if MICROPY_ENABLE_COMPILER
    #ifdef INIT_SDCARD
    #if INIT_SDCARD_BOOT
    SDCARD_boot();
    #endif
    #endif
    #if MICROPY_REPL_EVENT_DRIVEN
    pyexec_event_repl_init();
    for (;;) {
        int c = mp_hal_stdin_rx_chr();
        if (pyexec_event_repl_process_char(c)) {
            break;
        }
    }
    #else
    pyexec_friendly_repl();
    #endif
    #else
    pyexec_frozen_module("gpiotest.py");
    #endif
    mp_deinit();
    // Reset Board
    #if MICROPY_MIN_USE_LM4F_MCU
    lm4f_reset();
    #endif
    return 0;
}

#if MICROPY_ENABLE_GC
void gc_collect(void) {
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
}
#endif

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

void NORETURN __fatal_error(const char *msg) {
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

#if MICROPY_MIN_USE_CORTEX_CPU

// this is a minimal IRQ and reset framework for any Cortex-M CPU

extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void) __attribute__((naked));
void Reset_Handler(void) {
    // set stack pointer
    __asm volatile ("ldr sp, =_estack");
    // copy .data section from flash to RAM
    for (uint32_t *src = &_sidata, *dest = &_sdata; dest < &_edata;) {
        *dest++ = *src++;
    }
    // zero out .bss section
    for (uint32_t *dest = &_sbss; dest < &_ebss;) {
        *dest++ = 0;
    }
    // jump to board initialisation
    void _start(void);
    _start();
}

void Default_Handler(void) {
    for (;;) {
    }
}

const uint32_t isr_vector[] __attribute__((section(".isr_vector"))) = {
    (uint32_t)&_estack,
    (uint32_t)&Reset_Handler,
    (uint32_t)&Default_Handler, // NMI_Handler
    (uint32_t)&Default_Handler, // HardFault_Handler
    (uint32_t)&Default_Handler, // MemManage_Handler
    (uint32_t)&Default_Handler, // BusFault_Handler
    (uint32_t)&Default_Handler, // UsageFault_Handler
    0,
    0,
    0,
    0,
    (uint32_t)&Default_Handler, // SVC_Handler
    (uint32_t)&Default_Handler, // DebugMon_Handler
    0,
    (uint32_t)&Default_Handler, // PendSV_Handler
    (uint32_t)&Default_Handler, // SysTick_Handler
};

void _start(void) {
    // when we get here: stack is initialised, bss is clear, data is copied

    // SCB->CCR: enable 8-byte stack alignment for IRQ handlers, in accord with EABI
    *((volatile uint32_t *)0xe000ed14) |= 1 << 9;

    #if MICROPY_MIN_USE_LM4F_MCU
    void lm4f_init(void);
    lm4f_init();
    #endif

    // now that we have a basic system up and running we can call main
    main(0, NULL);

    // we must not return
    for (;;) {
    }
}

#endif


#if MICROPY_MIN_USE_LM4F_MCU

// this is minimal set-up code for a LM4F MCU

//*****************************************************************************
// A counter that keeps track of the number of times the TX & RX interrupt has
// occurred, which should match the number of messages that were transmitted /
// received.
//*****************************************************************************
volatile uint32_t g_ui32RXMsgCount = 0;
volatile uint32_t g_ui32TXMsgCount = 0;

//*****************************************************************************
// A flag for the interrupt handler to indicate that a message was received.
//*****************************************************************************
volatile bool g_bRXFlag = 0;

//*****************************************************************************
// A global to keep track of the error flags that have been thrown so they may
// be processed. This is necessary because reading the error register clears
// the flags, so it is necessary to save them somewhere for processing.
//*****************************************************************************
volatile uint32_t g_ui32ErrFlag = 0;

//*****************************************************************************
// Message Identifiers and Objects
// RXID is set to 0 so all messages are received
//*****************************************************************************
#define RXOBJECT                1
#define TXOBJECT                2

//*****************************************************************************
// Variables to hold character being sent / reveived
//*****************************************************************************
uint8_t g_ui8TXMsgData;
uint8_t g_ui8RXMsgData;

//*****************************************************************************
// The error routine that is called if the driver library encounters an error.
//*****************************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line) { }
#endif

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 50 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none

void UART_Init0(void){
  SYSCTL_RCGCUART_R |= 0x01;            // activate UART0
  SYSCTL_RCGCGPIO_R |= 0x01;            // activate port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
//  UART0_IBRD_R = 520;                    // IBRD = int(80,000,000 / (16 * 9,600)) = int(520.83)
//  UART0_FBRD_R = 53;                     // FBRD = int(0.83 * 64) = 53
  UART0_IBRD_R = 520;                    // IBRD = int(80,000,000 / (16 * 115,200)) = int(43.4028)
  UART0_FBRD_R = 26;                     // FBRD = int(0.4028 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

#if INIT_SDCARD
// SD Card Init
void SDCARD_Init(void) {
    Do_SysTick_Init();
	Do_SysTick_Waitms(100);
    Do_SD_SetPins(INIT_SDCARD_SPI_PORT);

    // Using SSI 3 Master, SPI frame format, 250 Kbps, 8 data bits
    // LM4F's SSI support 32 bits frames but the code is currently writen to shift 8 bits at a time
    Do_SSI_Init(INIT_SDCARD_SPI_PORT, 10001, true);

	if (Do_SD_initialise()) {
        Do_SD_cs_high();
        // Using SSI 3 Master, SPI frame format, 8 Mbps, 8 data bits.
        // 8 Mbps is conservative but should work with all SD cards and is plenty enough for most applications
        // LM4F's SSI support 32 bits frames but the code is currently writen to shift 8 bits at a time
        // NOTE: Settings must be consistent with Do_SD_initialise in sdcard.c used for high speed reset
        Do_SSI_Init(INIT_SDCARD_SPI_PORT, 20051, true);
        Do_SD_tx_SSI();

        Do_SD_cs_low();
        Do_SD_read_first_sector();
        Do_SD_read_disk_data();

        mp_printf(&mp_plat_print, "SD Card initialised.\n");

        // List files and store them in memory but doesn't print out the result
        long next_cluster=Do_SD_get_root_dir_first_cluster();
        do {
            next_cluster=Do_SD_get_files_and_dirs(next_cluster, LONG_NAME, GET_SUBDIRS, false);
        } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF);

    } else {
        mp_printf(&mp_plat_print, "SD Card init error.\n");
    }
}

#if INIT_SDCARD_BOOT
// Execute bootfile from SD Card
void SDCARD_boot (void) {

    uint8_t filenum = Do_SD_find_file_by_name("boot@@@@.py@");

    if (filenum<40) {
        uint16_t offset=0;
        unsigned char source[BUFFERSIZE];

        long next_cluster=Do_SD_get_first_cluster(filenum);

        do {
            next_cluster=Do_SD_read_file(next_cluster, &source[offset], &offset);
        } while (next_cluster!=0x0FFFFFFF && next_cluster!=0xFFFFFFFF && offset < BUFFERSIZE);

        if (offset>=BUFFERSIZE) {
            mp_printf(&mp_plat_print, "Boot file too large.\n");
        } else {
            mp_printf(&mp_plat_print, "Executing bootfile.\n");
            nlr_buf_t nlr;
            if (nlr_push(&nlr) == 0) {
                mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, (char*)source, strlen((char*)source), 0);
                qstr source_name = lex->source_name;
                mp_parse_tree_t parse_tree = mp_parse(lex, MP_PARSE_FILE_INPUT);
                mp_obj_t module_fun = mp_compile(&parse_tree, source_name, true);
                mp_call_function_0(module_fun);
                nlr_pop();
            } else {
                // uncaught exception
                mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
            }
        };
    } else {
        mp_printf(&mp_plat_print, "Boot file not found.\n");
    }


}
#endif
#endif


// Function called when REPL ends
void lm4f_reset(void) {
    // Reset MCU
    ROM_SysCtlReset();
}


//*****************************************************************************
//
// Set up the system, initialize the UART, and CAN. Then poll the
// UART for data. If there is any data send it, if there is any thing received
// print it out to the UART. If there are errors call the error handling
// function.
//
//*****************************************************************************
void lm4f_init(void) {
    // Set the clocking to run directly from the crystal.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Initialise the hardware
    UART_Init0();

    #if INIT_SDCARD
    SDCARD_Init();
    #endif

    #if INIT_LCD5110
    Do_LCD_Init();
    #endif

}

#endif

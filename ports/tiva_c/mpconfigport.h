#include <stdint.h>

// options to control how MicroPython is built

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)

#define MICROPY_HEAPSIZE            (8192)     // 8KB
#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_QSTR_EXTRA_POOL     mp_qstr_frozen_const_pool
#define MICROPY_ALLOC_PATH_MAX      (256)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (16)
#define MICROPY_COMP_CONST          (0)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_GC_ALLOC_THRESHOLD  (0)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT      (0)
#define MICROPY_PY_ASSIGN_EXPR      (0)
#define MICROPY_PY_BUILTINS_EXECFILE        (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (0)
#define MICROPY_PY_BUILTINS_DICT_FROMKEYS (0)
#define MICROPY_PY_BUILTINS_ENUMERATE (0)
#define MICROPY_PY_BUILTINS_FILTER  (0)
#define MICROPY_PY_BUILTINS_REVERSED (0)
#define MICROPY_PY_BUILTINS_SET     (0)
#define MICROPY_PY_BUILTINS_SLICE   (0)
#define MICROPY_PY_BUILTINS_PROPERTY (0)
#define MICROPY_PY_BUILTINS_MIN_MAX (0)
#define MICROPY_PY_BUILTINS_STR_COUNT (0)
#define MICROPY_PY_BUILTINS_STR_OP_MODULO (0)
#define MICROPY_PY___FILE__         (0)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (0)
#define MICROPY_PY_ATTRTUPLE        (0)
#define MICROPY_PY_COLLECTIONS      (0)
#define MICROPY_PY_IO               (0)
#define MICROPY_PY_STRUCT           (0)
#define MICROPY_PY_SYS              (0)
#define MICROPY_MODULE_FROZEN_MPY   (0)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_MODULE_GETATTR      (0)

// Port specific modules
#define MICROPY_MODULE_GPIO         (1)
#define MICROPY_MODULE_TIME         (1)
#define MICROPY_MODULE_SSI          (1)
#define MICROPY_MODULE_I2C          (1)
#define MICROPY_MODULE_LCD5110      (0)
#define MICROPY_MODULE_LCD_ILI9486  (1)
#define MICROPY_MODULE_XPT2046      (1)
#define MICROPY_MODULE_SDCARD       (1)
#define MICROPY_MODULE_BMP085       (0)
#define MICROPY_MODULE_MMA7455      (0)
//#define MICROPY_MODULE_DS1307      (1)

// Miscellaneous modules
#define MICROPY_MODULE_FRACTALS     (1)

// Default behaviour
#define INIT_SDCARD                 (1)
#define INIT_SDCARD_SPI_PORT        (0)
#define INIT_SDCARD_BOOT            (1)
#define INIT_LCD                    (1)
#define INIT_TFT_SPI                (3)
#define INIT_TFT_CS                 (34)
#define INIT_TFT_DC                 (35)
#define INIT_TFT_RST                (36)
#define INIT_TP_CS                  (37)


// type definitions for the specific machine

typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HW_BOARD_NAME "tiva_c"
#define MICROPY_HW_MCU_NAME "lm4f120xl"

#ifdef __linux__
#define MICROPY_MIN_USE_STDOUT (1)
#endif

#ifdef __thumb__
#define MICROPY_MIN_USE_CORTEX_CPU (1)
#define MICROPY_MIN_USE_LM4F_MCU (1)
#endif

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8];

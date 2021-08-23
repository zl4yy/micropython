/*

	Compute fractal sets for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	23 August 2021
	
    Based on code found here:
    https://gist.github.com/andrejbauer/7919569
    https://forum.micropython.org/viewtopic.php?f=21&t=9838

	Software provided under MIT License

*/
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"

#define DISPLAY_LCD5110 (1)    // Enable output to the Nokia 5110 Display

#if DISPLAY_LCD5110
#define MAX_X (84)
#define MAX_Y (48)
#include "modules/lcd5110.h"
#endif

uint16_t _maxiter = 40;
int16_t _xmin = -2;
int16_t _xmax = 1;
int16_t _ymin = -1;
int16_t _ymax = 1;
uint16_t _xres = 90;
uint16_t _yres;
const uint8_t symbols[20] ={' ', '.', '`', ',', ':', ';', '|', 'o', '<', '>', '(', ')', '{', '}', '+', '~', '=', '-', '#', '@'};

/*

    Internal C functions (For use in other modules)   

*/

// Print Mandelbrot with Char output
void Do_Fractals_Print_Mandel(uint8_t output) {
    assert(FLT_EVAL_METHOD == 0);
    uint8_t num_sym = sizeof(symbols)/sizeof(symbols[0]);
    mp_printf(&mp_plat_print, "Start\n");
    float x ,y;
    float u, v, u2, v2; /* Coordinates of the iterated point. */
    uint8_t i,j; /* Pixel counters */
    uint8_t k; /* Iteration counter */
    uint8_t colour;

    /* Precompute pixel width and height. */
    float dx=(float)(_xmax-_xmin)/(float)_xres;
    float dy=(float)(_ymax-_ymin)/(float)_yres;

    for (j = 0; j < _yres; j++) {
        y = (float)_ymax - (float)j * dy;
        for(i = 0; i < _xres; i++) {
            u = 0.0f;
            v = 0.0f;
            u2 = u * u;
            v2 = v * v;
            x = (float)_xmin + (float)i * dx;
            /* iterate the point */
            for (k = 1; k < _maxiter && (u2 + v2 < 4.0f); k++) {
                    v = 2 * u * v + y;
                    u = u2 - v2 + x;
                    u2 = u * u;
                    v2 = v * v;
            };

            colour = num_sym - ((k * num_sym) / _maxiter);
            switch (output) {
            #if DISPLAY_LCD5110
            case 1:
                if (colour > 0) Do_LCD_plot(i,j);
                break;
            #endif
            case 0:
            default:
                mp_printf(&mp_plat_print, "%c", symbols[colour]);
                break;
            };
        };
        switch (output) {
        #if DISPLAY_LCD5110
        case 1:
            break;
        #endif
        case 0:
        default:
            mp_printf(&mp_plat_print, "\n");
            break;
        };
    };
};


/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t fractals_info(void) {
    mp_printf(&mp_plat_print, "Compute fractals\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(fractals_info_obj, fractals_info);

// Print Mandelbrot to UART
STATIC mp_obj_t fractals_print_mandel(size_t n_args, const mp_obj_t *args) {
    if (n_args == 6) {  // If not the right number of arguments then default values
        _xmin = mp_obj_get_int(args[0]);
        _xmax = mp_obj_get_int(args[1]);
        _ymin = mp_obj_get_int(args[2]);
        _ymax = mp_obj_get_int(args[3]);
        _maxiter = mp_obj_get_int(args[4]);
        _xres = mp_obj_get_int(args[5]);
    } else {
        mp_printf(&mp_plat_print, "Default values.\n");
    }
    _yres = (_xres*(_ymax-_ymin))/(_xmax-_xmin);
    Do_Fractals_Print_Mandel(0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fractals_print_mandel_obj, 0, 6, fractals_print_mandel);

#if DISPLAY_LCD5110
STATIC mp_obj_t fractals_plot_mandel(size_t n_args, const mp_obj_t *args) {
    if (n_args == 5) {  // If not the right number of arguments then default values
        _xmin = mp_obj_get_int(args[0]);
        _xmax = mp_obj_get_int(args[1]);
        _ymin = mp_obj_get_int(args[2]);
        _ymax = mp_obj_get_int(args[3]);
        _maxiter = mp_obj_get_int(args[4]);
    } else {
        mp_printf(&mp_plat_print, "Default values.\n");
    }
    
    _xres = MAX_X;
    _yres = MAX_Y;
    Do_LCD_clear(0);
    Do_Fractals_Print_Mandel(1);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(fractals_plot_mandel_obj, 0, 5, fractals_plot_mandel);
#endif

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t fractals_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_fractals) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&fractals_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_print_mandel), MP_ROM_PTR(&fractals_print_mandel_obj) },
#if DISPLAY_LCD5110
    { MP_ROM_QSTR(MP_QSTR_plot_mandel), MP_ROM_PTR(&fractals_plot_mandel_obj) },
#endif
};
STATIC MP_DEFINE_CONST_DICT(fractals_module_globals, fractals_module_globals_table);

const mp_obj_module_t fractals_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&fractals_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_fractals, fractals_module, MICROPY_MODULE_FRACTALS);
/*

	Basic LCD 5110 (PCD8544) for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	5 June 2021
	
	Software provided under MIT License

  Project LCD 5110
  Based on work by Rei VILO on 28/05/12 (http://embeddedcomputing.weebly.com)

*/

#include <stdbool.h>
#include "py/objstr.h"
#include "modules/gpio.h"
#include "modules/time.h"
#include "modules/lcd5110Terml6.h"
#include "modules/lcd5110Term12.h"

#include "lcd5110.h"

#define HARDWARE_SPI (1)    // Hardware SPI uses SSI module of Tiva C, if this is set to 0, then bit banging is used
#define SPIPORT (0)         // Tiva C has 4 ports (0 to 3)
#define MAX_X (84)
#define MAX_Y (48)

#if HARDWARE_SPI
#include "modules/ssi.h"
#endif

uint8_t _pinReset;
uint8_t _pinSerialData;
uint8_t _pinBacklight;
uint8_t _pinChipSelect;
uint8_t _pinDataCommand;
uint8_t _pinSerialClock;
uint8_t _pinPushButton;

uint8_t _font;
bool LCD_InitDone = false;

uint8_t FrameBuffer[84][6];  // X * Y (Y being MAX_Y divided by 8 dot per byte)
//uint8_t CharBuffer[6*16];     // Line * Character

void Do_LCD_5110(uint8_t pinChipSelect, uint8_t pinSerialClock, uint8_t pinSerialData, uint8_t pinDataCommand, uint8_t pinReset, uint8_t pinBacklight, uint8_t pinPushButton) {
  // Set GPIO Pin values for global variables
  _pinChipSelect  = pinChipSelect;
  _pinSerialClock = pinSerialClock;
  _pinSerialData  = pinSerialData;
  _pinDataCommand = pinDataCommand;
  _pinReset       = pinReset;
  _pinBacklight   = pinBacklight;
  _pinPushButton  = pinPushButton;
}

void lcd_error_notinitialised () {
  // Send generic error
	  mp_printf(&mp_plat_print, "LCD not initialised.\n");
}

void Do_LCD_write(uint8_t dataCommand, uint8_t c) {
  // Send Command or Data via SPI (Soft or hard)
  // Wait time are required for LCD 5110
  Do_GPIO_write(_pinDataCommand, dataCommand);
  Do_SysTick_Waitus(10);
  #if HARDWARE_SPI
  Do_SSI_TX_FIFO(SPIPORT, c);        // Sending data to SSI 0
  #else
  Do_GPIO_write(_pinChipSelect, false);
  Do_SysTick_Waitus(10);
  Do_GPIO_shiftOut(_pinSerialData, _pinSerialClock, MSBFIRST, c);
  Do_SysTick_Waitus(10);
  Do_GPIO_write(_pinChipSelect, true);
  Do_SysTick_Waitus(10);
  #endif
}

void Do_LCD_setXY(uint8_t x, uint8_t y) {
  // Set position where to write data in memory (and to the screen)
  Do_LCD_write(COMMANDLCD, 0x40 | y);
  Do_SysTick_Waitus(5);
  Do_LCD_write(COMMANDLCD, 0x80 | x);
}

void Do_LCD_Init() {
  // Initialise the LCD 5110 (PCD8544)
  
  #if HARDWARE_SPI
  Do_LCD_5110(NOPIN,  // Chip Select
           NOPIN,     // Serial Clock
           NOPIN,     // Serial Data
           PB4,     // Data/Command
           PB0,     // Reset
           PB1,     // Backlight
           PF4);    // Push Button 2
  #else
  // Set pins
  Do_LCD_5110(PB5,  // Chip Select
           PB0,     // Serial Clock
           PB1,     // Serial Data
           PB4,     // Data/Command
           PB7,     // Reset
           PB6,     // Backlight
           PF4);    // Push Button 2
  #endif

  // Initialise MCU hardware
  Do_GPIO_Init();
  Do_SysTick_Init();

  #if HARDWARE_SPI
  Do_SSI_Init(SPIPORT,10041,false); // Using SSI 0 Master, SPI frame format, 4 Mbps, 8 data bits
  #else
  Do_GPIO_output(_pinSerialData);
  Do_GPIO_output(_pinSerialClock);
  Do_GPIO_output(_pinChipSelect);
  #endif
  Do_GPIO_output(_pinBacklight);
  Do_GPIO_input(_pinPushButton);
  Do_GPIO_output(_pinReset);
  Do_GPIO_output(_pinDataCommand);
  
  // Do a reset of the PCD8544
  Do_GPIO_write(_pinDataCommand, false);			
  Do_SysTick_Waitms(30);
  Do_GPIO_write(_pinReset, false);	
  Do_SysTick_Waitms(100); // as per 8.1 Initialisation
  Do_GPIO_write(_pinReset, true);
  
  // Set basic configuration
  Do_LCD_write(COMMANDLCD, 0x21); // chip is active, horizontal addressing, use extended instruction set
  Do_LCD_write(COMMANDLCD, 0xc8); // write VOP to register: 0xC8 for 3V — try other values
  Do_LCD_write(COMMANDLCD, 0x12); // set Bias System 1:48
  Do_LCD_write(COMMANDLCD, 0x07); // temperature control (TC0+TC1) Vlcd Temp Coef = 3
  Do_LCD_write(COMMANDLCD, 0x20); // chip is active, horizontal addressing, use basic instruction set
  Do_LCD_write(COMMANDLCD, 0x0c); // normal mode

  // TODO: create Do_LCD_setcontrast

  // Set default values
  Do_LCD_clear(true);
  _font = 0;
  Do_LCD_setBacklight(false);

  LCD_InitDone = true;
}

void Do_LCD_clear(bool black) {
  // Clear memory (and screen) with optional black pass first
  // Also erases framebuffer
  Do_LCD_setXY(0, 0);
  if (black == true) {
    for (uint16_t i=0; i<(MAX_Y/8)*MAX_X; i++) 
      Do_LCD_write(DATALCD, 0xFF);
     
    Do_LCD_setXY(0, 0);
  }
  for (uint16_t i=0; i<(MAX_Y/8)*MAX_X; i++) 
    Do_LCD_write(DATALCD, 0x00);
   
  Do_LCD_setXY(0, 0);

  // Erase the framebuffer
  for (uint8_t y=0; y<(MAX_Y/8); y++)
    for (uint8_t x=0; x<MAX_X; x++)
        FrameBuffer[x][y] = 0x00;
}

void Do_LCD_setBacklight(bool b) {
  Do_GPIO_write(_pinBacklight, b ? false : true);
}

void Do_LCD_setInverseVideo(bool iv) {
  if (iv) {
    Do_LCD_write(COMMANDLCD, 0x0d); // inverse video
  } else {
    Do_LCD_write(COMMANDLCD, 0x0c); // normal mode
  }
}

void Do_LCD_setFont(uint8_t font) {
  // Set font to use (see Header files and QSTR for details, 0 or 1 are two numerical values accepted)
  _font = font;
}

void Do_LCD_text(uint8_t x, uint8_t y, char* s) {
  // Convert characters to dots and send them to the PCD8544 memory (and screen)
  uint8_t i;
  uint8_t j;
  
  if (_font==0) {
    Do_LCD_setXY(6*x, y);
    for (j=0; j<strlen(s); j++) {
      for (i=0; i<5; i++) Do_LCD_write(DATALCD, Terminal6x8[((uint8_t)s[j]-0x20)][i]);
      Do_LCD_write(DATALCD, 0x00); // Add one space between characters
    } 
  } 
  else if (_font==1) { 
    Do_LCD_setXY(6*x, y);
    for (j=0; j<strlen(s); j++) {
      for (i=0; i<11; i++) Do_LCD_write(DATALCD, Terminal11x16[((uint8_t)s[j]-0x20)][2*i]);
      Do_LCD_write(DATALCD, 0x00);
    }
    
    Do_LCD_setXY(6*x, y+1);
    for (j=0; j<strlen(s); j++) {
      for (i=0; i<11; i++) Do_LCD_write(DATALCD, Terminal11x16[((uint8_t)s[j]-0x20)][2*i+1]);
      Do_LCD_write(DATALCD, 0x00);
    }
  }
}

void Do_LCD_plot(uint8_t x, uint8_t y) {
  // Add one dot to a memory position in the framebuffer and send it to the PCD8544
  // x and y are actual dots on the screen, not memory lines

  if (x < MAX_X && y < MAX_Y) {
    FrameBuffer[x][y/8] |= 0x01 << (y%8);
    Do_LCD_setXY(x,y/8);
    Do_LCD_write(DATALCD, FrameBuffer[x][y/8]);
  }
}

bool Do_LCD_getButton() {
  // Read value of a button with debounce
  if (Do_GPIO_read(_pinPushButton)==0) {
    while (Do_GPIO_read(_pinPushButton)==0); // debounce
    return true;
  } else {
    return false;
  }
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t lcd_info(void) {
  mp_printf(&mp_plat_print, "Control Nokia 5110 LCD. Initialise by lcd5110.init().\n");
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(lcd_info_obj, lcd_info);

// Initialise LCD
STATIC mp_obj_t lcd_init() {
	Do_LCD_Init();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(lcd_init_obj, lcd_init);

// Clear screen
STATIC mp_obj_t lcd_clear(mp_obj_t bl_obj) {
  bool bl = mp_obj_get_int(bl_obj);
  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_clear(bl);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lcd_clear_obj, lcd_clear);

// Set RAM position
STATIC mp_obj_t lcd_setXY(mp_obj_t x_obj, mp_obj_t y_obj) {
  uint8_t x = mp_obj_get_int(x_obj);
  uint8_t y = mp_obj_get_int(y_obj);
  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_setXY(x, y);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(lcd_setXY_obj, lcd_setXY);

// Write Data
STATIC mp_obj_t lcd_write(mp_obj_t dc_obj, mp_obj_t c_obj) {
  uint8_t dc = mp_obj_get_int(dc_obj);
  uint8_t c = mp_obj_get_int(c_obj);
  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_write(dc, c);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(lcd_write_obj, lcd_write);

// Print Text
STATIC mp_obj_t lcd_text(mp_obj_t x_obj, mp_obj_t y_obj, mp_obj_t s_obj) {
  uint8_t x = mp_obj_get_int(x_obj);
  uint8_t y = mp_obj_get_int(y_obj);
  mp_check_self(mp_obj_is_str_or_bytes(s_obj));
  GET_STR_DATA_LEN(s_obj, s, s_len);

  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_text(x, y, (char *)s);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(lcd_text_obj, lcd_text);

// Write a single dot on the LCD
STATIC mp_obj_t lcd_plot(mp_obj_t x_obj, mp_obj_t y_obj) {
  uint8_t x = mp_obj_get_int(x_obj);
  uint8_t y = mp_obj_get_int(y_obj);
  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_plot(x, y);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(lcd_plot_obj, lcd_plot);

// Turn on or off backlight
STATIC mp_obj_t lcd_backlight(mp_obj_t bl_obj) {
  bool bl = mp_obj_get_int(bl_obj);
  if (LCD_InitDone == false) {
    lcd_error_notinitialised();
	} else {
    Do_LCD_setBacklight(bl);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lcd_backlight_obj, lcd_backlight);

STATIC mp_obj_t lcd_inversevideo(mp_obj_t iv_obj) {
  bool iv = mp_obj_get_int(iv_obj);

  if (LCD_InitDone == false)
    lcd_error_notinitialised();
  else
    Do_LCD_setInverseVideo(iv);

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lcd_inversevideo_obj, lcd_inversevideo);

STATIC mp_obj_t lcd_setfont(mp_obj_t font_obj) {
  uint8_t font = mp_obj_get_int(font_obj);

  if (LCD_InitDone == false)
    lcd_error_notinitialised();
  else
    Do_LCD_setFont(font);

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lcd_setfont_obj, lcd_setfont);

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t lcd_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_lcd) },
    { MP_ROM_QSTR(MP_QSTR_small), MP_ROM_INT(SML) },
    { MP_ROM_QSTR(MP_QSTR_large), MP_ROM_INT(BIG) },
    { MP_ROM_QSTR(MP_QSTR_cmnd), MP_ROM_INT(COMMANDLCD) },
    { MP_ROM_QSTR(MP_QSTR_data), MP_ROM_INT(DATALCD) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&lcd_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&lcd_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_setXY), MP_ROM_PTR(&lcd_setXY_obj) },
    { MP_ROM_QSTR(MP_QSTR_setfont), MP_ROM_PTR(&lcd_setfont_obj) },
    { MP_ROM_QSTR(MP_QSTR_inversevideo), MP_ROM_PTR(&lcd_inversevideo_obj) },
    { MP_ROM_QSTR(MP_QSTR_backlight), MP_ROM_PTR(&lcd_backlight_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&lcd_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&lcd_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&lcd_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&lcd_plot_obj) },
};
STATIC MP_DEFINE_CONST_DICT(lcd_module_globals, lcd_module_globals_table);

const mp_obj_module_t lcd_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lcd_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lcd, lcd_module, MICROPY_MODULE_LCD5110);
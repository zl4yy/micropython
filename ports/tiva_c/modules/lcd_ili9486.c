/*

	Basic ILI9486 based TFT Display for Texas Instruments LM4F Microcontrollers
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
 	29 August 2021
	
	Software provided under MIT License

	Based on information from the following libraries:
	https://github.com/ImpulseAdventure/Arduino-TFT-Library-ILI9486
  and
  https://github.com/jaretburkett/ILI9488/blob/master/ILI9488.cpp
	
*/

#include <stdbool.h>
#include "py/objstr.h"
#include "modules/gpio.h"
#include "modules/time.h"

#include "lcd_ili9486.h"
#include "modules/ssi.h"

uint8_t _spiport_tft;
uint8_t _pinReset_tft;
uint8_t _pinChipSelect_tft;
uint8_t _pinDataCommand_tft;
uint8_t _rotation;
uint16_t _width, _height;

bool TFT_InitDone = false;


void tft_error_notinitialised () {
  // Send generic error
	  mp_printf(&mp_plat_print, "LCD not initialised.\n");
}

// Set the CS pin low
void Do_TFT_startWrite() {
  Do_GPIO_down(_pinChipSelect_tft);
  Do_SysTick_Waitus(5);
}

// Set the CS pin high
void Do_TFT_endWrite() {
  Do_SysTick_Waitus(5);
  Do_GPIO_up(_pinChipSelect_tft);
}

/* The following does not work
// Direct call to SSI driver to set speed
void Do_TFT_spiHighSpeed(bool high) {
  // This settings are hard coded, please adapt with your configuration
  if (high) {
    Do_SSI_Disable(3);
    ROM_SSIConfigSetExpClk(SSI3_BASE, 16000000, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 8000000, 4);
    Do_SSI_Enable(3);
    Do_TFT_ResetSequence();
  } else {
    Do_SSI_Disable(3);
    ROM_SSIConfigSetExpClk(SSI3_BASE, 16000000, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 2000000, 4);
    Do_SSI_Enable(3);
    Do_TFT_ResetSequence();
  }
}
*/

// Send 3 words data via SSI
void Do_TFT_TX_Colours(uint8_t ssinum, uint16_t word) {
  #if IS_ILI9488
  // ILI9488 needs 24 bits colours to be sent

  uint8_t r = (uint8_t)((word & 0xF800) >> 11);
  uint8_t g = (uint8_t)((word & 0x07E0) >> 5);
  uint8_t b = (uint8_t)(word & 0x001F);

  r = (r * 255) / 31;
  g = (g * 255) / 63;
  b = (b * 255) / 31;

  Do_SSI_TX_FIFO(ssinum,(uint32_t)r);
  Do_SSI_TX_FIFO(ssinum,(uint32_t)g);
  Do_SSI_TX_FIFO(ssinum,(uint32_t)b);
  #else
  Do_SSI_TX16_FIFO(ssinum, word, false);
  #endif
}

// Send commands (with the C/D pin low)
void Do_TFT_writeCommand(uint8_t command) {
  // Send Command or Data via SPI
  Do_GPIO_down(_pinDataCommand_tft);
  Do_SysTick_Waitus(5);
  Do_SSI_TX16_FIFO(_spiport_tft, command, false);
  Do_SysTick_Waitus(5);
  Do_GPIO_up(_pinDataCommand_tft);
}

// Send Chip base configuration sequence
void Do_TFT_ResetSequence() {
  // Set basic configuration
  Do_TFT_startWrite();

  Do_TFT_writeCommand(0x21); // chip is active, horizontal addressing, use extended instruction set

  //Driving ability Setting
  Do_TFT_writeCommand(0x11); // Sleep out, also SW reset
  Do_SysTick_Waitms(120);

  Do_TFT_writeCommand(0x3A); // Interface Pixel Format
  Do_SSI_TX16_FIFO(_spiport_tft, 0x55, false);

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0XC0);      //Power Control 1
	Do_SSI_TX16_FIFO(_spiport_tft, 0x17, false);    //Vreg1out
	Do_SSI_TX16_FIFO(_spiport_tft, 0x15, false);    //Verg2out

	Do_TFT_writeCommand(0xC1);      //Power Control 2
	Do_SSI_TX16_FIFO(_spiport_tft, 0x41, false);    //VGH,VGL

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0xC2); // Power Control 3 (For Normal Mode)
  Do_SSI_TX16_FIFO(_spiport_tft, 0x44, false);

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0xC5); // VCOM Control
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0F, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x1F, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x1C, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0C, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0F, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x08, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x48, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x98, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x37, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0A, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x13, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x04, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x11, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0D, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0F, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x32, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x2E, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0B, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x0D, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x05, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x47, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x75, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x37, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x06, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x10, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x03, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x24, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x20, false);
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);

  Do_TFT_writeCommand(0x20); // Display Inversion OFF   RPi LCD (A)
// Do_TFT_writeCommand(0x21); // Display Inversion ON    RPi LCD (B)

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0x36); // Memory Access Control
  Do_SSI_TX16_FIFO(_spiport_tft, 0x48, false);

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0x3A);      // Interface Pixel Format
  Do_SSI_TX16_FIFO(_spiport_tft, 0x66, false);  //18 bit

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(0XB0);      // Interface Mode Control
  Do_SSI_TX16_FIFO(_spiport_tft, 0x80, false);  //SDO not used

  Do_TFT_writeCommand(0x29); // Display ON
  Do_SysTick_Waitms(150);

  Do_TFT_writeCommand(0XE9);      // Set Image Functio
  Do_SSI_TX16_FIFO(_spiport_tft, 0x00, false);  // Disable 24 bit data

  Do_TFT_endWrite();
}

// Initialise the TFT display
void Do_TFT_Init(uint8_t spiport, uint8_t pinChipSelect, uint8_t pinDataCommand, uint8_t pinReset) {

  _spiport_tft        = spiport;
  _pinChipSelect_tft  = pinChipSelect;
  _pinDataCommand_tft = pinDataCommand;
  _pinReset_tft       = pinReset;

  _rotation = 0;
  _width = TFT_WIDTH;
  _height = TFT_HEIGHT;
  // Initialise MCU hardware
  Do_GPIO_Init();

	Do_GPIO_output(_pinDataCommand_tft);
	Do_GPIO_output(_pinDataCommand_tft);
	Do_GPIO_output(_pinReset_tft);
  Do_GPIO_up(_pinReset_tft);
  Do_GPIO_up(_pinDataCommand_tft);
  Do_GPIO_up(_pinChipSelect_tft);
  Do_SysTick_Waitms(100);

  // Reset does not seem to have any effet
  Do_GPIO_down(_pinReset_tft);
  Do_SysTick_Waitms(10);
  Do_GPIO_up(_pinReset_tft);
  Do_SysTick_Waitms(10);

	//Do_SSI_Init(_spiport_tft,20091,false);
	Do_SSI_Init(_spiport_tft,10031,false);
  Do_SysTick_Waitms(10);

  Do_TFT_ResetSequence();

  TFT_InitDone = true;
}

// Clear the whole screen with a background colour
void Do_TFT_clear(uint16_t colour) {
  Do_TFT_startWrite();
  Do_TFT_setAddrWindow(0,0,_width,_height);
  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(ILI9486_RAMWR);
  for (uint16_t x=0; x < _width; x++) {
    for (uint16_t y=0; y < _height; y++) {
        Do_TFT_TX_Colours(_spiport_tft, colour);
    }
  }
  Do_TFT_endWrite();
}

// Change screen orientation
void Do_TFT_setRotation(uint8_t m) {
    _rotation = m % 4; // can't be higher than 3
       Do_TFT_startWrite();
    Do_TFT_writeCommand(ILI9486_MADCTL);

    switch (_rotation) {
        case 0:
            Do_SSI_TX16_FIFO(_spiport_tft, MADCTL_MX | MADCTL_BGR, false);
            _width  = TFT_WIDTH;
            _height = TFT_HEIGHT;
            break;
        case 1:
            Do_SSI_TX16_FIFO(_spiport_tft, MADCTL_MV | MADCTL_BGR, false);
            _width  = TFT_HEIGHT;
            _height = TFT_WIDTH;
            break;
        case 2:
            Do_SSI_TX16_FIFO(_spiport_tft, MADCTL_MY | MADCTL_BGR, false);
            _width  = TFT_WIDTH;
            _height = TFT_HEIGHT;
            break;
        case 3:
            Do_SSI_TX16_FIFO(_spiport_tft, MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR, false);
            _width  = TFT_HEIGHT;
            _height = TFT_WIDTH;
            break;
    }
    Do_TFT_endWrite();
}

// Set brigthness (Not working on my display)
void Do_TFT_setBrightness(uint8_t brightness) {
    Do_TFT_startWrite();
#if IS_ILI9488
    Do_TFT_writeCommand(0x53);
//    Do_SSI_TX_FIFO(_spiport_tft, 0x0, false);
    Do_SSI_TX16_FIFO(_spiport_tft, brightness, false);
    Do_TFT_endWrite();
#else
    Do_TFT_writeCommand(ILI9486_CDBVAL);
    Do_SSI_TX16_FIFO(_spiport_tft, 0x16, false);
    Do_TFT_writeCommand(ILI9486_WDBVAL);
    Do_SSI_TX16_FIFO(_spiport_tft, brightness, false);
    Do_TFT_endWrite();
#endif
}

// Set inverse video
void Do_TFT_setInverseVideo(bool iv) {

  Do_TFT_startWrite();
  if (iv == true) {
    Do_TFT_writeCommand(0x21); // Display Inversion ON    RPi LCD (B)
  } else {
    Do_TFT_writeCommand(0x20); // Display Inversion OFF   RPi LCD (A)
  }
  Do_TFT_endWrite();
}

void Do_TFT_setFont(uint8_t font) {
}

void Do_TFT_text(uint8_t x, uint8_t y, char* s) {
}

// Set memory boundaries for update
void Do_TFT_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  Do_TFT_writeCommand(ILI9486_CASET); // Column addr set
  Do_SysTick_Waitms(10);
  Do_SSI_TX16_FIFO(_spiport_tft, ((x >> 8)&0xff), false);
  Do_SSI_TX16_FIFO(_spiport_tft, (x & 0xFF), false);     // XSTART
  w=x+w-1;
  Do_SSI_TX16_FIFO(_spiport_tft, ((w >> 8)&0xff), false);
  Do_SSI_TX16_FIFO(_spiport_tft, (w & 0xFF), false);     // XEND

  Do_SysTick_Waitms(10);
  Do_TFT_writeCommand(ILI9486_PASET); // Row addr set
  Do_SysTick_Waitms(10);
  Do_SSI_TX16_FIFO(_spiport_tft, ((y >> 8)&0xff), false);
  Do_SSI_TX16_FIFO(_spiport_tft, (y & 0xFF), false);     // YSTART
  h=y+h-1;
  Do_SSI_TX16_FIFO(_spiport_tft, ((h >> 8)&0xff), false);
  Do_SSI_TX16_FIFO(_spiport_tft, (h & 0xFF), false);     // YEND
  Do_SysTick_Waitms(10);
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed colour
uint16_t Do_TFT_colour565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

// Draw one single dot
void Do_TFT_plot(uint16_t x, uint16_t y, uint16_t colour) {
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    Do_TFT_startWrite();
    Do_TFT_setAddrWindow(x,y,1,1);  // Window is 4 by 4 but we will only write one byte
    Do_TFT_writeCommand(ILI9486_RAMWR);
    Do_TFT_TX_Colours(_spiport_tft, colour);
    Do_SSI_TX_FIFO(_spiport_tft,0x00);  // Send one byte to flush GRAM buffer
    Do_TFT_writeCommand(0x00);  // Send NOOP commands to stop writing
    Do_TFT_endWrite();
}

/*

    MicroPython Wrappers

*/

// Display basic help
STATIC mp_obj_t tft_info(void) {
  mp_printf(&mp_plat_print, "ILI9486 TFT Display Driver.\n");
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(tft_info_obj, tft_info);

// Initialise LCD
STATIC mp_obj_t tft_init(size_t n_args, const mp_obj_t *args) {
  uint8_t spiport = mp_obj_get_int(args[0]);
  uint8_t pinChipSelect = mp_obj_get_int(args[1]);
  uint8_t pinDataCommand = mp_obj_get_int(args[2]);
  uint8_t pinReset = mp_obj_get_int(args[3]);
	Do_TFT_Init(spiport, pinChipSelect, pinDataCommand, pinReset);
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tft_init_obj, 4, tft_init);

/*
// Set display orientation
STATIC mp_obj_t tft_setFastTransfer(mp_obj_t highspeed_obj) {
  bool highspeed = mp_obj_get_int(highspeed_obj);
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_spiHighSpeed(highspeed);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_setFastTransfer_obj, tft_setFastTransfer);*/

// Clear screen
STATIC mp_obj_t tft_clear(mp_obj_t colour_obj) {
  uint16_t colour = mp_obj_get_int(colour_obj);
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_clear(colour);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_clear_obj, tft_clear);

// Set display orientation
STATIC mp_obj_t tft_setOrientation(mp_obj_t orientation_obj) {
  uint8_t orientation = mp_obj_get_int(orientation_obj);
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_setRotation(orientation);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_setOrientation_obj, tft_setOrientation);

// Set RAM position
STATIC mp_obj_t tft_setWindow(size_t n_args, const mp_obj_t *args) {
  uint16_t x = mp_obj_get_int(args[0]);
  uint16_t y = mp_obj_get_int(args[1]);
  uint16_t w = mp_obj_get_int(args[2]);
  uint16_t h = mp_obj_get_int(args[3]);
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_startWrite();
    Do_TFT_setAddrWindow(x, y, w, h);
    Do_TFT_endWrite();
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tft_setWindow_obj, 4, tft_setWindow);

// Write Data
STATIC mp_obj_t tft_write(mp_obj_t dc_obj, mp_obj_t w_obj) {
  bool dc = mp_obj_get_int(dc_obj);
  uint16_t word = mp_obj_get_int(w_obj);
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_startWrite();
    if (dc == true) {
      Do_TFT_writeCommand(word); // Send command
    } else {
      Do_SSI_TX16_FIFO(_spiport_tft, word, false);
    }
    Do_TFT_endWrite();
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(tft_write_obj, tft_write);

/*
// Print Text
STATIC mp_obj_t tft_text(mp_obj_t x_obj, mp_obj_t y_obj, mp_obj_t s_obj) {
  uint8_t x = mp_obj_get_int(x_obj);
  uint8_t y = mp_obj_get_int(y_obj);
  mp_check_self(mp_obj_is_str_or_bytes(s_obj));
  GET_STR_DATA_LEN(s_obj, s, s_len);

  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_text(x, y, (char *)s);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(tft_text_obj, tft_text);
*/

// Write a single dot on the LCD
STATIC mp_obj_t tft_plot(size_t n_args, const mp_obj_t *args) {
  uint16_t x = mp_obj_get_int(args[0]);
  uint16_t y = mp_obj_get_int(args[1]);
  uint16_t colour = mp_obj_get_int(args[2]);

  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_plot(x, y, colour);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tft_plot_obj, 3, tft_plot);

STATIC mp_obj_t tft_fill(size_t n_args, const mp_obj_t *args) {
  uint16_t x = mp_obj_get_int(args[0]);
  uint16_t y = mp_obj_get_int(args[1]);
  uint16_t w = mp_obj_get_int(args[2]);
  uint16_t h = mp_obj_get_int(args[3]);
  uint16_t colour = mp_obj_get_int(args[4]);

  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_startWrite();
    Do_TFT_setAddrWindow(x, y, w, h);
    Do_TFT_writeCommand(ILI9486_RAMWR);
    for (uint16_t i=0; i < w; i++) {
      for (uint16_t j=0; j < h; j++){
        Do_TFT_TX_Colours(_spiport_tft, colour);
      }
    }
    Do_TFT_endWrite();
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tft_fill_obj, 5, tft_fill);

// Return 565 packed colour
STATIC mp_obj_t tft_get565colour(size_t n_args, const mp_obj_t *args) {
  uint8_t r = mp_obj_get_int(args[0]);
  uint8_t g = mp_obj_get_int(args[1]);
  uint8_t b = mp_obj_get_int(args[2]);
  uint16_t colour = TFT_BLACK;

  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    colour = Do_TFT_colour565(r, g, b);
	}
  return mp_obj_new_int((int32_t)colour);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR(tft_get565colour_obj, 3, tft_get565colour);

// Turn on or off backlight
STATIC mp_obj_t tft_setBrightness(mp_obj_t br_obj) {
  uint8_t br = mp_obj_get_int(br_obj);
  if (br>255) br = 255;
  if (TFT_InitDone == false) {
    tft_error_notinitialised();
	} else {
    Do_TFT_setBrightness(br);
	}
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_setBrightness_obj, tft_setBrightness);

STATIC mp_obj_t tft_inversevideo(mp_obj_t iv_obj) {
  bool iv = mp_obj_get_int(iv_obj);

  if (TFT_InitDone == false)
    tft_error_notinitialised();
  else
    Do_TFT_setInverseVideo(iv);

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_inversevideo_obj, tft_inversevideo);

/*
STATIC mp_obj_t tft_setfont(mp_obj_t font_obj) {
  uint8_t font = mp_obj_get_int(font_obj);

  if (TFT_InitDone == false)
    tft_error_notinitialised();
  else
    Do_TFT_setFont(font);

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(tft_setfont_obj, tft_setfont);
*/

// This section does the mapping between C functions and MicroPython in the module
STATIC const mp_rom_map_elem_t tft_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_tft) },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&tft_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&tft_init_obj) },
//    { MP_ROM_QSTR(MP_QSTR_setFastTransfer), MP_ROM_PTR(&tft_setFastTransfer_obj) },
    { MP_ROM_QSTR(MP_QSTR_setWindow), MP_ROM_PTR(&tft_setWindow_obj) },
    { MP_ROM_QSTR(MP_QSTR_setOrientation), MP_ROM_PTR(&tft_setOrientation_obj) },
//    { MP_ROM_QSTR(MP_QSTR_setfont), MP_ROM_PTR(&tft_setfont_obj) },
    { MP_ROM_QSTR(MP_QSTR_inversevideo), MP_ROM_PTR(&tft_inversevideo_obj) },
    { MP_ROM_QSTR(MP_QSTR_setBrightness), MP_ROM_PTR(&tft_setBrightness_obj) },
    { MP_ROM_QSTR(MP_QSTR_get565colour), MP_ROM_PTR(&tft_get565colour_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&tft_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&tft_write_obj) },
//    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&tft_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_plot), MP_ROM_PTR(&tft_plot_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&tft_fill_obj) },
};
STATIC MP_DEFINE_CONST_DICT(tft_module_globals, tft_module_globals_table);

const mp_obj_module_t tft_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&tft_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_tft, tft_module, MICROPY_MODULE_LCD_ILI9486);
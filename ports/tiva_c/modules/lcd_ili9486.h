/*

	Headers
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

#ifndef LCD_ILI9486_H_
#define LCD_ILI9486_H_

#include <stdbool.h>

#define IS_ILI9488      (1)

#define TFT_WIDTH       320
#define TFT_HEIGHT      480

#define ILI9486_INVOFF  0x20 // Display Inversion OFF
#define ILI9486_INVON   0x21 // Display Inversion ON
#define ILI9486_CASET   0x2A // Display On
#define ILI9486_PASET   0x2B // Page Address Set
#define ILI9486_RAMWR   0x2C // Memory Write
#define ILI9486_MADCTL  0x36 // Memory Data Access Control
    #define MADCTL_MY   0x80 // Bit 7 Parameter MADCTL
    #define MADCTL_MX   0x40 // Bit 6 Parameter MADCTL
    #define MADCTL_MV   0x20 // Bit 5 Parameter MADCTL
    #define MADCTL_ML   0x10 // Bit 4 Parameter MADCTL
    #define MADCTL_BGR  0x08 // Bit 3 Parameter MADCTL
    #define MADCTL_MH   0x04 // Bit 2 Parameter MADCTL
#define ILI9486_WDBVAL  0x51 // Write Display Brightness Value
#define ILI9486_CDBVAL  0x53 // Write Control Display Value

// RGB565 Color definitions
#define TFT_AQUAMARINE      0x7FFA // 127, 255, 212
#define TFT_BEIGE           0xF7BB // 245, 245, 220
#define TFT_BLACK           0x0000 //   0,   0,   0
#define TFT_BLUE            0x001F //   0,   0, 255
#define TFT_BROWN           0xA145 // 165,  42,  42
#define TFT_CHOCOLATE       0xD343 // 210, 105,  30
#define TFT_CORNSILK        0xFFDB // 255, 248, 220
#define TFT_CYAN            0x07FF //   0, 255, 255
#define TFT_DARKGREEN       0x0320 //   0, 100,   0
#define TFT_DARKGREY        0xAD55 // 169, 169, 169
#define TFT_DARKCYAN        0x0451 //   0, 139, 139
#define TFT_DEEPSKYBLUE     0x05FF //   0, 191, 255
#define TFT_GRAY            0x8410 // 128, 128, 128
#define TFT_GREEN           0x0400 //   0, 128,   0
#define TFT_GREENYELLOW     0xAFE5 // 173, 255,  47
#define TFT_GOLD            0xFEA0 // 255, 215,   0
#define TFT_HOTPINK         0xFB56 // 255, 105, 180
#define TFT_LAVENDER        0xE73F // 230, 230, 250
#define TFT_LAWNGREEN       0x7FE0 // 124, 252,   0
#define TFT_LIGHTBLUE       0xAEDC // 173, 216, 230
#define TFT_LIGHTCYAN       0xE7FF // 224, 255, 255
#define TFT_LIGHTGREY       0xD69A // 211, 211, 211
#define TFT_LIGHTGREEN      0x9772 // 144, 238, 144
#define TFT_LIGHTYELLOW     0xFFFC // 255, 255, 224
#define TFT_LIME            0x07E0 //   0. 255,   0
#define TFT_MAGENTA         0xF81F // 255,   0, 255
#define TFT_MAROON          0x7800 // 128,   0,   0
#define TFT_MEDIUMVIOLETRED 0xC0B0 // 199,  21, 133
#define TFT_NAVY            0x000F //   0,   0, 128
#define TFT_OLIVE           0x7BE0 // 128, 128,   0
#define TFT_ORANGE          0xFD20 // 255, 165,   0
#define TFT_PINK            0xFE19 // 255, 192, 203
#define TFT_PURPLE          0x780F // 128,   0, 128
#define TFT_RED             0xF800 // 255,   0,   0
#define TFT_SANDYBROWN      0xF52C // 244, 164,  96
#define TFT_TURQUOISE       0x471A //  64, 224, 208
#define TFT_VIOLET          0x801F // 128,   0, 255
#define TFT_WHITE           0xFFFF // 255, 255, 255
#define TFT_YELLOW          0xFFE0 // 255, 255,   0


// Set the CS pin low
void Do_TFT_startWrite();

// Set the CS pin high
void Do_TFT_endWrite();

// Send 3 words data via SSI
void Do_TFT_TX_Colours(uint8_t ssinum, uint16_t word);

// Send Chip base configuration sequence
void Do_TFT_ResetSequence();

// Send commands (with the C/D pin low)
void Do_TFT_writeCommand(uint8_t c);

// Initialise the TFT display
void Do_TFT_Init(uint8_t spiport, uint8_t pinChipSelect, uint8_t pinDataCommand, uint8_t pinReset);

// Direct call to SSI driver to set speed
void Do_TFT_spiHighSpeed(bool high);

// Clear the whole screen with a background colour
void Do_TFT_clear(uint16_t colour);

// Change screen orientation
void Do_TFT_setRotation(uint8_t m);

// Set brigthness (Not working)
void Do_TFT_setBrightness(uint8_t brightness);

// Set inverse video
void Do_TFT_setInverseVideo(bool iv);

void Do_TFT_setFont(uint8_t font);

void Do_TFT_text(uint8_t x, uint8_t y, char* s);

// Set memory boundaries for update
void Do_TFT_setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

// Pass 8-bit (each) R,G,B, get back 16-bit packed colour
uint16_t Do_TFT_colour565(uint8_t r, uint8_t g, uint8_t b);

// Draw one single dot
void Do_TFT_plot(uint16_t x, uint16_t y, uint16_t colour);



#endif
/*

Blue pill board (STM32F103C8T6)
TFT 2.4 SPI display
Membrane keyboard 5x4 switch
Arduino Software (IDE) + STM32 hardware core (https://github.com/rogerclarkmelbourne/Arduino_STM32)


Mělo by obsahovat...
Menu pro nastavení (baudrate, ascii nebo dec kód).
Scrolování displeje.
Použití Rx ale i Tx (klávesnice 5x4).

*/

// hw sclk   PA5
// hw mosi   PA7
#define cs   PA3
#define dc   PA1
#define rst  PA2

#include <Adafruit_GFX_AS.h>    // Core graphics library, with extra fonts.
#include <Adafruit_ILI9341_STM.h> // STM32 DMA Hardware-specific library
#include <SPI.h>

Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(cs, dc, rst);       // Invoke custom library


// ...
// ...
// to be continued :-) 

// (c)2016 Pawel A. Hernik
// code for the videos:
// https://www.youtube.com/watch?v=OgaLXoLhz4g
// https://www.youtube.com/watch?v=DAAbDGCeQ1o
/*
CONNECTIONS:
------------
STM32:
For programming via serial:
Tools/Board set to Generic STM32F103C
Tools/Upload set to Serial
Top jumper set to 1, press the button before uploading
  PA9 /TX to PC RX (VIOLET)
  PA10/RX to PC TX (GREY)
  3V3              (RED)
  GND              (BLUE)
 STM32 SPI1 pins:
  PA4 CS1
  PA5 SCK1
  PA6 MISO1
  PA7 MOSI1
  PA11 RST
  PA12 DC
TFT2.2 ILI9341 from top left:
  MISO  PA6
  LED   +3.3V
  SCK   PA5
  MOSI  PA7
  DC    PA12
  RST   PA11 or +3V3
  CS    PA4
  GND   GND
  VCC   +3.3V
*/
#include <Arduino.h>
#include "SPI.h"

#include <Adafruit_GFX_AS.h>    // Core graphics library, with extra fonts.
#include <Adafruit_ILI9341_STM.h> // STM32 DMA Hardware-specific library

#define ILI9341_VSCRDEF  0x33
#define ILI9341_VSCRSADD 0x37

int xp = 0;
int yp = 0;
uint16_t bg = ILI9341_BLACK;
uint16_t fg = ILI9341_WHITE;
int screenWd = 240;
int screenHt = 320;
int wrap = 0;
int bold = 0;
int sx = 1;
int sy = 1;
int horizontal = -1;
int scrollMode = 1;

#define WRAP_PIN    PB9
#define HORIZ_PIN   PB8
#define TFT_CS      PA4                  
#define TFT_DC      PA12              
#define TFT_RST     PA11 
Adafruit_ILI9341_STM tft = Adafruit_ILI9341_STM(TFT_CS, TFT_DC, TFT_RST); // Use hardware SPI

// new variables
bool deviceSetting; // 0 - device is't set;
byte viewMode; // 0 - view as dec (only number); 2 - view as char (all char); 3 - view to terminal ;-) 


// Uncomment below the font you find the most readable for you
// 7x8 bold - perfect for small term font
//#include "font_b7x8.h"
//const uint16_t *fontRects = font_b7x8_Rects;
//const uint16_t *fontOffs = font_b7x8_CharOffs;
//int charWd = 7;
//int charHt = 10; // real 8
//int charYoffs = 1;

// 7x8 - perfect for small terminal font
//#include "font_7x8.h"
//const uint16_t *fontRects = font_7x8_Rects;
//const uint16_t *fontOffs = font_7x8_CharOffs;
//int charWd = 7;
//int charHt = 10; // real 8
//int charYoffs = 1;

// 6x8
//#include "font_6x8.h"
//const uint16_t *fontRects = font_6x8_Rects;
//const uint16_t *fontOffs = font_6x8_CharOffs;
//int charWd = 6;
//int charHt = 9; // real 8
//int charYoffs = 1;

// nice 8x16 vga terminal font
#include "font_term_8x16.h"
const uint16_t *fontRects = wlcd_font_term_8x16_0_127_Rects;
const uint16_t *fontOffs = wlcd_font_term_8x16_0_127_CharOffs;
int charWd = 8;
int charHt = 16;
int charYoffs = 0;

// nice big for terminal
//#include "font_fxs_8x15.h"
//const uint16_t *fontRects = wlcd_font_fxs_8x15_16_127_Rects;
//const uint16_t *fontOffs = wlcd_font_fxs_8x15_16_127_CharOffs;
//int charWd = 8;
//int charHt = 15; // real 15
//int charYoffs = 0;

// my nice 10x16 term
//#include "font_term_10x16.h"
//const uint16_t *fontRects = font_term_10x16_Rects;
//const uint16_t *fontOffs = font_term_10x16_CharOffs;
//int charWd = 10;
//int charHt = 16;
//int charYoffs = 0;


void drawDec(int16_t x, int16_t y, unsigned int d, uint8_t sx, uint8_t sy){ // zobrazeni dat ve formatu "dec" 
  ;
}


// send "char" to display
void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t sx, uint8_t sy){ //---- odstranit/neresit(!!!) "bg" a "color" ----//
  if((x >= screenWd)              || // Clip right
     (y >= screenHt)              || // Clip bottom
     ((x + charWd * sx - 1) < 0)  || // Clip left
     ((y + charHt * sy - 1) < 0))    // Clip top
    return;
  if(c>127) return; //---- budou jenom "char" (nebo "dec") ----//
  uint16_t recIdx = fontOffs[c];
  uint16_t recNum = fontOffs[c+1]-recIdx;
  if(bg && bg!=color) tft.fillRect(x, y, charWd*sx, charHt*sy, bg); //---- "bg" bude stale white!!! ----//
  if(charWd<=16 && charHt<=16)
    for(int i=0; i<recNum; i++) {
      int v = fontRects[i+recIdx];
      int xf = v & 0xf;
      int yf = charYoffs+((v & 0xf0)>>4);
      int wf = 1+((v & 0xf00)>>8);
      int hf = 1+((v & 0xf000)>>12);
      tft.fillRect(x+xf*sx, y+yf*sy, bold+wf*sx, hf*sy, color);
    }
  else
    for(int i=0; i<recNum; i++) {
      uint8_t *rects = (uint8_t*)fontRects;
      int idx = (i+recIdx)*3;
      int xf = rects[idx+0] & 0x3f;
      int yf = rects[idx+1] & 0x3f;
      int wf = 1+rects[idx+2] & 0x3f;
      int hf = 1+(((rects[idx+0] & 0xc0)>>6) | ((rects[idx+1] & 0xc0)>>4) | ((rects[idx+2] & 0xc0)>>2));
      tft.fillRect(x+xf*sx, y+yf*sy, bold+wf*sx, hf*sy, color);
    }
}


void scroll(){
  xp=0;
  yp+=charHt*sy;
  if(yp+charHt>screenHt) yp=0;
  tft.fillRect(0, yp, screenWd, charHt*sy, ILI9341_BLACK);
  if(scrollMode)
    scrollFrame(320-yp-charHt*sy);
  else
    scrollFrame(0);
}

int escMode = 0;
int nVals = 0;
int vals[10]={0};


void printChar(char c){
  if(c==0x1b) { escMode=1; return; } // Esc
  if(escMode==1) {
    if(c=='[') { escMode=2; nVals=0; } else escMode=0;
    return; 
  }
  if(escMode==2) {
    if(isdigit(c))
      vals[nVals] = vals[nVals]*10+(c-'0');
    else if(c==';')
      nVals++;
    else if(c=='m') {
      escMode=0;
      nVals++;
      for(int i=0;i<nVals;i++) {
        int v = vals[i];
        static const uint16_t colors[] = {
              0x0000, // 0-black
              0xf800, // 1-red
              0x0780, // 2-green
              0xfe00, // 3-yellow
              0x001f, // 4-blue
              0xf81f, // 5-magenta
              0x07ff, // 6-cyan
              0xffff  // 7-white
        };
        if(v == 0){ // all attributes off
          if(nVals==1) {
            fg = ILI9341_WHITE;
            bg = ILI9341_BLACK;
          }
          bold = 0;
        } else
        if(v == 1){ // all attributes off
          bold = 1;
        } else
        if(v >= 30 && v < 38){ // fg colors
          fg = colors[v-30]; 
        } else if(v >= 40 && v < 48){
          bg = colors[v-40]; 
        }          
      }
      vals[0]=vals[1]=vals[2]=vals[3]=0;
      nVals=0;
    } else {
      escMode=0;
      vals[0]=vals[1]=vals[2]=vals[3]=0;
      nVals=0;
    }
    return;
  }
  if(c==10) { scroll(); return; } // LF - next line
  if(c==13) { xp=0; return; } // CR - return to firs position...
  if(c==8) { // back space... uf... :-D
    if(xp>0) xp-=charWd*sx; 
    tft.fillRect(xp, yp, charWd*sx, charHt*sy, ILI9341_BLACK); // send to display
    return; 
  }
  if(xp<screenWd)
    drawChar(xp, yp, c, fg, bg, sx, sy);
  xp+=charWd*sx;
  if(xp>=screenWd && wrap) scroll();
}


// only for Title(?)...
void printString(char *str)
{
  while(*str) printChar(*str++);
}


void setupScroll(uint16_t tfa, uint16_t bfa){
  tft.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
  tft.writedata(tfa >> 8);
  tft.writedata(tfa);
  tft.writedata((320 - tfa - bfa) >> 8);
  tft.writedata(320 - tfa - bfa);
  tft.writedata(bfa >> 8);
  tft.writedata(bfa);
}


void scrollFrame(uint16_t vsp) 
{
  tft.writecommand(ILI9341_VSCRSADD); // Vertical scrolling start address
  tft.writedata(vsp >> 8);
  tft.writedata(vsp);
}

/*void checkButtons() // not used
{
  wrap = digitalRead(WRAP_PIN) ? 0 : 1;
  int orient = digitalRead(HORIZ_PIN) ? 0 : 1;
  if(orient!=horizontal) {
    horizontal = orient;
    scrollMode = horizontal ? 0 : 1;
    tft.setRotation(horizontal ? 1 : 2);
    screenWd = tft.width();
    screenHt = tft.height();
  }
}*/


void setup(){
  
  Serial.begin(115200);
  pinMode(WRAP_PIN,  INPUT_PULLUP);
  pinMode(HORIZ_PIN, INPUT_PULLUP);
  tft.begin();
//  tft.setRotation(2);
  setupScroll(0, 0);
  //checkButtons(); // not used
  
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  sx = 1;
  sy = 2;
  //printString("\e[0;44m *** Terminal Init *** \e[0m\n"); // modified
  printString("\e[0;44m___uart_terminal___\e[0m\n"); 
  sy = 1;
  
} 


void loop(){
  
  //checkButtons(); // not used
  
  // view setting menu
  if(!deviceSetting){
    // view menu .....
    
    // default setting
    viewMode = 0; 
    deviceSetting = 1;
  }
  
  // reading serial data
  if(viewMode == 1){ // dec
    while(Serial.available()){
      printDec(Serial.read());
    }
  }
  else if(viewMode == 2){ // char
    while(Serial.available()){
      printOnlyChar(Serial.read());
    }
  }
  else{ // terminal
    while(Serial.available()){
      printChar(Serial.read());
    }
  } 
  
  
  // reading Menu button status and next change "deviceSetting" value for view setting menu 
  if(buttonMenu && deviceSetting){ // buttonMenu = GPIOX->regs->IDR & (1<<Y) = Port X, pin number Y
    deviceSetting = 0;
  }
  
} 

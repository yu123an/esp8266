#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Arduino_ST7789.h> // Hardware-specific library for ST7789 (with or without CS pin)
#include <SPI.h>

#define TFT_DC    14
#define TFT_RST   12 
//#define TFT_CS    10 // only for displays with CS pin
#define TFT_MOSI  4   // for hardware SPI data pin (all of available pins)
#define TFT_SCLK  5   // for hardware SPI sclk pin (all of available pins)

Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK); //for display without CS pin

float p = 3.1415926;
int nn = 0;
int n;
void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! ST7789 TFT Test");

  tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels

  Serial.println("Initialized");

  
}
uint16_t cc[10] = {BLACK,RED,YELLOW,GREEN,BLUE,WHITE};
void loop() {
n = nn % 5;
tft.fillRect(0,0,80,240,cc[n]);
delay(10);
tft.fillRect(80,0,80,240,cc[n]);
delay(10);
tft.fillRect(160,0,80,240,cc[n]);
 delay(10);
  testdrawtext("ABCDEFGHIJKLMNOPQabcdefg",WHITE);
  delay(100);
  /*tft.invertDisplay(true);
  delay(500);
  tft.invertDisplay(false);
  delay(500);*/
  nn++;
}

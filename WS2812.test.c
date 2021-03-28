#include <Adafruit_NeoPixel.h>
#define PIN            14
#define NUMPIXELS      320
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int colorR;
int colorG;
int colorB;
void pixelShow()
{
  pixels.setBrightness(50);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colorR, colorG, colorB);
  }
  pixels.show();
}
void setup() {
  // put your setup code here, to run once:
pixels.begin();
  pixels.setBrightness(10);
  pixelShow();
 Serial.begin(115200);
}

void loop() {
 for(int i = 0;i<320;i++){
  pixels.setPixelColor(i,0,0,10);
 }
  pixels.show();
  delay(500);
   for(int i = 0;i<320;i++){
  pixels.setPixelColor(i,0,10,0);
 }
  pixels.show();
  delay(500);
   for(int i = 0;i<320;i++){
  pixels.setPixelColor(i,10,0,0);
 }
  pixels.show();
  delay(500);
}

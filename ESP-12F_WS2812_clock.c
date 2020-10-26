#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define PIN            4
#define NUMPIXELS      256
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const char *ssid     = "WiFi_Name";         //WiFi名称
const char *password = "WiFi_Pass";   //WiFi密码
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
uint8_t litnumber[10][4] = {
  {0x00, 0xf8, 0x11, 0xf8}, //litnum0
  {0x00, 0x90, 0x1f, 0x80}, //litnum1
  {0x00, 0xe8, 0x15, 0xb8}, //litnum2
  {0x00, 0xa8, 0x15, 0xf8}, //litnum3
  {0x00, 0x38, 0x04, 0xf8}, //litnum4
  {0x00, 0xb8, 0x15, 0xe8}, //litnum5
  {0x00, 0xf8, 0x15, 0xe8}, //litnum6
  {0x00, 0x08, 0x10, 0xf8}, //litnum7
  {0x00, 0xf8, 0x15, 0xf8}, //litnum8
  {0x00, 0xb8, 0x15, 0xf8}, //litnum9
};
uint32_t bignum[10] = {
  0x00f811f8,
  0x00901f80,
  0x00e815b8,
  0x00a815f8,
  0x003804f8,
  0x00b815e8,
  0x00f815e8,
  0x000810f8,
  0x00f815f8,
  0x00b815f8
};
int colorR;
int colorG;
int colorB;
void pixelShow()
{
  pixels.setBrightness(15);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colorR, colorG, colorB);
  }
  pixels.show();
}
void setup() {
  WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }
    timeClient.begin();
  // put your setup code here, to run once:
  pixels.begin();
  pixels.setBrightness(10);
  pixelShow();
}
/*
  void write_time(uint8_t num) {
  //for (int nn = 0;nn< 4;nn++){
  for (int ni = 0; ni < 4; ni++) {
    for (int nj = 0; nj < 8; nj++) {
      uint8_t aa = litnumber[num][ni] ;
      if ( aa & 0x80) {
        pixels.setPixelColor(ni * 8 + nj, 20, 0, 0);
      } else {
        pixels.setPixelColor(ni * 8 + nj, 0, 0, 0);
      }
      aa <<= 1;
    }
  }
  }
*/
int line = 0;
void write_data(uint32_t a, int aa) {
  uint32_t b = a;
  for (int m = 0; m < 32; m++) {
    if (b & 0x80000000) {
      pixels.setPixelColor(aa * 32 + m, 20, 0, 0);
    } else {
      pixels.setPixelColor(aa * 32 + m, 0, 0, 0);
    }
    b <<= 1;
  }
}
uint32_t dot = 0x00500a00;
void loop() {
  timeClient.update();
 // for (int j = 0; j < 10 ; j++) {
    write_data(bignum[timeClient.getHours() / 10], 0);
    write_data(bignum[timeClient.getHours() % 10], 1);
    write_data(dot, 2);
    write_data(bignum[timeClient.getMinutes() / 10], 3);
    write_data(bignum[timeClient.getMinutes() % 10], 4);
    write_data(dot, 5);
    write_data(bignum[timeClient.getSeconds() / 10], 6);
    write_data(bignum[timeClient.getSeconds() % 10], 7);
    pixels.show();
    dot ^= 0x00500a00;
    delay(500);
 // }
}

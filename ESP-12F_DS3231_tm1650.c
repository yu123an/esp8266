#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define PIN            4
#define NUMPIXELS      256
int i = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
const char *ssid     = "WiFi_name";         //WiFi名称
const char *password = "WiFi_pass";   //WiFi密码
const char *ssid2     = "WiFi_name2";         //WiFi名称
const char *password2 = "WiFi_pass2";   //WiFi密码
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
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
    if (i > 40) {                    //60秒后如果还是连接不上，就判定为连接超时      
      Serial.println("");
      Serial.print("连接超时！尝试第二wifi");
      break;
    }
    i = 10;
    }
    WiFi.begin(ssid2, password2);
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    if (i > 40) {                    //60秒后如果还是连接不上，就判定为连接超时      
      Serial.println("");
      Serial.print("连接超时！请检查网络环境");
      break;
    }
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
    if (timeClient.getSeconds() == 55 ){
      for (int j = 0; j < 50 ; j++) {
    write_data(bignum[random(9)], random(7));
    pixels.show();
    delay(100);
      }
    }
 // }
}

/*
   1.色块填充，坐标加面积
   2.划线，首位坐标
   3.字体大小
      4号字，24*32大小
      3号字，16*16大小
      2号字，10*10大小
      1号字，没注意
*/
#include <ArduinoJson.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Arduino_ST7789.h> // Hardware-specific library for ST7789 (with or without CS pin)
#include <SPI.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
//#include <Wire.h>     //DS3231
#define TFT_DC    12
#define TFT_RST   14
#define TFT_MOSI  4   // for hardware SPI data pin (all of available pins)
#define TFT_SCLK  5   // for hardware SPI sclk pin (all of available pins)
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK); //for display without CS pin_Old
//Arduino_ST7789 tft = Arduino_ST7789(/*TFT_DC*/0, /*TFT_RST*/2, /*TFT_MOSI*/13, /*TFT_SCLK*/14);//HSPI接口
//int i = 0;          //超时检测
const char *ssid     = "WIFI";         //WiFi名称
const char *password = "PASS";   //WiFi密码
float p = 3.1415926;
int ds_hour, ds_min, ds_sec;
int minute1, minute2, hour1, hour2;
String serverName = "http://WebStation/WebStation/tft.php";
StaticJsonDocument<200> doc;
double longitude;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
HTTPClient http;
void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! ST7789 TFT Test");
  // Wire.begin();
  tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels
  Serial.println("Initialized");
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
  
  fill_screen(BLACK);
}
void loop() {
  timeClient.update();
  //tft.fillRect(0,0,240,80,BLACK);
  tft.setCursor(60, 0);
  tft.setTextColor(BLUE, BLACK);
  //tft.setColor(BLUE);
  tft.setTextSize(4);
  if (timeClient.getHours() > 9) {
  } else {
    tft.print(0);
  }
  tft.print(timeClient.getHours());
  tft.print(":");
  if (timeClient.getMinutes() > 9) {
  } else {
    tft.print(0);
  }
  tft.print(timeClient.getMinutes());
  delay(5000);
  tft.drawLine(0, 32, 4 * timeClient.getSeconds(), 32, YELLOW);
  tft.drawLine(4 * timeClient.getSeconds(), 32, 240, 32, BLACK);
  
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post_data = "name=aaa";
  http.POST(post_data);
  String load = http.getString();
  int len = load.length() + 1;
  char json[len] ;
 load.toCharArray(json,len);
  //DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
 /* const char* sensor = doc["sensor"];
  long time          = doc["time"];
  double latitude    = doc["data"][0];
  longitude   = doc["data"][1];
  Serial.println(longitude);*/
  int class_number = doc["num"];
 double win_speed = doc["weather"]["win_speed"];
  tft.setCursor(60,34);
  tft.setTextSize(4);
  tft.println(class_number);
   tft.setCursor(60,67);
  tft.setTextSize(4);
  tft.println(win_speed);
  // delay(200);
  /*tft.invertDisplay(true);
    delay(500);
    tft.invertDisplay(false);
    delay(500);*/
  //tft.setTextWrap(false);
  // tft.fillScreen(BLACK);
  /*tft.setCursor(0, 60);
    tft.setTextColor(RED);
    tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(YELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");
    tft.setTextColor(GREEN);
    tft.setTextSize(3);*/
  // draw_w();
  //delay(2000);
}
void fill_screen(uint16_t color) {
  tft.fillRect(0, 0, 240, 80, color);
  delay(5);
  tft.fillRect(0, 80, 240, 80, color);
  delay(5);
  tft.fillRect(0, 160, 240, 80, color);
  delay(5);
}

void draw_w() {
  fill_screen(BLACK);
  for (int a = 1; a <= 6; a++) {
    for (int b = 1; b <= 6; b++) {
      int c = a * b;
      tft.setCursor((a - 1) * 40, (b - 1) * 10);
      tft.setTextSize(1);
      tft.setTextColor(GREEN);
      tft.print(a);
      tft.print("*");
      tft.print(b);
      tft.print("=");
      tft.println(c);
    }
  }
}
void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}



void testdrawrects(uint16_t color) {
  tft.fillScreen(BLACK);
  for (int16_t x = 0; x < tft.width(); x += 6) {
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color);
  }
}

void testfillrects(uint16_t color1, uint16_t color2) {
  tft.fillScreen(BLACK);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color2);
  }
}

void testfillcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = radius; x < tft.width(); x += radius * 2) {
    for (int16_t y = radius; y < tft.height(); y += radius * 2) {
      tft.fillCircle(x, y, radius, color);
    }
  }
}

void testdrawcircles(uint8_t radius, uint16_t color) {
  for (int16_t x = 0; x < tft.width() + radius; x += radius * 2) {
    for (int16_t y = 0; y < tft.height() + radius; y += radius * 2) {
      tft.drawCircle(x, y, radius, color);
    }
  }
}

void testtriangles() {
  tft.fillScreen(BLACK);
  int color = 0xF800;
  int t;
  int w = tft.width() / 2;
  int x = tft.height() - 1;
  int y = 0;
  int z = tft.width();
  for (t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x -= 4;
    y += 4;
    z -= 4;
    color += 100;
  }
}

void testroundrects() {
  tft.fillScreen(BLACK);
  int color = 100;
  int i;
  int t;
  for (t = 0 ; t <= 4; t += 1) {
    int x = 0;
    int y = 0;
    int w = tft.width() - 2;
    int h = tft.height() - 2;
    for (i = 0 ; i <= 16; i += 1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x += 2;
      y += 3;
      w -= 4;
      h -= 6;
      color += 1100;
    }
    color += 100;
  }
}

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(WHITE);
  tft.print(" seconds.");
}

void mediabuttons() {
  // play
  tft.fillScreen(BLACK);
  tft.fillRoundRect(25, 10, 78, 60, 8, WHITE);
  tft.fillTriangle(42, 20, 42, 60, 90, 40, RED);
  delay(500);
  // pause
  tft.fillRoundRect(25, 90, 78, 60, 8, WHITE);
  tft.fillRoundRect(39, 98, 20, 45, 5, GREEN);
  tft.fillRoundRect(69, 98, 20, 45, 5, GREEN);
  delay(500);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, BLUE);
  delay(50);
  // pause color
  tft.fillRoundRect(39, 98, 20, 45, 5, RED);
  tft.fillRoundRect(69, 98, 20, 45, 5, RED);
  // play color
  tft.fillTriangle(42, 20, 42, 60, 90, 40, GREEN);
}

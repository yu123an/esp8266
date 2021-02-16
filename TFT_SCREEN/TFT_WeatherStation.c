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
#include <Adafruit_GFX.h>
#include <Arduino_ST7789.h>
#include <SPI.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>     //DS3231
#define TFT_DC    12
#define TFT_RST   14
#define TFT_MOSI  4
#define TFT_SCLK  5
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK); //for display without CS pin_Old
//Arduino_ST7789 tft = Arduino_ST7789(/*TFT_DC*/0, /*TFT_RST*/2, /*TFT_MOSI*/13, /*TFT_SCLK*/14);//HSPI接口
int i = 0;          //超时检测
const char *ssid     = "WiFi_Name";         //WiFi名称
const char *password = "WiFi_Pass";   //WiFi密码
String serverName = "http://Your.Station/WebStation/tft.php";    //服务器地址
int ds_hour, ds_min, ds_sec;
int minute1, minute2, hour1, hour2;
StaticJsonDocument<200> doc;
const char* wea;
int class_number;
double win_speed;
double temp2;
double hum;
double rain;
double pm;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void setup(void) {
  Serial.begin(9600);
  Serial.print("Hello! ST7789 TFT Test");
  Wire.begin();
  tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels
  Serial.println("Initialized");
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
Get_msg();
  fill_screen(BLACK);
}
void loop() {
  timeClient.update();
  //tft.fillRect(0,0,240,80,BLACK);
  tft.setCursor(60, 1);
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
  tft.drawLine(0, 33, 4 * timeClient.getSeconds(), 33, YELLOW);
  tft.drawLine(4 * timeClient.getSeconds(), 33, 240, 33, BLACK);
  delay(500);
  draw_msg();
}
void fill_screen(uint16_t color) {
  tft.fillRect(0, 0, 240, 80, color);
  delay(5);
  tft.fillRect(0, 80, 240, 80, color);
  delay(5);
  tft.fillRect(0, 160, 240, 80, color);
  delay(5);
}
void Get_msg() {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post_data = "name=aaa";
  http.POST(post_data);
  String load = http.getString();
  int len = load.length() + 1;
  char json[len] ;
  load.toCharArray(json, len);
  deserializeJson(doc, json);
  wea = doc["weather"]["wea"];
  class_number = doc["num"];
  win_speed = doc["weather"]["win_speed"];
  temp2 = doc["weather"]["temp"];
  hum = doc["weather"]["hum"];
  rain = doc["weather"]["rain"];
  pm = doc["weather"]["pm"];
}
void draw_msg() {
  //Class_number;Line2;Char9
  tft.setCursor(12, 34);
  tft.setTextColor(BLUE, BLACK);
  tft.setTextSize(4);
  tft.print("NUM:");
  tft.println(class_number);
  delay(5);
  //temp&hum;Line3;Char9
  tft.setCursor(12, 66);
  tft.setTextColor(BLUE, BLACK);
  tft.setTextSize(4);
  tft.print(hum);
  tft.print("%");
  tft.print(temp2);
  delay(5);
  //Wind_speed;Line4;Char8
  tft.setCursor(24, 98);
  tft.setTextColor(BLUE, BLACK);
  tft.setTextSize(4);
  tft.print("Wind:");
  tft.println(win_speed);
  delay(5);
  //Rain;Line5;Char8
  tft.setCursor(12, 130);
  tft.setTextColor(BLUE, BLACK);
  tft.setTextSize(4);
  tft.print("Rain:");
  tft.println(rain);
  delay(5);
  //PM2.5;Line2;Char8
  tft.setCursor(24, 162);
  tft.setTextColor(BLUE, BLACK);
  tft.setTextSize(4);
  tft.print("PM:");
  tft.println(pm);
}

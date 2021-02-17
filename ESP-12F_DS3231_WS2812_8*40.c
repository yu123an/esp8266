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
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#define sda 4
#define scl 5
#define PIN            14
#define NUMPIXELS      320
const char *ssid     = "WiFi";         //WiFi名称
const char *password = "Pass";   //WiFi密码
String serverName = "http://Your_Station.xyz/WebStation/tft.php";    //服务器地址
StaticJsonDocument<200> doc;
const char* wea;
int class_number;
double win_speed;
double temp2;
int temp22;
double hum;
double rain;
double pm;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Ticker tim;
int ds_hour, ds_min, ds_sec , sec;
int minute1, minute2, hour1, hour2;
int i = 0;
int colorR;
int colorG;
int colorB;
uint8_t litle[][4] = {
  0x00, 0xF8, 0x88, 0xF8,
  0x00, 0x90, 0xF8, 0x80,
  0x00, 0xE8, 0xA8, 0xB8,
  0x00, 0xA8, 0xA8, 0xF8,
  0x00, 0x38, 0x20, 0xF8,
  0x00, 0xB8, 0xA8, 0xE8,
  0x00, 0xF8, 0xA8, 0xE8,
  0x00, 0x08, 0x08, 0xF8,
  0x00, 0xF8, 0xA8, 0xF8,
  0x00, 0xB8, 0xA8, 0xF8,
};
uint8_t Weather[][8] = {
  0X00, 0X00, 0XC0, 0XFF, 0XCA, 0X0A, 0X00, 0X00, //temp  0
  0X00, 0X42, 0X18, 0X3C, 0X3C, 0X18, 0X42, 0X00, //sun   1
  0X00, 0X00, 0XFF, 0X1F, 0X0E, 0X04, 0X00, 0X00, //wind  2
  0X70, 0X8C, 0X82, 0X8C, 0X70, 0X06, 0X09, 0X06, //rain  3
  0X00, 0X2A, 0X6B, 0X1C, 0X77, 0X1C, 0X6B, 0X2A, //snow  4
  0XF3, 0X3F, 0X0F, 0X03, 0XC0, 0XF0, 0XFC, 0XCF, //sunder  5
  0X20, 0X30, 0X28, 0X24, 0X22, 0X22, 0X2C, 0X30, //cloud   6
  0X22, 0X35, 0X2A, 0X24, 0X22, 0X22, 0X2C, 0X30, //cloud&sun   7
  0XA0, 0X70, 0X28, 0XA4, 0X62, 0X22, 0XAC, 0X70, //cloud&rain  8
  0X00, 0X00, 0X7C, 0X08, 0X30, 0X08, 0X7C, 0X00, //class_money 9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//black block// 10
  0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00,// /11
};
uint8_t fonts[][8] = {
  0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00,// 0
  0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00,// 1
  0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00,// 2
  0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00,// 3
  0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00,// 4
  0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00,// 5
  0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00,// 6
  0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00,// 7
  0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00,// 8
  0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00,// 9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,// ' '
  0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x00,// !
  0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00,// "
  0x00, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, 0x00,// #
  0x00, 0x24, 0x2A, 0x07, 0x2A, 0x12, 0x00, 0x00,// $
  0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00,// %
  0x00, 0x37, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00,// &
  0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00,// '
  0x00, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 0x00,// (
  0x00, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, 0x00,// )
  0x00, 0x08, 0x2A, 0x1C, 0x2A, 0x08, 0x00, 0x00,// *
  0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 0x00,// +
  0x00, 0x00, 0x50, 0x30, 0x00, 0x00, 0x00, 0x00,// ,
  0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,// -
  0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00,// .
  0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00,// /
  0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00,// :
  0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00,// ;
  0x00, 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00,// <
  0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00,// =
  0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00, 0x00,// >
  0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00,// ?
  0x00, 0x32, 0x49, 0x79, 0x41, 0x3E, 0x00, 0x00,// @
  0x00, 0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00, 0x00,// A
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00,// B
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00,// C
  0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00, 0x00,// D
  0x00, 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00,// E
  0x00, 0x7F, 0x09, 0x09, 0x01, 0x01, 0x00, 0x00,// F
  0x00, 0x3E, 0x41, 0x41, 0x51, 0x32, 0x00, 0x00,// G
  0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 0x00,// H
  0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 0x00,// I
  0x00, 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 0x00,// J
  0x00, 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00,// K
  0x00, 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00,// L
  0x00, 0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00, 0x00,// M
  0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 0x00,// N
  0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00,// O
  0x00, 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00,// P
  0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 0x00,// Q
  0x00, 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00,// R
  0x00, 0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00,// S
  0x00, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00, 0x00,// T
  0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 0x00,// U
  0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 0x00,// V
  0x00, 0x7F, 0x20, 0x18, 0x20, 0x7F, 0x00, 0x00,// W
  0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00,// X
  0x00, 0x03, 0x04, 0x78, 0x04, 0x03, 0x00, 0x00,// Y
  0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00,// Z
  0x00, 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00,// [
  0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00,// "\"
  0x00, 0x41, 0x41, 0x7F, 0x00, 0x00, 0x00, 0x00,// ]
  0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00,// ^
  0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00,// _
  0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00,// `
  0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00,// a
  0x00, 0x7F, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00,// b
  0x00, 0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 0x00,// c
  0x00, 0x38, 0x44, 0x44, 0x48, 0x7F, 0x00, 0x00,// d
  0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00,// e
  0x00, 0x08, 0x7E, 0x09, 0x01, 0x02, 0x00, 0x00,// f
  0x00, 0x08, 0x14, 0x54, 0x54, 0x3C, 0x00, 0x00,// g
  0x00, 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00,// h
  0x00, 0x00, 0x44, 0x7D, 0x40, 0x00, 0x00, 0x00,// i
  0x00, 0x20, 0x40, 0x44, 0x3D, 0x00, 0x00, 0x00,// j
  0x00, 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00,// k
  0x00, 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 0x00,// l
  0x00, 0x7C, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00,// m
  0x00, 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00,// n
  0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00,// o
  0x00, 0x7C, 0x14, 0x14, 0x14, 0x08, 0x00, 0x00,// p
  0x00, 0x08, 0x14, 0x14, 0x18, 0x7C, 0x00, 0x00,// q
  0x00, 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00,// r
  0x00, 0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 0x00,// s
  0x00, 0x04, 0x3F, 0x44, 0x40, 0x20, 0x00, 0x00,// t
  0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00, 0x00,// u
  0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 0x00,// v
  0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, 0x00,// w
  0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00,// x
  0x00, 0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00, 0x00,// y
  0x00, 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 0x00,// z
  0x00, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00,// {
  0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00,// |
  0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00, 0x00,// }
  0x00, 0x02, 0x01, 0x02, 0x04, 0x02, 0x00, 0x00,// ~
  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,//black block// �
};
void pixelShow()
{
  pixels.setBrightness(50);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colorR, colorG, colorB);
  }
  pixels.show();
}
void setup() {
  Wire.begin();
  Serial.begin(115200);
  write_time();
  pixels.begin();
  pixels.setBrightness(10);
  pixelShow();
  tim.attach(600, write_time);
}
void write_data(uint8_t a, int aa, int bb) {
  uint8_t b = a;
  for (int m = 0; m < 8; m++) {
    if (b & 0x01) {
      pixels.setPixelColor(aa * 32 + bb * 8 + m, 0, 10, 10);
    } else {
      pixels.setPixelColor(aa * 32 + bb * 8 + m, 0, 0, 0);
    }
    b >>= 1;
  }
}
int dot = 0x44;
void loop() {
  read_time();
  dot ^= 0x44;
  for (int i = 0; i < 8; i++) {
    write_data(fonts[ds_hour / 16][i], 0, i);
    write_data(fonts[ds_hour % 16][i], 2, i);
    write_data(fonts[ds_min / 16][i], 4, i);
    write_data(fonts[ds_min % 16][i], 6, i);
  }
  for (int i = 0; i < 4; i++) {
    write_data(litle[ds_sec / 16][i], 8, i);
    write_data(litle[ds_sec % 16][i], 9, i);
  }
  write_data(dot, 2, 7);
  pixels.show();
  if (ds_sec % 16 == 2){
    draw_msg();
    //delay(3000);
  }
  delay(999);
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
void write_time() {
  WiFi.begin(ssid, password);         //联网
  while ( WiFi.status() != WL_CONNECTED ) {
    i++;
    delay ( 500 );
    Serial.print ( "." );
    if (i > 40) {                    //60秒后如果还是连接不上，就判定为连接超时
      Serial.println("");
      Serial.print("连接超时！请检查网络环境");
      break;
    }
  } Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("已联网，准备更新时间！！！");
    timeClient.begin();
    timeClient.update();
    //秒，分，时的获取及写入；
    int vall = timeClient.getHours();                   //hh
    if (vall != 8){
    Wire.beginTransmission(0x68);
    Wire.write(0x00);
    int valll = timeClient.getSeconds();                   //ss
    int dd = (valll / 10 * 16) + (valll % 10);
    Wire.write(dd);
    Wire.endTransmission();
    Wire.beginTransmission(0x68);
    Wire.write(0x01);
    int vallll = timeClient.getMinutes();                   //mm
    int ddd = (vallll / 10 * 16) + (vallll % 10);
    Wire.write(ddd);
    Wire.endTransmission();
    Wire.beginTransmission(0x68);
    Wire.write(0x02);
    int vall = timeClient.getHours();                   //hh
    int d = (vall / 10 * 16) + (vall % 10);
    Wire.write(d);
    Wire.endTransmission();
    Serial.println("时间更新完成！！！");}
    else{
      Serial.println("更新时间出错？？？");
    }
    Get_msg();
  }
  else
  {
    Serial.println("未联网，凑乎用吧。。。。。。");
  }
  WiFi.disconnect(1);                     //时间更新完成后，断开连接，保持低功耗；
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("无线终端和接入点的连接已中断");
  }
  else
  {
    Serial.println("未能成功断开连接！");
  }
}
void read_time() {
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3);
  ds_sec = Wire.read();
  ds_min = Wire.read();
  ds_hour = Wire.read();
  hour1 = (ds_hour / 16);
  hour2 = (ds_hour % 16 );
  minute1 = (ds_min / 16 );
  minute2 = (ds_min % 16);
  sec = ((ds_sec / 16) * 10 + ds_sec % 16);
  Serial.println((ds_sec / 16) * 10 + ds_sec % 16);
  Serial.println(ds_sec);
}
void draw_msg() {
  read_temp();
  int win_all = win_speed * 10;
  int temp2_all = temp2 * 10;
  int hum_all = hum * 10;
  int rain_all = rain * 10;
  int pm_all = pm * 10;
   //draw temp
   for (int i = 0; i < 8; i++) {
    write_data(Weather[0][i],4,i);
    write_data(fonts[temp2_all / 100][i], 0, i);
    write_data(fonts[temp2_all % 100 /10][i], 2, i);
    write_data(fonts[temp22 / 100][i], 6, i);
    write_data(fonts[temp22 % 100 /10][i], 8, i);
   }
   pixels.show();
   delay(2000);
   //draw class_money
   for (int i = 0; i < 8; i++) {
    write_data(Weather[9][i],0,i);
    write_data(Weather[10][i],2,i);
    write_data(fonts[class_number / 100][i], 4, i);
    write_data(fonts[class_number % 100 /10][i], 6, i);
    write_data(fonts[class_number % 10][i], 8, i);
   }
   pixels.show();
   delay(2000);
}
void read_temp()
{
  // _reset();
  Wire.beginTransmission(0x40);
  Wire.write(0xf3);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(0x40, 2);
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint16_t value = msb << 8 | lsb;
  float temp1 = value * (175.72 / 65536.0) - 46.85;
   temp22 = int(temp1 * 10 );
  Serial.print(" The temp is :");
  Serial.print(temp1);
  Serial.print(";-------------------The Stant temp is :");
  Serial.println(temp22);
}
void read_rh() {
  Wire.beginTransmission(0x40);
  Wire.write(0xf5);
  Wire.endTransmission();
  delay(40);
  Wire.requestFrom(0x40, 2);
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  uint16_t value = msb << 8 | lsb;
  float rh1 = value * (125.0 / 65536.0) - 6.0;
  int rh2 = int(rh1 * 10 );
  Serial.print(" The humidity is :");
  Serial.print(rh1);
  Serial.print(";-------------------The Stant humidity is :");
  Serial.println(rh2);
}

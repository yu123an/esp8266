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
#include "msg.h"
#include "font_icon.h"
#define sda 4
#define scl 5
#define PIN            14
#define NUMPIXELS      320
const char *ssid     = "WiFi_Name";         //WiFi名称
const char *password = "WiFi_Pass";   //WiFi密码
String serverName = "http://Your.Station.com/WebStation/tft.php";    //服务器地址
StaticJsonDocument<200> doc;
int weath = 1;
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
void pixelShow()
{
  pixels.setBrightness(5);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colorR, colorG, colorB);
  }
  pixels.show();
}
void setup() {
  //wifi.autoConnect("Clock","23456789");
  Wire.begin();
  Serial.begin(115200);
  write_time();
  pixels.begin();
  pixels.setBrightness(10);
  pixelShow();
  tim.attach(600, write_time);
}
void write_data(int x, int number) {
  uint8_t b;
  for (int m = 0; m < 8; m++) {
    b = fonts[number][m];
    for (int j = 0; j < 8; j++) {
      pixels.setPixelColor(8 * x + 8 * m + j, 0, (b & 0x01) * 160, (b & 0x01) * 160);
      b >>= 1;
    }
  }
}
/*
  void write_icon(int x, int number) {
  uint8_t b[3];
  for (int m = 0; m < 8; m++) {
    for (int i = 0; i < 3; i++) {
      b[i] = icons[number * 3 + i][m];
    }
    for (int j = 0; j < 8; j++) {
      pixels.setPixelColor(8 * x + 8 * m + j, (b[0] & 0x01) * 160, (b[1] & 0x01) * 80, (b[2] & 0x01) * 80);
      for (int i = 0; i < 3; i++) {
        b[i] >>= 1;
      }
    }
  }
  }
*/
void write_Icon(int x, int n) {
  for (int m = 0; m < 64; m++) {
    pixels.setPixelColor(8 * x + m, Icon[n][m * 3], Icon[n][m * 3 + 1], Icon[n][m * 3 + 2]);
  }
}

void loop() {
  read_time();
  write_data(0, ds_hour / 16);
  write_data(8, ds_hour % 16);
  write_data(16, ds_min / 16);
  write_data(24, ds_min % 16);
  write_data(32, ds_sec / 16 + 10);
  write_data(36, ds_sec % 16 + 10);
  //write_bitmap(32, 0);
  pixels.setPixelColor(15 * 8 + 2, 0, (ds_sec % 2) * 160, 0);
  pixels.setPixelColor(15 * 8 + 5, 0, (ds_sec % 2) * 160, 0);
  pixels.show();
  if (((ds_sec / 16) * 10 + ds_sec % 16) == 32) {
    msg_animo();
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
    if (vall != 8) {
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
      Serial.println("时间更新完成！！！");
    }
    else {
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
void msg_animo() {
  read_temp();
  int win_all = win_speed * 10;
  int temp2_all = temp2 * 10;
  int hum_all = hum * 10;
  int rain_all = rain * 10;
  int pm_all = pm * 10;
  delay(200);
  for (int x = 0; x > -97 ; x--) {
    read_time();
    //draw time
    write_data(x, ds_hour / 16);
    write_data(x + 8, ds_hour % 16);
    write_data(x + 16, ds_min / 16);
    write_data(x + 24, ds_min % 16);
    write_data(x + 32, ds_sec / 16 + 10);
    write_data(x + 36, ds_sec % 16 + 10);
    pixels.setPixelColor((x + 15) * 8 + 2, 0, (ds_sec % 2) * 30, 0);
    pixels.setPixelColor((x + 15) * 8 + 5, 0, (ds_sec % 2) * 30, 0);
    //draw msg
    if (weath) {
      write_Icon(x + 40, 3);
      write_data(x + 48, temp2_all / 100);
      write_data(x + 56, temp2_all % 100 / 10);
    } else {
      write_Icon(x + 40, water_icon);
      write_data(x + 48, hum_all / 100);
      write_data(x + 56, hum_all % 100 / 10);
    }

    write_Icon(x + 64, dollor_icon);
    write_data(x + 72, class_number / 100);
    write_data(x + 80, class_number % 100 / 10);
    write_data(x + 88, class_number % 10);
    //draw time
    write_data(x + 96, ds_hour / 16);
    write_data(x + 8 + 96, ds_hour % 16);
    write_data(x + 16 + 96, ds_min / 16);
    write_data(x + 24 + 96, ds_min % 16);
    write_data(x + 32 + 96, ds_sec / 16 + 10);
    write_data(x + 36 + 96, ds_sec % 16 + 10);
    pixels.setPixelColor((x + 15 + 96) * 8 + 2, 0, (ds_sec % 2) * 30, 0);
    pixels.setPixelColor((x + 15 + 96) * 8 + 5, 0, (ds_sec % 2) * 30, 0);
    pixels.show();
    delay(70);
  } weath = !weath;
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

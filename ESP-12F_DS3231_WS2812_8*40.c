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
const char *password = "PaWd";   //WiFi密码
String serverName = "http://Station/WebStation/tft.php";    //服务器地址
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
uint8_t icons[][8] = {
  0x60, 0x9F, 0x81, 0x9F, 0x60, 0x0E, 0x0A, 0x01, //tmp_1   //01
  0x60, 0xFF, 0xF9, 0xFF, 0x60, 0x0E, 0x0A, 0x01, //tmp_2
  0x60, 0x9F, 0x81, 0x9F, 0x60, 0x0E, 0x0A, 0x01, //tmp_3
  0x00, 0x24, 0x4A, 0xFF, 0x52, 0x24, 0x00, 0x00, //money   //02
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //money
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //money
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //block   //03
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //block
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //block
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
      pixels.setPixelColor(8 * x + 8 * m + j, 0, (b & 0x01) * 10, (b & 0x01) * 10);
      b >>= 1;
    }
  }
}
void write_data_lit(int x, int number) {
  uint8_t b;
  for (int m = 0; m < 4; m++) {
    b = litle[number][m];
    for (int j = 0; j < 8; j++) {
      pixels.setPixelColor(8 * x + 8 * m + j, 0, (b & 0x01) * 10, (b & 0x01) * 10);
      b >>= 1;
    }
  }
}
void write_icon(int x, int number) {
  uint8_t b[3];
  for (int m = 0; m < 8; m++) {
    for (int i = 0; i < 3; i++) {
      b[i] = icons[number * 3 + i][m];
    }
    for (int j = 0; j < 8; j++) {
      pixels.setPixelColor(8 * x + 8 * m + j, (b[0] & 0x01) * 10, (b[1] & 0x01) * 10, (b[2] & 0x01) * 10);
      for (int i = 0; i < 3; i++) {
        b[i] >>= 1;
      }
    }
  }
}
void loop() {
  read_time();
  write_data(0, ds_hour / 16);
  write_data(8, ds_hour % 16);
  write_data(16, ds_min / 16);
  write_data(24, ds_min % 16);
  write_data_lit(32, ds_sec / 16);
  write_data_lit(36, ds_sec % 16);
  pixels.setPixelColor(15 * 8 + 2, 0, (ds_sec % 2) * 10, 0);
  pixels.setPixelColor(15 * 8 + 5, 0, (ds_sec % 2) * 10, 0);
  pixels.show();
  if (((ds_sec / 16) * 10 + ds_sec % 16) == 32) {
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
void draw_msg() {
  read_temp();
  int win_all = win_speed * 10;
  int temp2_all = temp2 * 10;
  int hum_all = hum * 10;
  int rain_all = rain * 10;
  int pm_all = pm * 10;
  //draw temp
  write_icon(16, 0);
  write_data(0, temp2_all / 100);
  write_data(8, temp2_all % 100 / 10);
  write_data(24, temp22 / 100);
  write_data(32, temp22 % 100 / 10);
  pixels.show();
  delay(4000);
  //draw class_money
  write_icon(0, 1);
  write_icon(8, 2);
  write_data(16, class_number / 100);
  write_data(24, class_number % 100 / 10);
  write_data(32, class_number % 10);
  pixels.show();
  delay(4000);
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

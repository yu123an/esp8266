#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "msg.h"
#include "font_icon.h"
#define sda 4   //IIC data
#define scl 5   //IIC clk
#define PIN  14     //ws2812 data pin
#define NUMPIXELS 320   //ws2812 number
WiFiManager wifimanager;
String ssid ;         //WiFi名称
String password ;   //WiFi密码
/*
  以下为wifimanager自动配网的相关代码
  申请512字节大小的EEProm空间，用来存储wifi信息以及和风天气的地区码以及api密钥
*/
int wifi_name_len;  //wifi name length
int wifi_pass_len;   //wifi pass length
int wifi_name_add = 1;
int wifi_pass_add = 41;
int weather_local_add = 81;
int weather_key_add = 91;
String wifi_name;
String wifi_pass;
String weather_local;//hefeng weather local number
String weather_key;//hefeng weather api key
char key[9];
char local[32];
StaticJsonDocument<200> doc;
StaticJsonDocument<400> web;
int weather_icon = 3;
int weath = 1;
int wea;
int class_number;
double win_speed;
double temp2;
int temp22;
int hum;
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
  pixels.setBrightness(8);

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colorR, colorG, colorB);
  }
  pixels.show();
}
void setup() {
  Wire.begin();
  EEPROM.begin(512);
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
void write_fonts(int x, int number) {
  uint8_t b;
  for (int m = 0; m < 8; m++) {
    b = dm_fonts[number][m];
    for (int j = 0; j < 8; j++) {
      pixels.setPixelColor(8 * x + 8 * m + j, 0, (b & 0x01) * 160, (b & 0x01) * 160);
      b >>= 1;
    }
  }
}
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
  /*Serial.println(hefeng_wind);
    Serial.print("weather local is : ");
    Serial.println(read_eeprom(weather_local_add, 9));
    Serial.print("weather key is : ");
    Serial.println(read_eeprom(weather_key_add, 32));*/
}
void Get_msg() {
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post_data = "name=1";
  http.POST(post_data);
  String load = http.getString();
  int len = load.length() + 1;
  char json[len] ;
  load.toCharArray(json, len);
  deserializeJson(doc, json);
  class_number = doc["num"];
  Serial.println(load);
  http.end();
}
/*
   以下为和风天气的获取代码
*/
String hefengServer = "https://devapi.qweather.com/v7/weather/now";
void Get_web_msg() {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient Hs;
  String Hs_web = hefengServer + "?gzip=n&location=" +
                  read_eeprom(weather_local_add, 9) + "&key=" +
                  read_eeprom(weather_key_add, 32);
  Hs.begin(*client, Hs_web);
  int sta = Hs.GET();
  String web_get = Hs.getString();
  int len = web_get.length() + 1;
  char json[len] ;
  web_get.toCharArray(json, len);
  deserializeJson(web, json);
  wea = web["now"]["icon"];
  //class_number = doc["num"];
  win_speed = web["now"]["windScale"];
  temp2 = web["now"]["temp"];
  hum = web["now"]["humidity"];
  rain = web["now"]["precip"];
  Serial.println(web_get);
  Serial.println(hum);
  Serial.println(win_speed);
  Hs.end();
}
/*
  以上为和风天气的获取代码
*/
void write_time() {
  ssid = read_eeprom(wifi_name_add, EEPROM.read(141));
  password = read_eeprom(wifi_pass_add, EEPROM.read(142));
  WiFi.begin(ssid, password);         //联网
  while ( WiFi.status() != WL_CONNECTED ) {
    i++;
    delay ( 500 );
    Serial.print ( "." );
    if (i > 40) {                    //60秒后如果还是连接不上，就判定为连接超时
      Serial.println("");
      Serial.print("连接超时！请检查网络环境"); 
      Serial.println("");
      //wifimanager.resetSettings();
      wifimanager.setDebugOutput(0);//关闭Debug调试
      wifimanager.setTimeout(120);//配网超时2分钟
      WiFiManagerParameter Weather_key("weatherkey", "和风天气密钥", key, 32);
      WiFiManagerParameter Weather_local("weatherlocal", "城市代码", local, 9);
      wifimanager.addParameter(&Weather_key);
      wifimanager.addParameter(&Weather_local);
      wifimanager.autoConnect("Hua_Weather");
      ssid = String(WiFi.SSID());
      password = String(WiFi.psk());
      weather_key = String(Weather_key.getValue());
      weather_local = String(Weather_local.getValue());
      wifi_name_len = ssid.length();
      wifi_pass_len = password.length();
      write_eeprom(wifi_name_add, ssid);
      write_eeprom(wifi_pass_add, password);
      if (weather_key.length() == 32) {
        write_eeprom(weather_local_add, weather_local);
        write_eeprom(weather_key_add, weather_key);
        Serial.println(" key is written !!! ");
      } else {
        Serial.println(" key error !!! ");
      }
      EEPROM.write(141, wifi_name_len);
      EEPROM.write(142, wifi_pass_len);
      EEPROM.commit();
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("已联网，准备更新时间！！！");
    timeClient.begin();
    timeClient.update();
    //秒，分，时的获取及写入；
    int Hour_ex = timeClient.getHours();                   //hh
    if (Hour_ex != 8) {
      Wire.beginTransmission(0x68);
      Wire.write(0x00);
      int Sec_ex = timeClient.getSeconds();                   //ss
      int dd = (Sec_ex / 10 * 16) + (Sec_ex % 10);
      Wire.write(dd);
      Wire.endTransmission();
      Wire.beginTransmission(0x68);
      Wire.write(0x01);
      int Minu_ex = timeClient.getMinutes();                   //mm
      int ddd = (Minu_ex / 10 * 16) + (Minu_ex % 10);
      Wire.write(ddd);
      Wire.endTransmission();
      Wire.beginTransmission(0x68);
      Wire.write(0x02);
      int Hour_ex = timeClient.getHours();                   //hh
      int d = (Hour_ex / 10 * 16) + (Hour_ex % 10);
      Wire.write(d);
      Wire.endTransmission();
      Serial.println("时间更新完成！！！");
    }
    else {
      Serial.println("更新时间出错？？？");
    }
    Get_msg();
    Get_web_msg();
    ota_update();
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
}
void msg_animo() {
  //read_temp();
  int win_all = win_speed * 10;
  int temp2_all = temp2 * 10;
  int hum_all = hum * 10;
  int rain_all = rain * 10;
  int pm_all = pm * 10;
  if (wea / 100 == 3) {
    weather_icon = 4;   //rain
  } else if (wea == 100 || wea == 150) {
    weather_icon = 2;   //sunny
  } else if (wea == 101 || wea == 102 || wea == 103 || wea == 153 || wea == 104 || wea == 154) {
    weather_icon = 6;   //cloudy
  } else if (wea == 100 || wea == 150) {
    weather_icon = 2;   //sunny
  } else if (wea / 100 == 4) {
    weather_icon = 3;   //snow
  } else if (wea / 100 == 4) {
    weather_icon = 9;   //haze
  }
  delay(200);
  for (int x = 0; x > -105 ; x--) {
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
      write_Icon(x + 40, weather_icon);
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
    write_Icon(x + 96, 10);
    write_data(x + 104, ds_hour / 16);
    write_data(x + 8 + 104, ds_hour % 16);
    write_data(x + 16 + 104, ds_min / 16);
    write_data(x + 24 + 104, ds_min % 16);
    write_data(x + 32 + 104, ds_sec / 16 + 10);
    write_data(x + 36 + 104, ds_sec % 16 + 10);
    pixels.setPixelColor((x + 15 + 104) * 8 + 2, 0, (ds_sec % 2) * 30, 0);
    pixels.setPixelColor((x + 15 + 104) * 8 + 5, 0, (ds_sec % 2) * 30, 0);
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
/*
   读写EEPROM
*/
void write_eeprom(int addr, String velue) {
  int lenth = velue.length();
  for (int a = 0; a < lenth; a++) {
    EEPROM.write(addr + a, velue[a]);
  }
}
String read_eeprom(int addr, int lenth) {
  String Text;
  for (int a = 0; a < lenth; a++) {
    Text +=  char(EEPROM.read(addr + a));
  }
  return Text ;
  Serial.print(Text);
}
/*
OTA更新代码
*/
void ota_update() {
  HTTPClient Ota_update;
  String post_data = ota_chack + "?address=" + WiFi.macAddress() + "&Version=" + String(version + 1);
  Ota_update.begin(post_data);
  Serial.println("查询新版固件........");
  int gg = Ota_update.GET();
  String Is_updata = Ota_update.getString();
  Serial.print("固件版本：");
  Serial.println(version);
  /*Serial.print("返回状态：");
  Serial.println(Is_updata);
  Serial.print("请求数据：");
  Serial.println(post_data);
  Serial.print("固件地址：");
  Serial.println(ota_url);*/
  if ( Is_updata == "OK") {
    Serial.println("发现新版固件，准备更新........");
    ESPhttpUpdate.update(ota_url);
  } else {
    Serial.println("已是最新固件，继续使用");
  }
  Ota_update.end();
}

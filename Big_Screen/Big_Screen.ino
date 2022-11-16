//加载库
#include "msg.h"        //敏感信息
#include <NTPClient.h>  //网络校时
#include <HTTPClient.h> //http请求
#include "SPI.h"
#include "TFT_eSPI.h" //屏幕驱动
//#include "siyuan_20.h"//中文字库
#include <RtcDS1307.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ArduinoJson.h>      //JSON解析
#include <WiFiClientSecure.h> //https请求
#include "OneButton.h"        //按钮启用
#include <Ticker.h>           //定时任务
// SD卡读写
#include "FS.h"
#include "SD.h"
#include "SPI.h"
//定义颜色
#define c_BL tft.color24to16(0xc0ebd7)
#define c_Line tft.color24to16(0x426666)
#define c_text tft.color24to16(0x003371)
#define c_time tft.color24to16(0x333631)
//定义字体
#define Digi &DS_DIGI32pt7b         // 数码管字体
#define DejaVu &DejaVu_Sans_Mono_20 //等宽字体
String shici;                       //古诗词api返回诗句
String weak = "Double" /*Double or Single*/;
String temp, hump, windDir, wind, _weather, _date;
int _day, _hour, _minute,_second;
// 特殊引脚
#define WeakFlag 13//周选择
#define sdSelectPin 25//sd卡
#define SCL 22
#define SDA 21
// json串解析
#define MSG_BUFFER_SIZE (20000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<20000> Mqtt_Sub; // JSON解析缓存
// 实例化类
TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;
WiFiClientSecure espClient;
NTPClient timeClient(ntpUDP, "ntp2.aliyun.com", 8 * 3600, 60000);
RtcDS1307<TwoWire> Rtc(Wire);
OneButton Buton1(34, 1, 0);
OneButton Buton2(35, 1, 0);
OneButton Buton3(36, 1, 1);
Ticker button_wdg;
Ticker uptime;
void setup()
{
  pinMode(WeakFlag, INPUT);
  Serial.begin(9600);
  sd_en();
  //开机，显示开机画面
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(c_BL);
  tft.setTextColor(c_time);
  tft.setFreeFont(Digi);
  tft.drawString("Chwhsn", 50, 60);
  tft.drawString("Start.....", 50, 120);
  Scan_WiFi();
  WiFi.begin(ssid, password); // 连接网络
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  { // 等待连接
    delay(500);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  // get_net();
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  drawClass();
  time_update();
  uptime.attach(600,time_update);
}

void loop()
{
  // delay(10000);
  // time_update();
  // if (_minute / 10 == 1)
  // {
  //   drawClass();
  //   time_update();
  //   delay(60 * 1000);
  // }
  drawTime();
  delay(1000);
}
// 更新时间
void time_update()
{
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  _second = timeClient.getSeconds();
   Rtc._SetDateTime(_second, _minute, _hour);
}
void drawTime(){
  Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
 tft.setTextColor(c_time);
 tft.setFreeFont(Digi);
 tft.fillRect(10, 78, 216, 64, c_BL);
 tft.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) + ":" + String(now.Second() / 10) + String(now.Second() % 10), 12, 80);
}
// http请求
void get_net(String web)
{
  HTTPClient http;
  if (http.begin(web))
  {
    Serial.println("HTTPclient setUp done!");
  }
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      deserializeJson(Mqtt_Sub, payload);
    }
  }
  else
  {

    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
// http字符串解析
void JsonEecoed(String json)
{
  deserializeJson(Mqtt_Sub, json);
}
// 扫描wifi
void Scan_WiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if (WiFi.SSID(0) == "weixingban 3lou  bangongshi")
  {
    ssid = "weixingban 3lou  bangongshi";
    password = passsyucai;
  }
  else if (WiFi.SSID(0) == "Nexus")
  {
    ssid = "Nexus";
    password = passhome;
  }
  tft.fillScreen(c_BL);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_Line);
  tft.drawString("Begin scan WiFi:", 25, 30);
  tft.setTextColor(c_text);
  for (int i = 0; i < 5; i++)
  {
    tft.drawString(WiFi.SSID(i), 25, 50 + 20 * i);
  }
  tft.drawString("We will connect the \n" + WiFi.SSID(0), 25, 170);
}
// 绘制课表
void drawClass()
{
  get_net(web_hf);
  // 处理http返回信息
  temp = Mqtt_Sub["now"]["temp"].as<String>() + "°C ";
  hump = Mqtt_Sub["now"]["humidity"].as<String>() + "% ";
  windDir = Mqtt_Sub["now"]["windDir"].as<String>();
  wind = Mqtt_Sub["now"]["windScale"].as<String>();
  _weather = Mqtt_Sub["now"]["text"].as<String>();
  _date = Mqtt_Sub["now"]["obsTime"].as<String>().substring(0, 10) + " ";
  String _Day = Mqtt_Sub["now"]["obsTime"].as<String>().substring(8, 10);
  tft.fillScreen(c_BL);
  tft.fillRect(0, 0, 480, 30, c_text);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  tft.setTextColor(c_text);
  tft.drawString(_Day + " A1 A2 A3 A4 B1 B2 B3 B4 C1 C2 C3", 12, 35);
  for (int i = 0; i < 11; i++)
  {
    tft.drawLine(26 + 12 + 6 + 39 * i, 35, 26 + 12 + 6 + 39 * i, 70, c_Line);
  }
  tft.drawLine(12, 55, 468, 55, c_Line);
  deserializeJson(Mqtt_Sub, _class);
  String CL;
  if (digitalRead(WeakFlag))
  {
    CL = Mqtt_Sub["Single"][_day]["no"].as<String>();
  }
  else
  {
    CL = Mqtt_Sub["Double"][_day]["no"].as<String>();
  }
  tft.drawString(CL, 12, 57);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  tft.drawString(_date + temp + hump + wind, 12, 5);
  time_update();
}
void sd_en() {
  if (!SD.begin(sdSelectPin)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
}

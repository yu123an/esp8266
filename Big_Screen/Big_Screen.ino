#include <HTTPClient.h>
#include "SPI.h"
#include "TFT_eSPI.h"
//#include "siyuan_20.h"
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ArduinoJson.h>  //JSON解析
#include <WiFiClientSecure.h>
//定义颜色
#define c_BL tft.color24to16(0xc0ebd7)
#define c_Line tft.color24to16(0x426666)
#define c_text tft.color24to16(0x003371)
#define c_time tft.color24to16(0x333631)
//定义字体
#define Digi &DS_DIGI32pt7b
#define DejaVu &DejaVu_Sans_Mono_20
//敏感信息
const char *ssid = "weixingban 3lou  bangongshi";
const char *password = "hbycwxb@123456";
String shici ;
String temp ,hump,windDir,wind,_weather,_date;
#define MSG_BUFFER_SIZE (20000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<20000> Mqtt_Sub;  // JSON解析缓存
//实例化类
TFT_eSPI tft = TFT_eSPI();
WiFiUDP ntpUDP;
WiFiClientSecure espClient;
void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);  // 连接网络
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {  // 等待连接
    delay(500);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  // get_net();
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  //联网完成，先买年开始tft处理
  tft.begin();
  tft.setRotation(1);
  draw_Class();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
void draw_Class() {
  int _data = 12;
  tft.fillScreen(c_BL);
  tft.fillRect(0, 0, 480, 30, c_text);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  String _d = "2022/12/03" +  String(hump) ;
  //tft.drawString(_d, 12, 5);
  tft.setTextColor(c_text);
  tft.drawString(String(_data) + " A1 A2 A3 A4 B1 B2 B3 B4 C1 C2 C3", 12, 35);
  for (int i = 0; i < 11; i++) {
    tft.drawLine(26 + 12 + 6 + 39 * i, 35, 26 + 12 + 6 + 39 * i, 70, c_Line);
  }
  tft.drawLine(12, 55, 468, 55, c_Line);
  tft.drawString("D2       21    33    31    33 31 21", 12, 57);
  tft.setTextColor(c_time);
  tft.setFreeFont(Digi);
  tft.drawString("22:33", 12, 80);
  tft.drawRect(10, 78, 146, 64, c_Line);
  //String at = "落霞与孤鹜齐飞\n秋水共长天一色12345678904356345";
  //tft.loadFont(siyuan_20);
 tft.setFreeFont(DejaVu);
  tft.setCursor(121, 150);
  Get_weather();
  tft.setTextColor(c_BL);
    tft.drawString(_date + temp + hump + wind, 12, 5);
  //tft.println(WiFi.localIP());
  //get_shiju();
  //tft.loadFont(siyuan_20);
  //tft.println(shici);
}
void get_shiju(){
    String url = "https://v1.jinrishici.com/rensheng.txt";
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");

    if (http.begin(url))
    {
       
        Serial.println("HTTPclient setUp done!");
       
    }

    Serial.print("[HTTP] GET...\n");

    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {

        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
           // #ifdef DEBUG
            Serial.println(payload);
            shici = payload;
            //#endif
          //  _parseNowJson(payload);
        }
    }
    else
    {

        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());

       // return false;
    }

    http.end();
 //   return payload;
}
void Get_weather(){
   String url = "https://devapi.qweather.com/v7/weather/now?location=00000000&key=000000000&gzip=n";
    HTTPClient http;
    if (http.begin(url))
    {
        Serial.println("HTTPclient setUp done!");
    }
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            deserializeJson(Mqtt_Sub,payload);
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
temp = Mqtt_Sub["now"]["temp"].as<String>() + "°C ";
hump = Mqtt_Sub["now"]["humidity"].as<String>() + "% ";
windDir = Mqtt_Sub["now"]["windDir"].as<String>();
wind = Mqtt_Sub["now"]["windScale"].as<String>();
_weather = Mqtt_Sub["now"]["text"].as<String>();
_date = Mqtt_Sub["now"]["obsTime"].as<String>().substring(0,10) + " ";
Serial.print("Now the temp is :");
Serial.println(temp);
}

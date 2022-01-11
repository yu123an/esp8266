#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <WiFiClientSecureBearSSL.h>
//以下为MQtt通讯部分代码
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <PubSubClient.h>
#define LED 13
#define debug 1
/*
#define AIO_SERVER "888888"
#define AIO_SERVERPORT 8888                           // use 8883 for SSL
#define AIO_USERNAME "8888888888" //百度云MQTT用户名
#define AIO_KEY "888888888888888"     //百度云MQTT用户密码
*/
//*
  #define AIO_SERVER "8888888888888888888m" //百度云MQtt链接地址
  #define AIO_SERVERPORT 1883
  #define AIO_USERNAME "888888888888888888888" //百度云MQTT用户名
  #define AIO_KEY "8888888888888888888"     //百度云MQTT用户密码
//*/
//以下为和风天气相关参数
String hefengServer = "https://devapi.qweather.com/v7/astronomy/sun";
String hefengServer2 = "https://devapi.qweather.com/v7/weather/now";
String Hs_Key = "8888888888888";
String Hs_Local = "888888888888";
int temp_0, temp_1;
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};
WiFiClient client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000); //NTP时钟获取
//MQTT客户端链接
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
//MQTT消息订阅与发布
// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe get_msg = Adafruit_MQTT_Subscribe(&mqtt, "$iot/Xuangaun/user/light", MQTT_QOS_1);
Adafruit_MQTT_Publish Pub_Temp = Adafruit_MQTT_Publish(&mqtt, "$iot/Xuangaun/user/KeTing");
int time_flag = 0;
uint32_t x = 0;
const char *ssid = "Nexus";
const char *password = "13033883439";
StaticJsonDocument<400> doc;
StaticJsonDocument<400> web;
StaticJsonDocument<400> Mqtt_Sub;
String Hs_Time_All;
String Hs_Time_Use;
String Sun_Rise_Time_All;
String Sun_Rise_Time_Use;
String Sun_Set_Time_All;
String Sun_Set_Time_Use;
int Rise_Time = 0;
int Set_Time = 0;
int Led_Flag = 0;
int _hour = 0;
int _minute = 0;
int _second = 0;
//灯光模式控制位
int Led_State = 0;
/*
   以下为和风天气的测试代码
*/
void Get_sun_time()
{
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient Hs;
  String HS_time = hefengServer2 + "?gzip=n&key=" + Hs_Key + "&location=" + Hs_Local;
  String Hs_web = hefengServer + "?gzip=n&key=" + Hs_Key + "&location=" + Hs_Local + "&date=";
  //获取每日时间
  Hs.begin(*client, HS_time);
  int sta = Hs.GET();
  String web_get = Hs.getString();
  int len = web_get.length() + 1;
  char json[len];
  web_get.toCharArray(json, len);
  deserializeJson(web, json);
  /*wea = web["now"]["icon"];
    //class_number = doc["num"];
    win_speed = web["now"]["windScale"];
    temp2 = web["now"]["temp"];
    hum = web["now"]["humidity"];
    rain = web["now"]["precip"];*/
  Hs_Time_All = String(web["updateTime"]);
  Hs_Time_Use = Hs_Time_All.substring(0, 4) + Hs_Time_All.substring(5, 7) + Hs_Time_All.substring(8, 10);
  if (debug)
  {
    Serial.print("Web get is :");
    Serial.println(HS_time);
    Serial.println("The msg is :");
    Serial.println(web_get);
    Serial.println(Hs_Time_All);
    Serial.println(Hs_Time_Use);
  }
  Hs.end();
  //获取日出日落时间

  Hs.begin(*client, Hs_web + Hs_Time_Use);
  int sta2 = Hs.GET();
  String web_get2 = Hs.getString();
  int len2 = web_get2.length() + 1;
  char json2[len2];
  web_get2.toCharArray(json2, len2);
  deserializeJson(doc, json2);
  Sun_Rise_Time_All = String(doc["sunrise"]);
  Sun_Set_Time_All = String(doc["sunset"]);
  Sun_Set_Time_Use = Sun_Set_Time_All.substring(11, 13) + Sun_Set_Time_All.substring(14, 16);
  Sun_Rise_Time_Use = Sun_Rise_Time_All.substring(11, 13) + Sun_Rise_Time_All.substring(14, 16);
  if (debug)
  {
    Serial.println("The Sun msg is :");
    Serial.println(Sun_Rise_Time_All);
    Serial.println(Sun_Set_Time_All);
    Serial.println(Sun_Rise_Time_Use);
    Serial.println(Sun_Set_Time_Use);
    Rise_Time = Sun_Rise_Time_All.substring(11, 13).toInt() * 60 + Sun_Rise_Time_All.substring(14, 16).toInt();
    Set_Time = Sun_Set_Time_All.substring(11, 13).toInt() * 60 + Sun_Set_Time_All.substring(14, 16).toInt();
    Serial.print("The Sun rise time is :");
    Serial.println(Rise_Time);
    Serial.print("The Sun det time is :");
    Serial.println(Set_Time);
  }
  Hs.end();
}
/*
  以上为和风天气的测试代码
*/
//MQTT心跳包检测
void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000); // wait 10 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}
void update_time()
{
  timeClient.begin();
  timeClient.update();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  _second = timeClient.getSeconds();
  if (debug)
  {
    Serial.print("现在的网络时间为：");
    Serial.println(timeClient.getFormattedTime());
  }
}
void get_temp()
{
  Wire.begin(5, 4);
  Wire.beginTransmission(0x48);
  Wire.write(0x00);
  Wire.requestFrom(0x48, 1);
  temp_0 = Wire.read();
  Wire.beginTransmission(0x49);
  Wire.write(0x00);
  Wire.requestFrom(0x49, 1);
  temp_1 = Wire.read();
  if (debug)
  {
    Serial.print("当前第一个温度为：");
    Serial.println(temp_0);
    Serial.print("当前第二个温度为：");
    Serial.println(temp_1);
  }
}
//获取信息
void Get_Mqtt()
{
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    if (debug)
    {
      //Serial.println("The Sub Topic is :");
      //Serial.println(*subcription);
      Serial.println("I get th msg :");
      Serial.println((char *)get_msg.lastread);
    }
    //处理接收到的信息,进行Json化
    deserializeJson(Mqtt_Sub, (char *)get_msg.lastread);
    if (debug)
    {
      Serial.println("The temp_0 is :");
      Serial.println(String(Mqtt_Sub["Temp_0"]));
    }
    Led_State = Mqtt_Sub["led_Sta"];
    if (Led_State == 1)
    {
      open_light();
    }
    else if (Led_State == 0)
    {
      close_light();
    }
    else
    {
      lighting();
    }
  }
  //保持连接
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
}
void open_light()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 1);
  Led_Flag = 1;
  Msg_Pub();
}
void close_light()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  Led_Flag = 0;
  Msg_Pub();
}
void lighting()
{
  pinMode(LED, OUTPUT);
  for (int t = 0; t < 5; t++)
  {
    digitalWrite(LED, 1);
    delay(1500);
    digitalWrite(LED, 0);
    delay(1500);
    digitalWrite(LED, Led_Flag);
  }
}
void Msg_Pub()
{
  //{\"Temp_IN\":temp_0,\"Temp_EX\":temp_1,\"light\":Led_Flag,\"led_Sta\":Led_State}
  //,\"led_Sta\":
  get_temp();
  String Pub_1 = "{\"Temp_IN\":";
  String Pub_2 = ",\"Temp_EX\":";
  String Pub_3 = ",\"light\":";
  String Pub_4 = ",\"led_Sta\":";
  String Pub_5 = "}";
  String _Pub = Pub_1 + temp_0 + Pub_2 + temp_1 + Pub_3 + Led_Flag + Pub_4 + Led_State + Pub_5;
  char Pub[300];
  _Pub.toCharArray(Pub, 300);
  Pub_Temp.publish(Pub);
  if (debug)
  {
    Serial.println("推送消息内容为：");
    Serial.println(Pub);
    Serial.println(timeClient.getFormattedTime());
  }
}
void Get_Net()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  delay(500);
  update_time();
}
//中断函数，按键控制灯光开关
ICACHE_RAM_ATTR void Led_Key()
{
  if (Led_Flag)
  {
    close_light();
  }
  else
  {
    open_light();
  }
  for (int t = 0; t <= 300; t++)
  {
    delayMicroseconds(999);
  }
}
void setup()
{
  pinMode(LED, OUTPUT);
  // put your setup code here, to run once:
  Serial.begin(74880);
  Get_Net();
  if (Rise_Time == 0 && Set_Time == 0)
  {
    if (debug)
    {
      Serial.println("已经联网，准备获取日出日落时间：");
      Serial.println("+---------------------------+");
    }
    delay(200);
    //获取日出与日落时间的函数
    Get_sun_time();
    Serial.println("+---------------------------+");
  }
}
void loop()
{
  Serial.println("Begin!!!");
  update_time();
  //判断当前时间，与日出日落时间做对比
  if ((_hour - 1) * 60 > Rise_Time && (_hour * 60) < Set_Time)
  { //日出
    if (debug)
    {
      Serial.println("时间已更新，天已亮了，可以关灯");
    }
    close_light();
    // ESP.deepSleep((61 - _minute)*10e5);
  }
  else
  { //黑夜，开启灯光控制
    pinMode(12, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(12), Led_Key, FALLING);
    if (debug)
    {
      Serial.println("时间已更新，天色已晚，可以关灯");
    }
  }
  mqtt.subscribe(&get_msg);

  Get_Mqtt();
  delay(2000);
  time_flag++;
  if (time_flag == 14)
  {
    time_flag = 0;
    Msg_Pub();
    //每隔30s推送一次消息
  }
  Serial.print("Led_Sta is :");
  Serial.println(Led_State);
  Serial.print("Led_Flag is :");
  Serial.println(Led_Flag);
}

#include <StackThunk.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <WiFiClientSecureBearSSL.h>
#include <EEPROM.h>
#include <ThreeWire.h> //DS1302
#include <RtcDS1302.h>
//以下为MQtt通讯部分代码
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <PubSubClient.h>
/*
 #define AIO_SERVER "*********"
#define AIO_SERVERPORT ********* // use 8883 for SSL
#define AIO_USERNAME "*********" //百度云MQTT用户名
#define AIO_KEY "*********"     //百度云MQTT用户密码
*/
#define LED 13
#define debug 1
WiFiClient client;
WiFiUDP ntpUDP;
//MQTT客户端链接
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
//MQTT消息订阅与发布
// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe get_msg = Adafruit_MQTT_Subscribe(&mqtt, "EspHome", MQTT_QOS_1);
Adafruit_MQTT_Publish Pub_Temp = Adafruit_MQTT_Publish(&mqtt, "_EspHome");
ThreeWire myWire(15, 0, 14); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
uint32_t x = 0;
const char *ssid     = "wei xing ban ban gong shi";
const char *password = "weixing1234+-*/";
StaticJsonDocument<400> doc;
StaticJsonDocument<400> web;
StaticJsonDocument<800> Mqtt_Sub;
int Sun_rise_hour, Sun_rise_minute, Sun_set_hour, Sun_set_minute;
int _hour, _minute;
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
int get_temp()
{
  Wire.begin(5, 4);
  Wire.beginTransmission(0x4F);
  Wire.write(0x00);
  Wire.requestFrom(0x4F, 1);
  uint16_t temp = Wire.read();
  if (debug)
  {
    Serial.print("当前温度为：");
    Serial.println(temp);
  }
  return temp;
}
void Get_Mqtt(uint32_t x)
{
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(800)))
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
    String type = Mqtt_Sub["Type"];
    if (type == "time")
    {
      Serial.println("准备更新时间");
      int _hour = Mqtt_Sub["hour"];
      int _minute = Mqtt_Sub["minute"];
      int _second = Mqtt_Sub["second"];
      Rtc.My_SetDateTime( _second, _minute, _hour, 2, 2, 2, 22);
    }
    else if (type == "SunTime")
    {
      Sun_rise_hour = Mqtt_Sub["R_H"];
      Sun_rise_minute = Mqtt_Sub["R_M"];
      Sun_set_hour = Mqtt_Sub["S_H"];
      Sun_set_minute = Mqtt_Sub["S_M"];
      EEPROM.write(0, Sun_rise_hour);
      EEPROM.write(1, Sun_rise_minute);
      EEPROM.write(2, Sun_set_hour);
      EEPROM.write(3, Sun_set_minute);
      EEPROM.commit();
    } else {
      Serial.println("Nothing");
    }
  }
  //保持连接
  if (!mqtt.ping())
  {
    mqtt.disconnect();
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
}
void setup()
{
  Serial.begin(9600);
  EEPROM.begin(64);
  Get_Net();
  get_msg.setCallback(Get_Mqtt);
  mqtt.subscribe(&get_msg);
  MQTT_connect();
  Rtc.Begin();
  Rtc.SetIsRunning(true);
  //解除写保护
  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }
}
void loop()
{
  RtcDateTime now = Rtc.GetDateTime();
  Serial.println("I am ok !!!");
  MQTT_connect();
  //mqtt.subscribe(&get_msg);
  Get_Mqtt(3);
  delay(998);
  Serial.print("Now is :");
  Serial.print(now.Hour());
  Serial.print(" :");
  Serial.print(now.Minute());
  Serial.print(" :");
  Serial.print(now.Second());
  Serial.println(" ");
  get_temp();
}

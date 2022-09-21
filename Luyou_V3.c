//加载库
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
#include <PubSubClient.h>
//敏感信息
const char *mqtt_server = "*********";
const char *ssid = "wei xing ban ban gong shi";
const char *password = "weixing1234+-*/";
//LED使能引脚
#define LED 13
//LOG打印标志位
#define debug 1
//实例化类
WiFiClient espClient;
PubSubClient client(espClient);
ThreeWire myWire(15, 0, 14); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
Ticker update_msg;
StaticJsonDocument<800> Mqtt_Sub;
//变量声明
int Sun_rise_hour, Sun_rise_minute, Sun_set_hour, Sun_set_minute;
int _hour, _minute;
//读取温度
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
//联网
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
  //申请EEPROM读写空间
  EEPROM.begin(64);
  //联网
  Get_Net();
  //操作DS1302
  Rtc.Begin();
  Rtc.SetIsRunning(true);
  //解除写保护
  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }
  //定时任务，每隔5s发布信息
  update_msg.attach(5, Pub_msg);
  //MQTT，指定服务器，回调函数，重连操作
  client.setServer(mqtt_server, 1383);
  client.setCallback(callback);
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
void loop()
{
   //MQTT心跳检测
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  delay(998);
  get_temp();
}
// MQTT相关函数
//订阅主题：EspIn
//发布主题：EspOut
void callback(char *topic, byte *payload, unsigned int length)
{
  payload[length] = 0;
  deserializeJson(Mqtt_Sub, String((char *)payload)); //对接收到的MQTT_Message进行JSON解析
  // JSON文件格式：https://github.com/yu123an/esp8266/blob/master/EPS32/json.log
  //"Type": "weather",//消息类型，包含：weather，message，gif，
  String type = Mqtt_Sub["Type"];
  if (type == "time")
  {
    Serial.println("准备更新时间");
    int _hour = Mqtt_Sub["hour"];
    int _minute = Mqtt_Sub["minute"];
    int _second = Mqtt_Sub["second"];
    Rtc.My_SetDateTime(_second, _minute, _hour, 2, 2, 2, 22);
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
  }
  else if (type == "Light")
  {
    Serial.println("Ready to open led");
    if (Mqtt_Sub["light"] == 1)
    {
      digitalWrite(LED, 1);
      Serial.println("open led");
    }
    else
    {
      digitalWrite(LED, 0);
      Serial.println("close led");
    }
  }
  else if (type == "Light")
  {
    digitalWrite(LED, 0);
    // pwm频率
    analogWriteFreq(10000); // 10KHZ
    analogWrite(LED, Mqtt_Sub["PWM"]);
  }
  else
  {
    Serial.println("Nothing");
  }
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266-LouYu";
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("EspIn");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 //定时发布信息
void Pub_msg()
{
  String A = "{\"Temp\":";
  String B = ",\"light\":";
  String ALL = A + get_temp() + B + digitalRead(LED) + "}";
  char _ALL[800];
  ALL.toCharArray(_ALL, 800);
  client.publish("EspOut", _ALL);
}

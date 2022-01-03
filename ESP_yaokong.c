//Key map
/*
        +-------+-------+-------+
        |  4E   |   4D  |   5D  |
        +-------+-------+-------+
        |  56   |   55  |   45  |
        +-------+-------+-------+
        |  46   |  66   |  5E   |
        +-------+-------+-------+
        
*/
//字模
//16段
/*
    +----------------------------+--------------------------+----------------+
    |  '
    |   +-----A-----+
    |   |\    |    /|
    |   | \   |   / |
    |   F  H  I  J  B
    |   |   \ | /   |
    |   |    \|/    |
    |   +--G--+--K--+
    |   |    /|\    |
    |   |   / | \   |
    |   E  N  M  L  C
    |   | /   |   \ |
    |   |/    |    \|
    |   +-----D-----+   .
*/
#include <Arduino.h>
extern "C"
{
#include <user_interface.h>
}
#include <ESP.h>
#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
const char *ssid = "Nexus";
const char *password = "88888888";
const char *mac_ke = "88888888";
const char *mac_wo = "88888888c";
#define AIO_SERVER "chwhsen.xyz"
#define AIO_SERVERPORT 1383 // use 8883 for SSL
WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, "AIO_USERNAME", "AIO_KEY");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "device888888882/state");
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, "de88888888et");
Adafruit_MQTT_Publish public_wo = Adafruit_MQTT_Publish(&mqtt, "dev88888888set");
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000); //NTP时钟获取
#define cs_p 12
#define wr_p 14
#define data_p 0
#define RTC_SDA 0
#define RTC_SCL 14
#define RTC_CE 12
//HT1621操作
//变量定义
//#define BIAS 0x52    //0b1000 0101 0010  1/3duty 4com
#define BIAS 0x50    //0b1000 0101 0010
#define SYSDIS 0X00  //0b1000 0000 0000  关振系统荡器和LCD偏压发生器
#define SYSEN 0X02   //0b1000 0000 0010 打开系统振荡器
#define LCDOFF 0X04  //0b1000 0000 0100  关LCD偏压
#define LCDON 0X06   //0b1000 0000 0110  打开LCD偏压
#define XTAL 0x28    //0b1000 0010 1000 外部接时钟
#define RC256 0X30   //0b1000 0011 0000  内部时钟
#define TONEON 0X12  //0b1000 0001 0010  打开声音输出
#define TONEOFF 0X10 //0b1000 0001 0000 关闭声音输出
#define WDTDIS1 0X0A //0b1000 0000 1010  禁止看门狗
#define BUZ 15
#define KEY_1 14
#define KEY_2 13
#define KEY_3 12
#define debug 1
#define REG_BURST 0xBE
#define REG_WP 0x8E
uint8_t second, minute, hour, day, mouth, dow, year;
int weak_flag = 0;
uint8_t HT_cache[16];
uint16_t number[] = {
    0xe007,
    0x0006,
    0xc243,
    0x8247,
    0x2246,
    0xa245,
    0xe245,
    0x0007,
    0xe247,
    0xa247,
};
uint16_t letter[] = {
    0x0466, //A  0
    0x8857, //B  1
    0xe001, //C  2
    0x8817, //D  3
    0xe241, //E  4
    0x6241, //F  5
    0xe845, //G  6
    0x6246, //H  7
    0x8811, //I  8
    0x4421, //J  9
    0x62a0, //K  10
    0xe000, //L  11
    0x6126, //M  12
    0x6186, //N  13
    0xe007, //O  14
    0x6243, //P  15
    0xe087, //Q  16
    0x62a1, //R  17
    0xa245, //S  18
    0x0811, //T  19
    0xe006, //U  20
    0x6420, //V  21
    0x6486, //W  22
    0x05a0, //X  23
    0x0920, //Y  24
    0x8421, //Z  25
};
static struct rst_info *rinfo = ESP.getResetInfoPtr();
int RST_reason = rinfo->reason;
int sleeptime = 0;
uint32_t x = 0;
//以下为DS1302操作函数
void Write_data(uint8_t data)
{
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(RTC_SCL, LOW);
    delayMicroseconds(2);
    digitalWrite(RTC_SDA, (data & 0x01) ? HIGH : LOW);
    digitalWrite(RTC_SCL, HIGH);
    delayMicroseconds(2);
    data >>= 1;
  }
}
uint8_t Read_data()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(RTC_SCL, LOW);
    delayMicroseconds(2);
    if (digitalRead(RTC_SDA) == HIGH)
      byte |= 0x01 << b;
    digitalWrite(RTC_SCL, HIGH);
    delayMicroseconds(2);
  }
  return byte;
}
void _setDirection(int direction)
{
  pinMode(RTC_SDA, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(RTC_CE, HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000001 | address;
  Write_data(command);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(RTC_CE, HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000000 | address;
  Write_data(command);
}
void _end()
{
  digitalWrite(RTC_CE, LOW);
  pinMode(RTC_SDA, OUTPUT);
}
uint8_t _dec2bcd(uint8_t dec)
{
  return ((dec / 10 * 16) + (dec % 10));
}
uint8_t _bcd2dec(uint8_t bcd)
{
  return ((bcd / 16 * 10) + (bcd % 16));
}
void get_time()
{
  _prepareRead(REG_BURST);
  second = _bcd2dec(Read_data() & 0b01111111);
  minute = _bcd2dec(Read_data() & 0b01111111);
  hour = _bcd2dec(Read_data() & 0b00111111);
  day = _bcd2dec(Read_data() & 0b00111111);
  mouth = _bcd2dec(Read_data() & 0b00011111);
  dow = _bcd2dec(Read_data() & 0b00000111);
  year = _bcd2dec(Read_data() & 0b01111111);
  _end();
  if (debug)
  {
    Serial.print("Now time is ");
    Serial.print(year);
    Serial.print("-");
    Serial.print(mouth);
    Serial.print("-");
    Serial.print(day);
    Serial.print("-");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
  }
}
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year)
{
  _prepareWrite(REG_WP);
  Write_data(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  Write_data(_dec2bcd(sec % 60));
  Write_data(_dec2bcd(minute % 60));
  Write_data(_dec2bcd(hour % 24));
  Write_data(_dec2bcd(day % 32));
  Write_data(_dec2bcd(mouth % 13));
  Write_data(_dec2bcd(dow % 8));
  Write_data(_dec2bcd(year % 100));
  Write_data(0b10000000);
  _end();
}
void DS1302_INT()
{
  pinMode(RTC_CE, OUTPUT);
  pinMode(RTC_SCL, OUTPUT);
  pinMode(RTC_SDA, INPUT);
  digitalWrite(RTC_CE, LOW);
  digitalWrite(RTC_SCL, LOW);
}
//写数据
void HT1621_Write(uint8_t data, uint8_t Long)
{
  for (int i = 0; i < Long; i++)
  {
    digitalWrite(wr_p, LOW);
    delayMicroseconds(4);
    if (data & 0x80)
    {
      digitalWrite(data_p, HIGH);
    }
    else
    {
      digitalWrite(data_p, LOW);
    }
    digitalWrite(wr_p, HIGH);
    delayMicroseconds(4);
    data <<= 1;
  }
}

//指定地址写数据
void HT1621_WriteClkData(uint8_t addr, uint8_t data)
{
  addr <<= 2;
  digitalWrite(cs_p, LOW);
  HT1621_Write(0xa0, 3);
  HT1621_Write(addr, 6);
  HT1621_Write(data, 4);
  digitalWrite(cs_p, HIGH);
}
//连续地址写数据
void HT1621_WriteClkData_(uint8_t addr, uint16_t data)
{
  addr <<= 2;
  digitalWrite(cs_p, LOW);
  HT1621_Write(0xa0, 3);
  HT1621_Write(addr * 4, 6);
  HT1621_Write(data >> 8, 4); //待补充
  HT1621_Write(data >> 4, 4);
  HT1621_Write(data, 4);
  HT1621_Write(data << 4, 4);
  digitalWrite(cs_p, HIGH);
  digitalWrite(2, HIGH);
}
// 写指令
void HT1621_WriteCmd(uint8_t cmd)
{
  digitalWrite(cs_p, LOW);
  HT1621_Write(0x80, 4); //写入指令ID
  HT1621_Write(cmd, 8);
  digitalWrite(cs_p, HIGH);
}
//初始化配置
void HT1621_begin()
{
  HT1621_WriteCmd(BIAS);
  HT1621_WriteCmd(RC256);
  HT1621_WriteCmd(SYSDIS);
  HT1621_WriteCmd(WDTDIS1);
  HT1621_WriteCmd(SYSEN);
  HT1621_WriteCmd(LCDON);
}
//HT1621初始化
void HT1621_INT()
{
  pinMode(cs_p, OUTPUT);
  pinMode(wr_p, OUTPUT);
  pinMode(data_p, OUTPUT);
  HT1621_begin();
}
//电池电量计算

void DisBat()
{
  int bat = analogRead(A0);
  int vote = bat * 100 * 6 / 1024 ;
  //Serial.print("The bat now is :");
  // Serial.println(vote);
  HT1621_WriteClkData_(0, letter[1]);
  HT1621_WriteClkData_(1, letter[0]);
  HT1621_WriteClkData_(2, letter[19]);
  HT1621_WriteClkData_(3, 0x0240);
  HT1621_WriteClkData_(4, number[vote / 100] + 0x0008);
  HT1621_WriteClkData_(5, number[vote % 100 / 10]);
  HT1621_WriteClkData_(6, number[vote % 10]);
  HT1621_WriteClkData_(7, letter[21]);
}
void DisWiFi_()
{
  HT1621_WriteClkData_(0, letter[22]);
  HT1621_WriteClkData_(1, letter[8]);
  HT1621_WriteClkData_(2, letter[5]);
  HT1621_WriteClkData_(3, letter[8]);
  HT1621_WriteClkData_(4, 0x0240);
  HT1621_WriteClkData_(5, 0x0240);
  HT1621_WriteClkData_(6, 0x0240);
  HT1621_WriteClkData_(7, 0x0240);
}
void DisWiFi()
{
  HT1621_WriteClkData_(0, letter[22]);
  HT1621_WriteClkData_(1, letter[8]);
  HT1621_WriteClkData_(2, letter[5]);
  HT1621_WriteClkData_(3, letter[8]);
  HT1621_WriteClkData_(4, 0x0240);
  HT1621_WriteClkData_(5, letter[14]);
  HT1621_WriteClkData_(6, letter[10]);
  HT1621_WriteClkData_(7, 0x0);
}
void DisSleep()
{
  int Temp = get_temp();
  HT1621_WriteClkData_(0, letter[18]);
  HT1621_WriteClkData_(1, letter[11]);
  HT1621_WriteClkData_(2, letter[4]);
  HT1621_WriteClkData_(3, letter[4]);
  HT1621_WriteClkData_(4, letter[15]);
  HT1621_WriteClkData_(5, number[Temp / 10]);
  HT1621_WriteClkData_(6, number[Temp % 10]);
  HT1621_WriteClkData_(7, 0x2241);
}
void DisDelay()
{
  DS1302_INT();
  int temp = get_temp();
  get_time();
  HT1621_WriteClkData_(0, number[hour / 10]);
  HT1621_WriteClkData_(1, number[hour % 10]);
  HT1621_WriteClkData_(2, 0x0240);
  HT1621_WriteClkData_(3, number[minute / 10]);
  HT1621_WriteClkData_(4, number[minute % 10]);
  HT1621_WriteClkData_(5, 0x0240);
  HT1621_WriteClkData_(6, number[temp / 10]);
  HT1621_WriteClkData_(7, number[temp % 10]);
  ESP.deepSleep((61 - second) * 1000000);
}
//MQTT操作
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
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    for (int t = 0; t < 5; t++)
    {
      delayMicroseconds(5000);
    } // wait 5 seconds
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
//获取温度
int get_temp()
{
  Wire.begin(5, 4);
  Wire.beginTransmission(0x48);
  Wire.write(0x00);
  Wire.requestFrom(0x48, 1);
  uint16_t temp = Wire.read();
  // uint8_t temp_2 = Wire.read();
  //Serial.print("The temp is : ");
  // Serial.print(temp, HEX);
  // Serial.print("  and  ");
  // Serial.println(temp_2, HEX);
  return temp;
}
//显示温度
void DisTemp()
{
  int Temp = get_temp();
  HT1621_WriteClkData_(0, letter[19]);
  HT1621_WriteClkData_(1, letter[4]);
  HT1621_WriteClkData_(2, letter[12]);
  HT1621_WriteClkData_(3, letter[15]);
  HT1621_WriteClkData_(4, 0x0240);
  HT1621_WriteClkData_(5, number[Temp / 10]);
  HT1621_WriteClkData_(6, number[Temp % 10]);
  HT1621_WriteClkData_(7, 0x2241);
}
//中断时按钮判断函数opke
ICACHE_RAM_ATTR void talk()
{
  MQTT_connect();
  Wire.beginTransmission(0x24);
  Wire.write(0x49);
  Wire.requestFrom(0x24, 1);
  int b = Wire.read();
  //Serial.println(b, HEX);
  HT1621_WriteClkData_(0, 0x0240);
  HT1621_WriteClkData_(1, 0x0240);
  HT1621_WriteClkData_(2, number[b >> 4]);
  HT1621_WriteClkData_(3, letter[b % 16 % 10]);
  HT1621_WriteClkData_(4, 0x0240);
  HT1621_WriteClkData_(5, 0x0240);
  HT1621_WriteClkData_(6, letter[14]);
  HT1621_WriteClkData_(7, letter[10]);
  delayMicroseconds(500e3);
  DisBat();
  switch (b)
  {
  case 0x5D:
    //打开客厅插座开关
    photocell.publish("{\88888888"on\":1}}");
    break;
  case 0x45:
    //关闭客厅插座开关
    photocell.publish("{\"88888888{\"on\":0}}");
    break;
  case 0x4D:
    //打开电脑开关
    public_wo.publish("{\"88888888\":1}}");
    break;
  case 0x55:
    //关闭电脑开关
    public_wo.publish("{\"88888888n\":0}}");
    break;
  case 0x4E:
    weak_flag = 1;
    break;
  default:
    public_wo.publish("Nothing");
    break;
  case 0x5E:
    //DisSleep();
    DisDelay();
  }
  // MQTT_connect();
  //  photocell.publish("I am here !!!");
}

void setup()
{
  //pinMode(RTC_SDA, OUTPUT);
  //digitalWrite(RTC_SDA,1);
  Serial.begin(74880);
  //Serial.println("+---------------------------+");
  //Serial.print("The weak reason is :");
 // Serial.println(RST_reason);
  //Serial.println("+---------------------------+");
  HT1621_INT();
  //digitalWrite(data_p, 1);
  if (RST_reason == 0)
  {
    //EN复位
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      DisWiFi_();
    }
    DisWiFi();
    delay(500);
    timeClient.begin();
    timeClient.update();
    //更新时间
    DS1302_INT();
    set_time(timeClient.getSeconds(), timeClient.getMinutes(), timeClient.getHours(), 2, 1, 2, 21);
    get_time();
    MQTT_connect();
    mqtt.subscribe(&onoffbutton);
    Wire.begin(5, 4);
    Wire.beginTransmission(0x24);
    Wire.write(0x49);
    Wire.endTransmission();
    pinMode(13, INPUT);
    attachInterrupt(digitalPinToInterrupt(13), talk, FALLING);
  }
  else
  {
    DisDelay();
  }
}

void loop()
{
  DS1302_INT();
  DisBat();
  Wire.begin(5, 4);
  Wire.beginTransmission(0x24);
  Wire.write(0x49);
  Wire.endTransmission();
  pinMode(13, INPUT);
  attachInterrupt(digitalPinToInterrupt(13), talk, FALLING);
  delay(10000);
  delay(10000);
  delay(10000);
  DisDelay();
}

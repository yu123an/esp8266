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
#include <Arduino.h>
#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
const char *ssid = "N888s";
const char *password = "88888888";
const char *mac_ke = "8888888888";
const char *mac_wo = "88888888";
#define AIO_SERVER "192.168.2.127"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, "AIO_USERNAME", "AIO_KEY");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "device/ztc1/d888888882/state");
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, "device/ztc1/d0b88888888/state");

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000); //NTP时钟获取
#define cs_p 12
#define wr_p 15
#define data_p 2
//HT1621操作
//变量定义
#define BIAS 0x52    //0b1000 0101 0010  1/3duty 4com
#define SYSDIS 0X00  //0b1000 0000 0000  关振系统荡器和LCD偏压发生器
#define SYSEN 0X02   //0b1000 0000 0010 打开系统振荡器
#define LCDOFF 0X04  //0b1000 0000 0100  关LCD偏压
#define LCDON 0X06   //0b1000 0000 0110  打开LCD偏压
#define XTAL 0x28    //0b1000 0010 1000 外部接时钟
#define RC256 0X30   //0b1000 0011 0000  内部时钟
#define TONEON 0X12  //0b1000 0001 0010  打开声音输出
#define TONEOFF 0X10 //0b1000 0001 0000 关闭声音输出
#define WDTDIS1 0X0A //0b1000 0000 1010  禁止看门狗
//#define BUFFERSIZE 12
uint8_t HT_cache[16];
uint8_t number[] = {
    0xe0,
    0x00,
    0xc2,
    0x82, //3
    0x22, //4
    0xa2,
    0xe2,
    0x00,
    0xe2,
    0xa2,
    0x07, //0
    0x06,
    0x43,
    0x47, //3
    0x46,
    0x45,
    0x45,
    0x07,
    0x47,
    0x47,
};
uint16_t letter[] = {
    0x0466, 0x8857, 0xe001, 0x8817, 0xe241, 0x6241, 0xe845, 0x6246, 0x8811, 0x4421, 0x62a0, 0xe000, 0x6126, 0x6186, 0xe007, 0x6243, //P
    0xe087, 0x62a1, 0xa245, 0x0811, 0xe006, 0x6420, 0x6486, 0x05a0, 0x0920, 0x8421};
int hour;
uint32_t x = 0;
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
void HT1621_WriteClkData_(uint8_t addr, uint8_t data)
{
    addr <<= 2;
    digitalWrite(cs_p, LOW);
    HT1621_Write(0xa0, 3);
    HT1621_Write(addr, 6);
    for (int m = 0; m < 8; m++)
    {
        HT1621_Write(((data >> m) & 0x01) << 3, 4); //待补充
    }
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

void DisBat(){
    int bat = analogRead(A0);
    int vote = bat * 247 / 1024 / 47 * 100;
   
Serial.print("The bat now is :");
Serial.println(vote);
    
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
        delay(5000); // wait 5 seconds
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
//TM1650操作
//中断时按钮判断函数opke
ICACHE_RAM_ATTR void talk()
{
    Wire.beginTransmission(0x24);
    Wire.write(0x49);
    Wire.requestFrom(0x24, 1);
    int b = Wire.read();
    Serial.println(b, HEX);
    switch (b)
    {
    case 0x5D:
        photocell.publish("{\"mac\":\"d888888882\",\"plug_3\":{\"on\":1}}");
        break;
    case 0x45:
        photocell.publish("{\"mac\":\"d888888882\",\"plug_3\":{\"on\":0}}");
        break;
    default:
        photocell.publish("I am here !!!");
        break;
    }
    // MQTT_connect();
    //  photocell.publish("I am here !!!");
}
void setup()
{

    Serial.begin(9600);
    Serial.println("!!!!!!!!!!");
    HT1621_INT();
    digitalWrite(data_p, 1);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    timeClient.begin();
    timeClient.update();
    hour = timeClient.getHours();
    Serial.println(timeClient.getFormattedTime());
    mqtt.subscribe(&onoffbutton);
    Wire.begin(5, 4);
    Wire.beginTransmission(0x24);
    Wire.write(0x49);
    Wire.endTransmission();
    pinMode(13, INPUT);
    attachInterrupt(digitalPinToInterrupt(13), talk, FALLING);
}

void loop()
{
DisBat();
    MQTT_connect();
   /* for (int m = 0; m < 26; m++)
    {

        for (int a = 0; a < 8; a++)
        {
            HT1621_WriteClkData(a * 4, letter[m] >> 8);
            HT1621_WriteClkData(a * 4 + 1, letter[m] >> 4);
            HT1621_WriteClkData(a * 4 + 2, letter[m]);
            HT1621_WriteClkData(a * 4 + 3, letter[m] << 4);
            delay(200);
        }
    }
    delay(2000);
    for (int a = 0; a < 32; a++)
    {
        HT1621_WriteClkData(a, 0x00);
        delay(100);
    }
*/
    /*
    Serial.print("Sending photocell val ");
    Serial.print(x);
    Serial.print("...");
    if (!photocell.publish(x++))
    {
        Serial.println("Failed");
    }
    else
    {
        Serial.println("OK!");
    }*/

    Wire.begin(5, 4);
    Wire.beginTransmission(0x24);
    Wire.write(0x49);
    Wire.endTransmission();
    pinMode(13, INPUT);
    attachInterrupt(digitalPinToInterrupt(13), talk, FALLING);
    Serial.println("I am ok !!!");
    delay(2000);
}

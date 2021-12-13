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
const char *ssid = "Nexus";
const char *password = "188888439";
const char *mac_ke = "d888888888832";
const char *mac_wo = "88888888888c";
#define AIO_SERVER "192.168.2.127"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, "AIO_USERNAME", "AIO_KEY");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "888888888888888e");
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, "88888888888888888");

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
int hour;
int sleeptime = 0;
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
    int vote = bat * 100 * 247 / 1024 / 47;
    Serial.print("The bat now is :");
    Serial.println(vote);
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
void DisSleep(){
    HT1621_WriteClkData_(0, letter[18]);
    HT1621_WriteClkData_(1, letter[11]);
    HT1621_WriteClkData_(2, letter[4]);
    HT1621_WriteClkData_(3, letter[4]);
    HT1621_WriteClkData_(4, letter[15]);
    HT1621_WriteClkData_(5, 0x8000);
    HT1621_WriteClkData_(6, 0x8000);
    HT1621_WriteClkData_(7, 0x8000);
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
        photocell.publish("{\"mac\":\"88888888888888\",\"plug_3\":{\"on\":1}}");
        break;
    case 0x45:
        photocell.publish("{\"mac\":\"88888888888888888888\",\"plug_3\":{\"on\":0}}");
        break;
    default:
        photocell.publish("I am here !!!");
        break;
        case 0x5E:
        DisSleep();
        ESP.deepSleep(3600e6);
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
        DisWiFi_();
    }
    DisWiFi();
    delay(500);
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
    sleeptime+=1;
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
    delay(20000);
    if(sleeptime > 3){
        DisSleep();
        ESP.deepSleep(3600e6);
    }
}

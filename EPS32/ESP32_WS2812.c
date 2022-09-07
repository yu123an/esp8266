/*
  本产品为ESP32驱动的ws2812像素时钟项目
  硬件功能支持：
  基于DS1307的时钟基准
  基于SHT30的温湿度传感器
  基于C125627的光敏传感器
  基于BL6281的单声道音频播放
  SD内存卡读取
  引脚引出 IO34，IO35，IO36，IO39作为中断按钮
  引脚引出IO16，IO17，IO18，IO19，IO26，IO27，IO33留作备用
  预想的软件功能：
  WiFimanager配网
  时间，天气基本内容显示
  语音播报
  通过按钮调节显示内容
  字符串，图像的获取与显示
  与N1联动控制
  MQTT通讯
  项目使用了 706205 字节，占用了 (53%) 程序存储空间。最大为 1310720 字节。
  全局变量使用了59520字节，(18%)的动态内存，余留268160字节局部变量。最大为327680字节。
*/
//引用库
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // WS2812驱动
#endif
#include <Wire.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <RtcDS1307.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>  //JSON解析
#include <PubSubClient.h> //MQTT
#include <Ticker.h>       //定时器
#include "ClosedCube_SHT31D.h" //SHT30
//引脚分配
#define SCL 22
#define SDA 21
#define LED 23
#define LIG 32
#define VOICE 25
#define EN_V 17
// LOG打印变量
#define DEBUG 1
//功能变量
//String ssid ;
//String password ;
/*
  const char *mqtt_server = "*****";
  const char *ssid = "*****";
  const char *password = "*****";
*/
const char *ssid     = "wei xing ban ban gong shi";
const char *password = "weixing1234+-*/";
const char *mqtt_server = "********";
#define LED_NUM 320
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (20000)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int color_r = 255, color_g = 36, color_b = 234;
int Temp_in, Temp_out; //室内室外温度
int Humidity_in, Humidity_out;
StaticJsonDocument<20000> Mqtt_Sub; // JSON解析缓存
int light;
int Sun_rise_hour, Sun_rise_minute, Sun_set_hour, Sun_set_minute;
// uint8_t sht32_read 0x44;
//数组变量
uint8_t OutSide[192];
uint8_t InSide[192];
uint8_t number[][8] = {
  0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 0x00, // 0
  0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, // 1
  0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, // 2
  0x00, 0x21, 0x41, 0x45, 0x4B, 0x31, 0x00, 0x00, // 3
  0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00, // 4
  0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, // 5
  0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00, // 6
  0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, // 7
  0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, // 8
  0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, // 9
  0x00, 0xF8, 0x88, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little0
  0x00, 0x90, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, // Little1
  0x00, 0xE8, 0xA8, 0xB8, 0x00, 0x00, 0x00, 0x00, // Little2
  0x00, 0xA8, 0xA8, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little3
  0x00, 0x38, 0x20, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little4
  0x00, 0xB8, 0xA8, 0xE8, 0x00, 0x00, 0x00, 0x00, // Little5
  0x00, 0xF8, 0xA8, 0xE8, 0x00, 0x00, 0x00, 0x00, // Little6
  0x00, 0x08, 0x08, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little7
  0x00, 0xF8, 0xA8, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little8
  0x00, 0xB8, 0xA8, 0xF8, 0x00, 0x00, 0x00, 0x00, // Little9
  0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00, // 25 %
};
uint8_t ascii[][8] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 20
  0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, // 21 !
  0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, // 22 "
  0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00, 0x00, // 23 #
  0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00, 0x00, // 24 $
  0x00, 0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 0x00, // 25 %
  0x00, 0x36, 0x49, 0x55, 0x22, 0x50, 0x00, 0x00, // 26 &
  0x00, 0x00, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, // 27 '
  0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x00, // 28 (
  0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, 0x00, 0x00, // 29 )
  0x00, 0x14, 0x08, 0x3e, 0x08, 0x14, 0x00, 0x00, // 2a *
  0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00, // 2b +
  0x00, 0x00, 0x50, 0x30, 0x00, 0x00, 0x00, 0x00, // 2c
  0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, // 2d -
  0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, // 2e .
  0x00, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, // 2f /
  0x00, 0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00, 0x00, // 30 0
  0x00, 0x00, 0x42, 0x7f, 0x40, 0x00, 0x00, 0x00, // 31 1
  0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, // 32 2
  0x00, 0x21, 0x41, 0x45, 0x4b, 0x31, 0x00, 0x00, // 33 3
  0x00, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x00, 0x00, // 34 4
  0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, // 35 5
  0x00, 0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00, 0x00, // 36 6
  0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, // 37 7
  0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, // 38 8
  0x00, 0x06, 0x49, 0x49, 0x29, 0x1e, 0x00, 0x00, // 39 9
  0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, // 3a :
  0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00, 0x00, // 3b ;
  0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00, // 3c <
  0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00, // 3d =
  0x00, 0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 0x00, // 3e >
  0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00, 0x00, // 3f ?
  0x00, 0x32, 0x49, 0x79, 0x41, 0x3e, 0x00, 0x00, // 40 @
  0x00, 0x7e, 0x11, 0x11, 0x11, 0x7e, 0x00, 0x00, // 41 A
  0x00, 0x7f, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, // 42 B
  0x00, 0x3e, 0x41, 0x41, 0x41, 0x22, 0x00, 0x00, // 43 C
  0x00, 0x7f, 0x41, 0x41, 0x22, 0x1c, 0x00, 0x00, // 44 D
  0x00, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x00, 0x00, // 45 E
  0x00, 0x7f, 0x09, 0x09, 0x09, 0x01, 0x00, 0x00, // 46 F
  0x00, 0x3e, 0x41, 0x49, 0x49, 0x7a, 0x00, 0x00, // 47 G
  0x00, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00, 0x00, // 48 H
  0x00, 0x00, 0x41, 0x7f, 0x41, 0x00, 0x00, 0x00, // 49 I
  0x00, 0x20, 0x40, 0x41, 0x3f, 0x01, 0x00, 0x00, // 4a J
  0x00, 0x7f, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, // 4b K
  0x00, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, // 4c L
  0x00, 0x7f, 0x02, 0x0c, 0x02, 0x7f, 0x00, 0x00, // 4d M
  0x00, 0x7f, 0x04, 0x08, 0x10, 0x7f, 0x00, 0x00, // 4e N
  0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00, 0x00, // 4f O
  0x00, 0x7f, 0x09, 0x09, 0x09, 0x06, 0x00, 0x00, // 50 P
  0x00, 0x3e, 0x41, 0x51, 0x21, 0x5e, 0x00, 0x00, // 51 Q
  0x00, 0x7f, 0x09, 0x19, 0x29, 0x46, 0x00, 0x00, // 52 R
  0x00, 0x46, 0x49, 0x49, 0x49, 0x31, 0x00, 0x00, // 53 S
  0x00, 0x01, 0x01, 0x7f, 0x01, 0x01, 0x00, 0x00, // 54 T
  0x00, 0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00, 0x00, // 55 U
  0x00, 0x1f, 0x20, 0x40, 0x20, 0x1f, 0x00, 0x00, // 56 V
  0x00, 0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00, 0x00, // 57 W
  0x00, 0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 0x00, // 58 X
  0x00, 0x07, 0x08, 0x70, 0x08, 0x07, 0x00, 0x00, // 59 Y
  0x00, 0x61, 0x51, 0x49, 0x45, 0x43, 0x00, 0x00, // 5a Z
  0x00, 0x00, 0x7f, 0x41, 0x41, 0x00, 0x00, 0x00, // 5b [
  0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 0x00, // 5c ¥
  0x00, 0x00, 0x41, 0x41, 0x7f, 0x00, 0x00, 0x00, // 5d ]
  0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 0x00, // 5e ^
  0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 0x00, // 5f _
  0x00, 0x00, 0x01, 0x02, 0x04, 0x00, 0x00, 0x00, // 60 `
  0x00, 0x20, 0x54, 0x54, 0x54, 0x78, 0x00, 0x00, // 61 a
  0x00, 0x7f, 0x48, 0x44, 0x44, 0x38, 0x00, 0x00, // 62 b
  0x00, 0x38, 0x44, 0x44, 0x44, 0x20, 0x00, 0x00, // 63 c
  0x00, 0x38, 0x44, 0x44, 0x48, 0x7f, 0x00, 0x00, // 64 d
  0x00, 0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 0x00, // 65 e
  0x00, 0x08, 0x7e, 0x09, 0x01, 0x02, 0x00, 0x00, // 66 f
  0x00, 0x0c, 0x52, 0x52, 0x52, 0x3e, 0x00, 0x00, // 67 g
  0x00, 0x7f, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00, // 68 h
  0x00, 0x00, 0x44, 0x7d, 0x40, 0x00, 0x00, 0x00, // 69 i
  0x00, 0x20, 0x40, 0x44, 0x3d, 0x00, 0x00, 0x00, // 6a j
  0x00, 0x7f, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, // 6b k
  0x00, 0x00, 0x41, 0x7f, 0x40, 0x00, 0x00, 0x00, // 6c l
  0x00, 0x7c, 0x04, 0x18, 0x04, 0x78, 0x00, 0x00, // 6d m
  0x00, 0x7c, 0x08, 0x04, 0x04, 0x78, 0x00, 0x00, // 6e n
  0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 0x00, // 6f o
  0x00, 0x7c, 0x14, 0x14, 0x14, 0x08, 0x00, 0x00, // 70 p
  0x00, 0x08, 0x14, 0x14, 0x18, 0x7c, 0x00, 0x00, // 71 q
  0x00, 0x7c, 0x08, 0x04, 0x04, 0x08, 0x00, 0x00, // 72 r
  0x00, 0x48, 0x54, 0x54, 0x54, 0x20, 0x00, 0x00, // 73 s
  0x00, 0x04, 0x3f, 0x44, 0x40, 0x20, 0x00, 0x00, // 74 t
  0x00, 0x3c, 0x40, 0x40, 0x20, 0x7c, 0x00, 0x00, // 75 u
  0x00, 0x1c, 0x20, 0x40, 0x20, 0x1c, 0x00, 0x00, // 76 v
  0x00, 0x3c, 0x40, 0x30, 0x40, 0x3c, 0x00, 0x00, // 77 w
  0x00, 0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 0x00, // 78 x
  0x00, 0x0c, 0x50, 0x50, 0x50, 0x3c, 0x00, 0x00, // 79 y
  0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x00, 0x00, // 7a z
  0x00, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00, // 7b
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, // 7c |
  0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00, 0x00, // 7d
  0x00, 0x10, 0x08, 0x08, 0x10, 0x08, 0x00, 0x00, // 7e ←
  0x00, 0x00, 0x06, 0x09, 0x09, 0x06, 0x00, 0x00, // 7f →
};
uint8_t bmp[][192] = {
  /*humidity*/ 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 242, 220, 254, 189, 73, 254, 189, 73, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 242, 220, 255, 242, 220, 254, 189, 73, 254, 189, 73, 254, 189, 73, 205, 131, 0, 32, 32, 33, 254, 189, 72, 255, 242, 220, 254, 189, 73, 254, 189, 73, 254, 189, 73, 254, 189, 73, 205, 131, 0, 32, 32, 33, 32, 32, 33, 254, 189, 73, 254, 189, 73, 254, 189, 73, 254, 189, 73, 205, 131, 0, 205, 131, 0, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 205, 131, 0, 205, 131, 0, 205, 131, 0, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33,                                          // humidity
  /*temp*/ 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 14, 14, 241, 14, 14, 241, 255, 255, 255, 255, 255, 255, 32, 32, 33, 14, 14, 241, 14, 14, 241, 14, 14, 241, 14, 14, 241, 14, 14, 241, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 14, 14, 241, 14, 14, 241, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33,            // temp
  /*time*/ 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 116, 116, 116, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 116, 116, 116, 255, 255, 255, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 255, 255, 255, 32, 32, 33, 116, 116, 116, 32, 32, 33, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33,                        // time
  /*msg*/ 32, 32, 33, 32, 32, 33, 40, 207, 56, 19, 170, 34, 40, 207, 56, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 79, 220, 93, 19, 170, 34, 19, 170, 34, 12, 193, 30, 40, 207, 56, 32, 32, 33, 19, 170, 34, 32, 32, 33, 19, 170, 34, 19, 170, 34, 32, 32, 33, 12, 193, 30, 19, 170, 34, 19, 170, 34, 118, 222, 129, 32, 32, 33, 19, 170, 34, 19, 170, 34, 19, 170, 34, 19, 170, 34, 19, 170, 34, 79, 220, 93, 32, 32, 33, 32, 32, 33, 19, 170, 34, 19, 170, 34, 32, 32, 33, 12, 193, 30, 19, 170, 34, 32, 32, 33, 32, 32, 33, 32, 32, 33, 19, 170, 34, 19, 170, 34, 12, 193, 30, 12, 193, 30, 19, 170, 34, 32, 32, 33, 32, 32, 33, 32, 32, 33, 79, 220, 93, 12, 193, 30, 32, 32, 33, 19, 170, 34, 40, 207, 56, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 40, 207, 56, 19, 170, 34, 40, 207, 56, 32, 32, 33, 32, 32, 33, 32, 32, 33,                                                       // msg
  /*rabbit*/ 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 255, 255, 255, 77, 0, 255, 77, 0, 255, 255, 255, 255, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 77, 0, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 77, 0, 255, 255, 255, 255, 255, 255, 255, 77, 0, 255, 77, 0, 255, 255, 255, 255, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, 32, 32, 33, // rabbit
};
//构造对象
Adafruit_NeoPixel WS(LED_NUM, LED, NEO_GRB + NEO_KHZ800); // WS2812
RtcDS1307<TwoWire> Rtc(Wire);                             // DS1307
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "1.openwrt.pool.ntp.org", 3600 * 8, 60000);
WiFiClient espClient;
PubSubClient client(espClient);
Ticker time_update;
Ticker msg_update;
ClosedCube_SHT31D sht3xd;
//主函数
void setup()
{
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  WS.begin(); // WS2812驱动使能
  Serial.begin(9600);
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
  client.setServer(mqtt_server, 1383);
  client.setCallback(callback);
  timeClient.begin(); //获取网络时间
  timeClient.update();
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  sht3xd.begin(0x44);
  if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR)
    Serial.println("[ERROR] Cannot start periodic mode");
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  client.publish("outTopic", "{\"Type\":\"update\"}");
  time_update.attach(600, New_time);
  msg_update.attach(3,Pub_msg);
  EEPROM.begin(512);
  light = EEPROM.read(127);
  Sun_rise_hour = EEPROM.read(40);
  Sun_rise_minute = EEPROM.read(41);
  Sun_set_hour = EEPROM.read(42);
  Sun_set_minute = EEPROM.read(43);
  Temp_out = EEPROM.read(44);
  Humidity_out = EEPROM.read(45);
}

void loop()
{
  RtcDateTime now = Rtc.GetDateTime();\
  get_sht30("Periodic Mode", sht3xd.periodicFetchData());
  WS.setBrightness(light);
  draw_X(0, now.Hour() / 10, color_r, color_g, color_b);
  draw_X(8, now.Hour() % 10, color_r, color_g, color_b);
  draw_X(24, now.Minute() / 10, color_r, color_g, color_b);
  draw_X(32, now.Minute() % 10, color_r, color_g, color_b);
  if ((now.Hour() * 60 + now.Hour() > Sun_rise_hour * 60 + Sun_rise_minute) && (now.Hour() * 60 + now.Hour() < Sun_set_hour * 60 + Sun_set_minute))
  {
    // Serial.println("now is day");
    for (int i = 0; i < 64; i++)
    {
      WS.setPixelColor(128 + i, EEPROM.read(128 + i * 3), EEPROM.read(129 + i * 3), EEPROM.read(130 + i * 3));
    }
  }
  else
  {
    // Serial.println("now is night");
    for (int i = 0; i < 64; i++)
    {
      WS.setPixelColor(128 + i, EEPROM.read(320 + i * 3), EEPROM.read(321 + i * 3), EEPROM.read(322 + i * 3));
    }
  }
  // draw_X(16, 20, 2, 0, 0);
  WS.show();
  if (now.Second() == 13)
  {
     if (now.Minute() % 2 ) {
    draw_X(0, Temp_out / 10, color_r, color_g, color_b);
    draw_X(8, Temp_out % 10, color_r, color_g, color_b);
    draw_X(24, Humidity_out / 10, color_r, color_g, color_b);
    draw_X(32, Humidity_out % 10, color_r, color_g, color_b);
    for (int i = 0; i < 64; i++)
    {
      WS.setPixelColor(128 + i, OutSide[i * 3], OutSide[i * 3 + 1], OutSide[i * 3 + 2]);
    }
    WS.show();
    delay(5000);
  }else{
    draw_X(0, Temp_in / 10, color_r, color_g, color_b);
    draw_X(8, Temp_in % 10, color_r, color_g, color_b);
    draw_X(24, Humidity_in / 10, color_r, color_g, color_b);
    draw_X(32, Humidity_in % 10, color_r, color_g, color_b);
    for (int i = 0; i < 64; i++)
    {
      WS.setPixelColor(128 + i, bmp[1][i * 3], bmp[1][i * 3 + 1], bmp[1][i * 3 + 2]);
    }
    WS.show();
    delay(5000);
  }
  }
  delay(998);
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}
//函数区
// WS2812绘图
void dis_test()
{
  WS.setBrightness(75);
  WS.clear();
  WS.show();
  delay(2000);
  for (int i = 0; i <= 320; i++)
  {
    if (i % 2)
    {
      WS.setPixelColor(i, 4, 4, 0);
    }
    else
    {
      WS.setPixelColor(i, 0, 0, 0);
    }
  }
  WS.show();
}
void draw_X(int x, int num, int r, int g, int b)
{
  for (int X = 0; X < 8; X++)
  {
    for (int Y = 0; Y < 8; Y++)
    {
      WS.setPixelColor(8 * (x + 1) + 8 * (X - 1) + Y, ((number[num][X] >> Y) & 0x01) * r, ((number[num][X] >> Y) & 0x01) * g, ((number[num][X] >> Y) & 0x01) * b);
    }
  }
}
void draw_Xasc(int x, int num, int r, int g, int b)
{
  for (int X = 0; X < 8; X++)
  {
    for (int Y = 0; Y < 8; Y++)
    {
      WS.setPixelColor(8 * (x + 1) + 8 * (X - 1) + Y, ((ascii[num][X] >> Y) & 0x01) * r, ((number[num][X] >> Y) & 0x01) * g, ((number[num][X] >> Y) & 0x01) * b);
    }
  }
}
void draw_ascii(String str)
{

  int n = str.length();
  for (int NN = 0; NN < 4; NN++)
  {
    WS.clear();
    for (int N = 0; N < 8 * n; N++)
    {
      for (int len = 0; len < n; len++)
      {
        draw_Xasc(8 * len + 40 - N, (str[len]) - ' ', 208, 16, 76);
      }
      WS.show();
      delay(50);
    }
    delay(2000);
  }
}
//读写EEPROM
void write_eeprom(int addr, String velue)
{
  int lenth = velue.length();
  for (int a = 0; a < lenth; a++)
  {
    EEPROM.write(addr + a, velue[a]);
  }
}
String read_eeprom(int addr, int lenth)
{
  String Text;
  for (int a = 0; a < lenth; a++)
  {
    Text += char(EEPROM.read(addr + a));
  }
  return Text;
  Serial.print(Text);
}
// MQTT 相关
void callback(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message arrived [");
  //  Serial.print(topic);
  //   Serial.print("] ");
  payload[length] = 0;
  deserializeJson(Mqtt_Sub, String((char *)payload)); //对接收到的MQTT_Message进行JSON解析
  // JSON文件格式：https://github.com/yu123an/esp8266/blob/master/EPS32/json.log
  //"Type": "weather",//消息类型，包含：weather，message，gif，
  String type = Mqtt_Sub["Type"];
  if (type == "time")
  {
    Serial.println("准备更新时间");
    Rtc.Begin(); // DS1307时间读写
    if (!Rtc.GetIsRunning())
    {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
    }
    int _hour = Mqtt_Sub["hour"];
    int _minute = Mqtt_Sub["minute"];
    int _second = Mqtt_Sub["second"];
    Rtc._SetDateTime( _second, _minute, _hour);
  }
  else if (type == "Temp")
  {
    Serial.println("I get temps !!!");
    Temp_out = Mqtt_Sub["temp"];
    Humidity_out = Mqtt_Sub["hump"];
    EEPROM.write(44, Temp_out);
    EEPROM.write(45, Humidity_out);
    for (int i = 0; i < 192; i++)
    {
      OutSide[i] = Mqtt_Sub["bmp"][i]["n"];
    }
  }
  else if (type == "message")
  {
    String msg = Mqtt_Sub["msg"];
    Serial.println(msg);
    draw_ascii(msg);
  }
  else if (type == "moon")
  {
    for (int i = 0; i < 192; i++)
    {
      EEPROM.write(320 + i, Mqtt_Sub["bmp"][i]["n"]);
    }
  }
  else if (type == "find")
  {
    int _addr = Mqtt_Sub["addr"];
    int _len = Mqtt_Sub["len"];
    String aa = read_eeprom(_addr, _len);
    String P = "{\"Type\":\"test\",\"data:\"";
    String Q = "\"}";
    String ALL = P + aa + Q;
    char _ALL[80];
    ALL.toCharArray(_ALL, 80);
    client.publish("outTopic", _ALL);
  }
  else if (type == "findN")
  {
    int _addr = Mqtt_Sub["addr"];
    Serial.println( EEPROM.read(_addr));
  }
  else if (type == "SunTime")
  {
    Sun_rise_hour = Mqtt_Sub["R_H"];
    Sun_rise_minute = Mqtt_Sub["R_M"];
    Sun_set_hour = Mqtt_Sub["S_H"];
    Sun_set_minute = Mqtt_Sub["S_M"];
    EEPROM.write(40, Sun_rise_hour);
    EEPROM.write(41, Sun_rise_minute);
    EEPROM.write(42, Sun_set_hour);
    EEPROM.write(43, Sun_set_minute);
  }
  else if (type == "calendar")
  {
    for (int i = 0; i < 192; i++)
    {
      EEPROM.write(128 + i, Mqtt_Sub["bmp"][i]["n"]);
    }
  }
  else if (type == "Brightness")
  {
    EEPROM.write(127, Mqtt_Sub["Brightness"]);
    light = EEPROM.read(127);
  }
  else if (type == "Color")
  {
    color_r = Mqtt_Sub["color"]["R"];
    color_g = Mqtt_Sub["color"]["G"];
    color_b = Mqtt_Sub["color"]["B"];
    Serial.println("Color");
  }
  else
  {
    Serial.println("New_thing");
  }
  EEPROM.commit();
}
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
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
void New_time()
{
  client.publish("outTopic", "{\"Type\":\"time\"}");
}
void Mqtt_Pub( String aa) {
  //char pub_msg = "{\"Type\":\"test\"}"
  String P = "{\"Type\":\"test\"";
  String Q = "}";
  String ALL = P + aa + Q;
  char _ALL[80];
  ALL.toCharArray(_ALL, 80);
  client.publish("outTopic", _ALL);
}
//获取SHT30温湿度
void get_sht30(String text, SHT31D result) {
  if (result.error == SHT3XD_NO_ERROR) {
    Temp_in = result.t;
    Humidity_in = result.rh;
  } else {
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(result.error);
  }
}
void Pub_msg(){
  
  String A = "{\"Type\":\"Msg\",\"Temp\":";
  String B = ",\"Hump\":";
  String C = ",\"Time_H\":";
  String D = ",\"Time_H\":";
  String E = ",\"Time_M\":";
  String F = ",\"Light\":";
  String G = "}";
  String ALL = A +String(Temp_in) + B + String(Humidity_in) + F + String(light) + G;
  char _ALL[800];
  ALL.toCharArray(_ALL, 800);
  client.publish("out_msg", _ALL);
}

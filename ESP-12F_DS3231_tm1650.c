#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#define sda 4
#define scl 5       //I2C接口
#define led A0      //光敏电阻
int i = 0;          //超时检测
const char *ssid     = "WiFi_Name";         //WiFi名称
const char *password = "WiFi_Pass";   //WiFi密码
/*
 *  数码管显示
 *  后续会加上水银开关，作为方向感应
 */
uint8_t num[10] = {                    
  //0   1     2     3     4     5     6     7     8     9
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7f, 0x6f
};
int ds_hour, ds_min, ds_sec;
int minute1, minute2, hour1, hour2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
/*
 * 联网更新时间函数
 * 每次上电时联网更新时间，并且写入到DS3231
 * 之后全部通过DS3231来获取时间，降低功耗
 */
void write_time() {
  WiFi.begin(ssid, password);         //联网
  while ( WiFi.status() != WL_CONNECTED ) {
    i++;
    delay ( 500 );
    Serial.print ( "." );
    if (i > 120) {                    //60秒后如果还是连接不上，就判定为连接超时
      Serial.print("连接超时！请检查网络环境");
      break;
    }
  }
  timeClient.begin();
  timeClient.update();
  //秒，分，时的获取及写入；
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  int Net_second = timeClient.getSeconds();                   //ss
  int Second = (Net_second / 10 * 16) + (Net_second % 10);
  Wire.write(Second);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x01);
  int Net_minute = timeClient.getMinutes();                   //mm
  int Minute = (Net_minute / 10 * 16) + (Net_minute % 10);
  Wire.write(Minute);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x02);
  int Net_hour = timeClient.getHours();                   //hh
  int Hour = (Net_hour / 10 * 16) + (Net_hour % 10);
  Wire.write(Hour);
  Wire.endTransmission();
  Serial.println("时间更新完成！！！");
  WiFi.disconnect(1);                     //时间更新完成后，断开连接，保持低功耗；
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("无线终端和接入点的连接已中断");
  }
  else
  {
    Serial.println("未能成功断开连接！");
  }
}
/*
 * 刷新时间
 * 读取DS3231的时间
 */
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
}
/*
 * 显示时间；通过TM1650显示时间
 * 亮度控制还没想到合适的方案
 */
void draw_time( int addr, int timer) {
  //int light = (analogRead(led) / 25);
  int light = 0x41;
  Wire.beginTransmission(0x24);
  Wire.write(light);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
  Wire.beginTransmission(addr);
  Wire.write(timer);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
}
void setup() {
  Wire.begin();
  Serial.begin(115200);
  timeClient.begin();
  timeClient.update();
  Serial.println ( timeClient.getSeconds() );
  Serial.println ("Time Begin");
  write_time();
}
/*
 * 循环显示时间，通过秒来控制时钟点的闪烁；
 * TM1650虽然是I2C接口，但是文档上的地址不对劲，
 * 其中指令地址为0x24;四个显示地址依次为:0x34,0x35,0x36,0x37；
 * 也就是文档上的地址右移一位，应该是空出来了读写控制位。
 */
void loop() {
  if (ds_hour == 1 && ds_sec == 12 )
    write_time();
  read_time();
  draw_time(0x34, num[hour1]);
  draw_time(0x35, num[hour2] ^ (ds_sec % 2 << 7));
  draw_time(0x36, num[minute1] ^ (ds_sec % 2 << 7));
  draw_time(0x37, num[minute2]);
  delay(499);
}

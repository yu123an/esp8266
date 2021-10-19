/*
  TM1650键盘扫描阵列
  +-------------------------------------------+
  |  ADD  |  DIG4  |  DIG3  |  DIG2  |  DIG1  |
  +-------------------------------------------+
  |   A   |  0x47  |  0x46  |  0x45  |  0x44  |
  +-------------------------------------------+
  |   B   |  0x4F  |  0x4E  |  0x4D  |  0x4C  |
  +-------------------------------------------+
  |   C   |  0x57  |  0x56  |  0x55  |  0x54  |
  +-------------------------------------------+
  |   D   |  0x5F  |  0x5E  |  0x5D  |  0x5C  |
  +-------------------------------------------+
  |   E   |  0x67  |  0x66  |  0x65  |  0x64  |
  +-------------------------------------------+
  |   F   |  0x6F  |  0x6E  |  0x6D  |  0x6C  |
  +-------------------------------------------+
  |   G   |  0x77  |  0x76  |  0x75  |  0x74  |
  +-------------------------------------------+
*/
/*
   外部引用
*/
#include <EEPROM.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
/*
   变量
*/
#define key_mian 0
#define key_1 0x56
#define key_2 0x4E
#define key_3 0x46
#define _pin_ena  13/*DS1302*/
#define _pin_clk  14
#define _pin_dat  12
#define BUZ 15
#define REG_BURST             0xBE
#define REG_WP                0x8E
#define led A0
int wifi_name_len;  //wifi name length
int wifi_pass_len;   //wifi pass length
int wifi_name_add = 1;
int wifi_pass_add = 41;
int wifi_name_len_add = 141;
int wifi_pass_len_add = 142;
int i = 0;            /*联网超时*/
int a = 1 ;           /*亮度等级*/
int flag = 0;
int debug = 1;
String ssid ;         //WiFi名称
String password ;   //WiFi密码
uint8_t second, minute, hour, day, mouth, dow, year;
uint8_t number[] = {
  //0   1     2     3     4     5     6     7     8     9
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF
};
/*
   实例
*/
WiFiManager wifi;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000);
/*
   蜂鸣器
*/
void dot() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(BUZ, 1);
    delay(2);
    digitalWrite(BUZ, 0);
    delay(2);
  }
  delay(1000);
}
/*
   中断配置
*/

/*
  DS1302驱动库
*/
void _nextBit()
{
  digitalWrite(_pin_clk, HIGH);
  delayMicroseconds(1);

  digitalWrite(_pin_clk, LOW);
  delayMicroseconds(1);
}
uint8_t _readByte()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    if (digitalRead(_pin_dat) == HIGH) byte |= 0x01 << b;
    _nextBit();
  }

  return byte;
}
void _writeByte(uint8_t value)
{
  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(_pin_dat, (value & 0x01) ? HIGH : LOW);
    _nextBit();
    value >>= 1;
  }
}
void _setDirection(int direction) {
  pinMode(_pin_dat, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000001 | address;
  _writeByte(command);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000000 | address;
  _writeByte(command);
}
void _end()
{
  digitalWrite(_pin_ena, LOW);
}

uint8_t _dec2bcd(uint8_t dec)
{
  return ((dec / 10 * 16) + (dec % 10));
}

uint8_t _bcd2dec(uint8_t bcd)
{
  return ((bcd / 16 * 10) + (bcd % 16));
}
void get_time() {
  _prepareRead(REG_BURST);
  second = _bcd2dec(_readByte() & 0b01111111);
  minute = _bcd2dec(_readByte() & 0b01111111);
  hour   = _bcd2dec(_readByte() & 0b00111111);
  day    = _bcd2dec(_readByte() & 0b00111111);
  mouth  = _bcd2dec(_readByte() & 0b00011111);
  dow    = _bcd2dec(_readByte() & 0b00000111);
  year   = _bcd2dec(_readByte() & 0b01111111);
  _end();
  if (debug) {
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
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  _writeByte(_dec2bcd(sec % 60 ));
  _writeByte(_dec2bcd(minute % 60 ));
  _writeByte(_dec2bcd(hour   % 24 ));
  _writeByte(_dec2bcd(day    % 32 ));
  _writeByte(_dec2bcd(mouth  % 13 ));
  _writeByte(_dec2bcd(dow    % 8  ));
  _writeByte(_dec2bcd(year   % 100));
  _writeByte(0b10000000);
  _end();
}
void set_time_mini(int sec, int minute, int hour) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  _writeByte(_dec2bcd(sec % 60 ));
  _writeByte(_dec2bcd(minute % 60 ));
  _writeByte(_dec2bcd(hour   % 24 ));
  _writeByte(0b10000000);
  _end();
}
/*
   读写EEPROM
*/
void write_eeprom(int addr, String velue) {
  int lenth = velue.length();
  for (int a = 0; a < lenth; a++) {
    EEPROM.write(addr + a, velue[a]);
  }
}
String read_eeprom(int addr, int lenth) {
  String Text;
  for (int a = 0; a < lenth; a++) {
    Text +=  char(EEPROM.read(addr + a));
  }
  return Text ;
  Serial.print(Text);
}
void draw_time(uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4) {
  // int light = (((analogRead(led) / 128) << 4) | 0x01);
  int light = 0x21;
  Wire.beginTransmission(0x24);
  Wire.write(light);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
  Wire.beginTransmission(0x34);
  Wire.write(num1);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
  Wire.beginTransmission(0x35);
  Wire.write(num2);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
  Wire.beginTransmission(0x36);
  Wire.write(num3);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
  Wire.beginTransmission(0x37);
  Wire.write(num4);
  while (Wire.endTransmission() != 0) {
    delayMicroseconds(5);
  }
}
void wifi_over() {
  draw_time(0x6F, 0x5C, 0x5C, 0x5E);
  delay(500);
  draw_time(0xEF, 0x5C, 0x5C, 0x5E);
  delay(500);
  draw_time(0x6F, 0xDC, 0x5C, 0x5E);
  delay(500);
  draw_time(0x6F, 0x5C, 0xDC, 0x5E);
  delay(500);
  draw_time(0x6F, 0x5C, 0x5C, 0xDE);
  delay(500);
}
void update_time() {
  timeClient.begin();
  timeClient.update();
  //秒，分，时的获取及写入；
  if ( timeClient.getMinutes() != 8) {
    //set_time(timeClient.getSeconds(), timeClient.getMinutes(), int timeClient.getHours(), int day, int mouth, int dow, int year);
    //set_time(timeClient.getSeconds(), timeClient.getMinutes(), timeClient.getHours(), 19, 10, 2, 21);
    set_time_mini(timeClient.getSeconds(), timeClient.getMinutes(), timeClient.getHours());
  } else {
    Serial.println("时间更新出错！！！");
  }
}
void Net() {
  ssid = read_eeprom(wifi_name_add, EEPROM.read(wifi_name_len_add));
  password = read_eeprom(wifi_pass_add, EEPROM.read(wifi_pass_len_add));
  WiFi.begin(ssid, password);         //联网
  while ( WiFi.status() != WL_CONNECTED ) {
    i++;
    delay ( 500 );
    Serial.print ( "." );
    if (i > 40) {                    //20秒后如果还是连接不上，就判定为连接超时
      Serial.println("");
      Serial.print("连接超时！请检查网络环境");
      Serial.println("");
      //wifimanager.resetSettings();
      wifi.setSaveConfigCallback(wifi_over);
      wifi.setDebugOutput(0);//关闭Debug调试
      wifi.setTimeout(120);//配网超时2分钟
      wifi.autoConnect("Hua_Clock");
      ssid = String(WiFi.SSID());
      password = String(WiFi.psk());
      wifi_name_len = ssid.length();
      wifi_pass_len = password.length();
      write_eeprom(wifi_name_add, ssid);
      write_eeprom(wifi_pass_add, password);
      EEPROM.write(wifi_name_len_add, wifi_name_len);
      EEPROM.write(wifi_pass_len_add, wifi_pass_len);
      EEPROM.commit();
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("已联网，准备更新时间！！！");
    update_time();
    Serial.println("时间更新完成！！！");
  }
  else {
    Serial.println("更新时间出错？？？");
  }
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
   中断函数
*/
ICACHE_RAM_ATTR void setting() {
  flag = 1 - flag;
  if (flag) {
    Serial.print("Flag is ");
    Serial.println(flag);
    for (int i = 300; i > 0; i--) {
      draw_time(number[i / 60 / 10], number[i / 60 % 10 + 10], number[i % 60 / 10 + 10], number[i % 60 % 10]);
      for (int j = 0; j < 999; j++) {
        delayMicroseconds(1000);
      }
      ESP.wdtFeed();
    }
    //flag = 0;
  } else {
    Serial.print("Now the Flag is ");
    Serial.println(flag);
  }
}
/*
   主函数
*/
void setup() {
  // put your setup code here, to run once:
  pinMode(key_mian, INPUT);
  pinMode(_pin_ena, OUTPUT);
  pinMode(_pin_clk, OUTPUT);
  pinMode(_pin_dat, INPUT);
  digitalWrite(_pin_ena, LOW);
  digitalWrite(_pin_clk, LOW);
  pinMode(BUZ, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(key_mian), setting, FALLING);
  Wire.begin(5, 4);
  EEPROM.begin(512);
  Serial.begin(9600);
  // Net();
  // delay(200);
}

void loop() {
  //Net();
  get_time();
  draw_time(number[hour / 10], number[hour % 10 + 10], number[minute / 10 + 10], number[minute % 10]);
  // draw_time(0xff,0xff,0xff,0xff);
  //delay(2000);
  if (minute % 10 == 0) {
    dot();
    if (minute / 10 == 0) {
      Net();
      draw_time(0x6F, 0x5C, 0x5C, 0x5E);
      delay(500);
    }
  } else {
    delay(1000);
  }
  delay((60 - second) * 1000);
  //delay(2000);
}

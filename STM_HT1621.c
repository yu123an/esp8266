
//字模
//七段
/*                          +----------------------+
    A                       +---+ G F A B E D C Dp |
  +---------+                 | 0 | 0 1 1 1 1 1 1 0  |    0X7E
  |         |                 | 1 | 0 0 0 1 0 0 1 0  |    0X12
  | F       | B               | 2 | 1 0 1 1 1 1 0 0  |    0XBC
  |         |                 | 3 | 1 0 1 1 0 1 1 0  |    0XB6
  |         |                 | 4 | 1 1 0 1 0 0 1 0  |    0XD2
  +----G----+                 | 5 | 1 1 1 0 0 1 1 0  |    0XE5
  |         |                 | 6 | 1 1 1 0 1 1 1 0  |    0XEE
  | E       | C               | 7 | 0 0 1 1 0 0 1 0  |    0X32
  |         |                 | 8 | 1 1 1 1 1 1 1 0  |    0XFE
  |         |                 | 9 | 1 1 1 1 0 1 1 0  |    0XF5
  +---------+                 +---+------------------+-----
    D
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
#define cs_HT PD3 //PB4 /*HT1621*/
//#define clk PA1   //PA3
//#define dat PA2 //PA2
/*
  //此处为从左至右进行操作
  //#define BIAS 0x52    //0b1000 0101 0010  1/3duty 4com
  #define BIAS 0x40    //0b1000 0x00 0000  1/2duty 2com
  #define SYSDIS 0X00  //0b1000 0000 0000  关振系统荡器和LCD偏压发生器
  #define SYSEN 0X02   //0b1000 0000 0010 打开系统振荡器
  #define LCDOFF 0X04  //0b1000 0000 0100  关LCD偏压
  #define LCDON 0X06   //0b1000 0000 0110  打开LCD偏压
  #define XTAL 0x28    //0b1000 0010 1000 外部接时钟
  #define RC256 0X30   //0b1000 0011 0000  内部时钟
  #define TONEON 0X12  //0b1000 0001 0010  打开声音输出
  #define TONEOFF 0X10 //0b1000 0001 0000 关闭声音输出
  #define WDTDIS1 0X0A //0b1000 0000 1010  禁止看门狗
*/
//先更改至从右至左进行写入
#define BIAS 0x02    //0b1000 0100 0000  1/2duty 2com
#define SYSDIS 0X00  //0b1000 0000 0000  关振系统荡器和LCD偏压发生器
#define SYSEN 0X40   //0b1000 0000 0010 打开系统振荡器
#define LCDOFF 0X20  //0b1000 0000 0100  关LCD偏压
#define LCDON 0X60   //0b1000 0000 0110  打开LCD偏压
#define XTAL 0x14    //0b1000 0010 1000 外部接时钟
#define RC256 0X0C   //0b1000 0011 0000  内部时钟
#define TONEON 0X48  //0b1000 0001 0010  打开声音输出
#define TONEOFF 0X08 //0b1000 0001 0000 关闭声音输出
#define WDTDIS1 0X50 //0b1000 0000 1010  禁止看门狗

#define cs_DS PD3 //PC6 /*DS1302*/
#define clk PA1   //PA3
#define dat PA2   //PA2
#define REG_BURST 0xBE
#define REG_WP 0x8E

int DIS_FLAG = 0; /*DisPlay Flag*/
int CHANGE_NUMER;
uint8_t Light;
//数组
uint8_t REG_FLAG[] = {
  /*8   C     L     S     F     H     d     y     n*/
  0xFF, 0xE2, 0xE4, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8C
};
uint8_t left_FLAG[] = {
  /*8   C     L     S     F     H     d     y     n*/
  0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x7F
};
uint8_t IMG_FLAG[] = {
  /*8   C     L     S     F     H     d     y     n*/
  0xFF, 0x39, 0x38, 0x6D, 0x71, 0x76, 0x5E, 0x6E, 0x54
};
uint8_t second, minute, hour, day, mouth, dow, year;
int debug = 1;
int Deadline = 0;
uint8_t number[] = {
  //0   1     2     3     4     5     6     7     8     9
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF
};

uint8_t num_DIS[] = {
  0x7e, 0x12, 0xBC, 0xB6, 0xD2, 0xE5, 0xEE, 0x32, 0xFE, 0xF5
};

//R&W data
//统一为从右至左操作
//按照DS3102的逻辑进行处理

void Write_data(uint8_t data, uint8_t Long)
{
  for (int i = 0; i < Long; i++)
  {
    digitalWrite(clk, LOW);
    delayMicroseconds(2);
    digitalWrite(dat, (data & 0x01) ? HIGH : LOW);
    digitalWrite(clk, HIGH);
    delayMicroseconds(2);
    data >>= 1;
  }
}

uint8_t Read_data()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(clk, LOW);
    delayMicroseconds(2);
    if (digitalRead(dat) == HIGH)
      byte |= 0x01 << b;
    digitalWrite(clk, HIGH);
    delayMicroseconds(2);
  }
  return byte;
}
//DS1302  操作
void _setDirection(int direction)
{
  pinMode(dat, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(cs_DS, HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000001 | address;
  Write_data(command, 8);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(cs_DS, HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000000 | address;
  Write_data(command, 8);
}
void _end()
{
  digitalWrite(cs_DS, LOW);
  //digitalWrite(cs_HT, LOW);
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
    Serial_print_s("Now time is ");
    Serial_print_i(year);
    Serial_print_s("-");
    Serial_print_i(mouth);
    Serial_print_s("-");
    Serial_print_i(day);
    Serial_print_s("-");
    Serial_print_i(hour);
    Serial_print_s(":");
    Serial_print_i(minute);
    Serial_print_s(":");
    Serial_println_i(second);
  }
}
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year)
{
  _prepareWrite(REG_WP);
  Write_data(0b00000000, 8);
  _end();
  _prepareWrite(REG_BURST);
  Write_data(_dec2bcd(sec % 60), 8);
  Write_data(_dec2bcd(minute % 60), 8);
  Write_data(_dec2bcd(hour % 24), 8);
  Write_data(_dec2bcd(day % 32), 8);
  Write_data(_dec2bcd(mouth % 13), 8);
  Write_data(_dec2bcd(dow % 8), 8);
  Write_data(_dec2bcd(year % 100), 8);
  Write_data(0b10000000, 8);
  _end();
}
void DS1302_INT()
{
  Serial_begin(9600);
  pinMode(cs_DS, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(dat, INPUT);
  digitalWrite(cs_DS, LOW);
  digitalWrite(clk, LOW);
}

//HT1621  操作
//待制作
// 指定地址写数据
void Write_dataClkData(uint8_t addr, uint8_t data)
{
  //addr <<= 2;
  digitalWrite(cs_HT, LOW);
  Write_data(0x05, 3);
  Write_data(addr, 6);
  Write_data(data, 4);
  digitalWrite(cs_HT, HIGH);
}
//连续地址写数据
void Write_dataClkData_(uint8_t addr, uint8_t data)
{
  //addr <<= 2;
  digitalWrite(cs_HT, LOW);
  Write_data(0x05, 3);
  Write_data(addr, 6);
  for (int m = 0; m < 32; m++)
  {
    //Write_data(((data >> m) & 0x01) << 3, 4); //待补充
    // Write_data(0xf0 >> (4 * ((data >> (8 - m)) & 0x01)), 4);
    Write_data(data, 4);
  }
  digitalWrite(cs_HT, HIGH);
}
// 写指令
void Write_dataCmd(uint8_t CMD)
{
  digitalWrite(cs_HT, LOW);
  Write_data(0x01, 4); //写入指令ID
  Write_data(CMD, 8);
  digitalWrite(cs_HT, HIGH);
}

//初始化配置
void HT1621_begin()
{
  Write_dataCmd(BIAS);
  Write_dataCmd(RC256);
  Write_dataCmd(SYSDIS);
  Write_dataCmd(WDTDIS1);
  Write_dataCmd(SYSEN);
  Write_dataCmd(LCDON);
}
//HT1621初始化
void HT1621_INT()
{
  pinMode(cs_HT, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(dat, OUTPUT);
  HT1621_begin();
}

void setup()
{
  DS1302_INT();
  HT1621_INT();
  set_time(0, 55, 11, 2, 11, 5, 19);
}

void loop()
{
  Write_dataClkData_(0, 0xff);
  delay(2000);
  Write_dataClkData_(0, 0x00);
  delay(2000);
  get_time();
  HT1621_INT();
}

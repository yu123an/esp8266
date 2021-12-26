#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//宏定义
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
//引脚声明
#define DIS_CLK 0
#define DIS_SDA 2
#define DIS_DC 4
#define DIS_CE 5
#define DIS_RST 16
#define RTC_SDA 2
#define RTC_SCL 0
#define RTC_CE 5
#define BUZ 15
#define KEY_1 14
#define KEY_2 13
#define KEY_3 12
#define debug 1
#define REG_BURST 0xBE
#define REG_WP 0x8E
uint8_t second, minute, hour, day, mouth, dow, year;
Adafruit_PCD8544 display = Adafruit_PCD8544(DIS_CLK, DIS_SDA, DIS_DC, DIS_CE, DIS_RST);
uint8_t Big_num[] = {
    0XFC, 0XFA, 0XF6, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0XFF, 0X7F, 0X3F, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3F, 0X7F, 0XFF, 0XFE, 0XFC, 0XF8, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF8, 0XFC, 0XFE, 0X7F, 0XBF, 0XDF, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //0
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF0, 0XF8, 0XFC, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3F, 0X7F, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF8, 0XFC, 0XFE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X3F, 0X7F, //1
    0X00, 0X02, 0X06, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0X00, 0X00, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0XBF, 0X7F, 0XFF, 0XFE, 0XFD, 0XFB, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X01, 0X00, 0X7F, 0XBF, 0XDF, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0X80, 0X00, 0X00, //2
    0X00, 0X02, 0X06, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0X00, 0X00, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0XBF, 0X7F, 0XFF, 0X00, 0X01, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X00, 0X80, 0XC0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //3
    0XFC, 0XF8, 0XF0, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF0, 0XF8, 0XFC, 0XFF, 0X7F, 0XBF, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0XBF, 0X7F, 0XFF, 0X00, 0X01, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X3F, 0X7F, //4
    0XFC, 0XFA, 0XF6, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X06, 0X02, 0X00, 0XFF, 0X7F, 0XBF, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X00, 0X00, 0X00, 0X01, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X00, 0X80, 0XC0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //5
    0XFC, 0XFA, 0XF6, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X06, 0X02, 0X00, 0XFF, 0X7F, 0XBF, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X00, 0X00, 0XFE, 0XFD, 0XFB, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X7F, 0XBF, 0XDF, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //6
    0X00, 0X02, 0X06, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X3F, 0X7F, 0XFF, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0XF8, 0XFC, 0XFE, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X1F, 0X3F, 0X7F, //7
    0XFC, 0XFA, 0XF6, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0XFF, 0X7F, 0XBF, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0XBF, 0X7F, 0XFF, 0XFE, 0XFD, 0XFB, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X7F, 0XBF, 0XDF, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //8
    0XFC, 0XFA, 0XF6, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0X0E, 0XF6, 0XFA, 0XFC, 0XFF, 0X7F, 0XBF, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0X80, 0XBF, 0X7F, 0XFF, 0X00, 0X01, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0X03, 0XFB, 0XFD, 0XFE, 0X00, 0X80, 0XC0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XE0, 0XDF, 0XBF, 0X7F, //9
};
uint8_t MINI_num[] = {
    0x00, 0x00, 0x78, 0x44, 0x3C, 0x00, 0x00, 0x00, //0
    0x00, 0x00, 0x08, 0x7C, 0x00, 0x00, 0x00, 0x00, //1
    0x00, 0x00, 0x64, 0x54, 0x48, 0x00, 0x00, 0x00, //2
    0x00, 0x00, 0x44, 0x54, 0x2C, 0x00, 0x00, 0x00, //3
    0x00, 0x00, 0x38, 0x24, 0x70, 0x00, 0x00, 0x00, //4
    0x00, 0x00, 0x5C, 0x54, 0x34, 0x00, 0x00, 0x00, //5
    0x00, 0x00, 0x78, 0x54, 0x34, 0x00, 0x00, 0x00, //6
    0x00, 0x00, 0x04, 0x74, 0x0C, 0x00, 0x00, 0x00, //7
    0x00, 0x00, 0x68, 0x54, 0x2C, 0x00, 0x00, 0x00, //8
    0x00, 0x00, 0x18, 0x54, 0x3C, 0x00, 0x00, 0x00, //9
    0x00, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, //Time Dot
    0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, //ALL_1
    0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x3C, 0x3C, 0x00, //ALL_2
    0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x42, 0x42, //HALF_1
    0x42, 0x42, 0x42, 0x42, 0x7E, 0x3C, 0x3C, 0x00, //HALF_2
    0x00, 0x7E, 0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, //NULL_1
    0x42, 0x42, 0x42, 0x42, 0x7E, 0x3C, 0x3C, 0x00, //NULL_2
    0x00, 0xC0, 0x00, 0xE0, 0x00, 0xF8, 0x00, 0xFE, //WIFI
    0x00, 0x00, 0x14, 0x08, 0x14, 0xC0, 0xE0, 0x00, //NoWiFi

};
uint8_t Big_letter[] = {
    0x00, 0x00, 0x38, 0x14, 0x12, 0x3C, 0x00, 0x00, //A
    0x00, 0x00, 0x3E, 0x2A, 0x2A, 0x14, 0x00, 0x00, //B
    0x00, 0x00, 0x1C, 0x22, 0x22, 0x14, 0x00, 0x00, //C
    0x00, 0x00, 0x3E, 0x22, 0x12, 0x0C, 0x00, 0x00, //D
    0x00, 0x00, 0x3E, 0x2A, 0x2A, 0x00, 0x00, 0x00, //E
    0x00, 0x00, 0x3E, 0x0A, 0x0A, 0x00, 0x00, 0x00, //F
    0x00, 0x00, 0x1C, 0x22, 0x2A, 0x3A, 0x00, 0x00, //G
    0x00, 0x00, 0x3E, 0x08, 0x08, 0x3E, 0x00, 0x00, //H
    0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, //I
    0x00, 0x00, 0x10, 0x20, 0x1E, 0x00, 0x00, 0x00, //J
    0x00, 0x00, 0x3E, 0x08, 0x14, 0x22, 0x00, 0x00, //K
    0x00, 0x00, 0x3E, 0x20, 0x20, 0x00, 0x00, 0x00, //L
    0x00, 0x00, 0x3E, 0x04, 0x08, 0x04, 0x3E, 0x00, //M
    0x00, 0x00, 0x3E, 0x04, 0x08, 0x3E, 0x00, 0x00, //N
    0x00, 0x00, 0x1C, 0x22, 0x22, 0x1C, 0x00, 0x00, //O
    0x00, 0x00, 0x3E, 0x0A, 0x0A, 0x04, 0x00, 0x00, //P
    0x00, 0x00, 0x1C, 0x2A, 0x12, 0x2C, 0x00, 0x00, //Q
    0x00, 0x00, 0x3E, 0x0A, 0x1A, 0x24, 0x00, 0x00, //R
    0x00, 0x00, 0x24, 0x2A, 0x2A, 0x10, 0x00, 0x00, //S
    0x00, 0x00, 0x02, 0x3E, 0x02, 0x00, 0x00, 0x00, //T
    0x00, 0x00, 0x1E, 0x20, 0x20, 0x1E, 0x00, 0x00, //U
    0x00, 0x00, 0x1E, 0x20, 0x10, 0x0E, 0x00, 0x00, //V
    0x00, 0x00, 0x0E, 0x30, 0x0C, 0x30, 0x0E, 0x00, //W
    0x00, 0x00, 0x36, 0x08, 0x08, 0x36, 0x00, 0x00, //X
    0x00, 0x00, 0x06, 0x28, 0x18, 0x06, 0x00, 0x00, //Y
    0x00, 0x00, 0x32, 0x2A, 0x26, 0x00, 0x00, 0x00, //Z
};

void dis_num(int x, int y, int num)
{
  for (int m = 0; m < 16; m++)
  {
    for (int n = 0; n < 32; n++)
    {
      display.drawPixel(x + m, y + n, (Big_num[num * 64 + (n / 8) * 16 + m] >> (n % 8)) & 0x01);
    }
  }
}
void dis_latter(int x, int y, int num)
{
  for (int m = 0; m < 8; m++)
  {
    for (int n = 0; n < 8; n++)
    {
      display.drawPixel(x + m, y + n, (Big_letter[num * 8 + m] >> n) & 0x01);
    }
  }
}
void dis_little_mun(int x, int y, int num)
{
  for (int m = 0; m < 8; m++)
  {
    for (int n = 0; n < 8; n++)
    {
      display.drawPixel(x + m, y + n, (MINI_num[num * 8 + m] >> n) & 0x01);
    }
  }
}
void dis_top()
{
  get_time();
  dis_little_mun(40, 0, 10); //Time Dot
  dis_little_mun(24, 0, hour / 10);
  dis_little_mun(32, 0, hour % 10);
  dis_little_mun(44, 0, minute / 10);
  dis_little_mun(52, 0, minute % 10);
  dis_little_mun(8, 0, 17); //WiFi Status
  dis_little_mun(68, 0, 11); //Bat_1
  dis_little_mun(76, 0, 12); //Bat_2
}
void testdrawchar(void)
{
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++)
  {
    if (i == '\n')
      continue;
    display.write(i);
    //if ((i > 0) && (i % 14 == 0))
    //display.println();
  }
  display.display();
}

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
  digitalWrite(RTC_SDA,1);
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

void buzzer()
{
  for (int t = 0; t < 2; t++)
  {
    digitalWrite(BUZ, 1);
    for (int i = 0; i < 10; i++)
    {
      delayMicroseconds(1000);
    }
    digitalWrite(BUZ, 0);
    for (int i = 0; i < 30; i++)
    {
      delayMicroseconds(1000);
    }
  }
}
ICACHE_RAM_ATTR void KEY_O()
{
  for (int i = 0; i < 300; i++)
  {
    delayMicroseconds(1000);
  }
  Serial.println("The button Key_1 has been press !!!");
  buzzer();
}
ICACHE_RAM_ATTR void KEY_T()
{
  for (int i = 0; i < 300; i++)
  {
    delayMicroseconds(1000);
  }
  Serial.println("The button Key_2 has been press !!!");
   buzzer();
}
ICACHE_RAM_ATTR void KEY_TH()
{
 for (int i = 0; i < 300; i++)
  {
    delayMicroseconds(1000);
  }
  Serial.println("The button Key_3 has been press !!!");
   buzzer();
}
void testdrawline()
{
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, 0, i, display.height() - 1, BLACK);
    display.display();
  }
  for (int16_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(0, 0, display.width() - 1, i, BLACK);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, display.height() - 1, i, 0, BLACK);
    display.display();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, BLACK);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = display.width() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, BLACK);
    display.display();
  }
  for (int16_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, BLACK);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, 0, i, BLACK);
    display.display();
  }
  for (int16_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, BLACK);
    display.display();
  }
  delay(250);
}
void setup()
{
  display.begin();
  //display.setContrast(75);
  DS1302_INT();
  Serial.begin(9600);
  pinMode(KEY_1, INPUT_PULLUP);
  pinMode(KEY_2, INPUT_PULLUP);
  pinMode(KEY_3, INPUT_PULLUP);
  pinMode(BUZ, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(KEY_1), KEY_O, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEY_2), KEY_T, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEY_3), KEY_TH, FALLING);
}
void loop()
{
  /*for (int i = 0; i < 10; i++)
  {
    dis_num(50, 0, i);
    display.display();
    delay(20);
  }*/
  display.clearDisplay();
  display.display();
  get_time();
  dis_num(3, 9, hour / 10);
  dis_num(21, 9, hour % 10);
  dis_num(47, 9, minute / 10);
  dis_num(66, 9, minute % 10);
  /*
  for (int d = 7; d < 10; d++)
  {
    display.drawPixel(41, d, 1);
    display.drawPixel(42, d, 1);
  }
  for (int d = 23; d < 26; d++)
  {
    display.drawPixel(41, d, 1);
    display.drawPixel(42, d, 1);
  }
  
  dis_latter(0, 0, 0);
  dis_latter(8, 0, 1);
  dis_latter(16, 0, 2);
  dis_latter(24, 0, 3);
  dis_latter(32, 0, 4);
  dis_latter(40, 0, 5);
  dis_little_mun(48, 0, 0);
  dis_little_mun(56, 0, 1);
  dis_little_mun(64, 0, 2);
  dis_little_mun(72, 0, 3);
  */
 dis_top();
  display.display();
  digitalWrite(RTC_SDA,1);
  //delay((61 - minute) * 1000);
  delay(6000);
 // pinMode(DIS_SDA,INPUT);
 //digitalWrite(RTC_SDA,0);
  ESP.deepSleep((61 - minute) * 1000000);
}

/*
  Key:
  +-------+-------+-------+-------+
  |--PB5--+--PD2--+--PC7--+--PA1--|
  +-------+-------+-------+-------+
*/
//字模
//七段
/*                          +----------------------+
   // A                       +---+ G F A B E D C Dp |    0-7     7-0
  +---------+                 | 0 | 0 1 1 1 1 1 1 0  |    0X7E    0x7E
  |         |                 | 1 | 0 0 0 1 0 0 1 0  |    0X12    0x48
  | F       | B               | 2 | 1 0 1 1 1 1 0 0  |    0XBC    0x3D
  |         |                 | 3 | 1 0 1 1 0 1 1 0  |    0XB6    0x6D
  |         |                 | 4 | 1 1 0 1 0 0 1 0  |    0XD2    0x4B
  +----G----+                 | 5 | 1 1 1 0 0 1 1 0  |    0XE5    0x67
  |         |                 | 6 | 1 1 1 0 1 1 1 0  |    0XEE    0x77
  | E       | C               | 7 | 0 0 1 1 0 0 1 0  |    0X32    0x4C
  |         |                 | 8 | 1 1 1 1 1 1 1 0  |    0XFE    0x7F
  |         |                 | 9 | 1 1 1 1 0 1 1 0  |    0XF5    0x6F
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
//阵脚引用待调整
#include <Arduino.h>
#define cs_HT PB4 //PB4,LCD_NEW  /*HT1621*/
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
//现更改至从右至左进行写入
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

#define cs_DS PC6 //PC6,LCD_NEW /*DS1302*/
#define clk PA3   //PA3
#define dat PA2   //PA2
#define REG_BURST 0xBE
#define REG_WP 0x8E
#define Hg PD4 //水银开关

int DIS_FLAG = 0; /*DisPlay Flag*/
int CHANGE_NUMER;
uint8_t Light;
//数组
uint8_t REG_FLAG[] = {
    /*8   C     S     F     H     d     y     n*/
    0xFF, 0xE2, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8C};
uint8_t left_FLAG[] = {
    /*8   C     S     F     H     d     y     n*/
    0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x7F};
uint8_t IMG_FLAG[] = {
    /*8   C     S     F     H     d     y     n*/
    0xFF, 0x36, 0x67, 0x17, 0x5b, 0x5E, 0x6E, 0x54};
uint8_t second, minute, hour, day, mouth, dow, year;
int debug = 0;
int Deadline = 0;
/*
  //TM1650数字字模
  uint8_t number[] = {
    //0   1     2     3     4     5     6     7     8     9
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
    0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF};
  uint8_t num_DIS[] = {
   //0   1     2     3     4     5     6     7     8     9
    0x7e, 0x12, 0xBC, 0xB6, 0xD2, 0xE5, 0xEE, 0x32, 0xFE, 0xF5};
*/
uint8_t num_HT[] = {
    0x7E, 0x48, 0x3D, 0x6D, 0x4B, 0x67, 0x77, 0x4C, 0x7F, 0x6F,
    0x7E, 0x12, 0x3D, 0x37, 0x53, 0x67, 0x6F, 0x32, 0x7F, 0x77};
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
void Write_addr(uint8_t data, uint8_t Long)
{
  for (int i = 0; i < Long; i++)
  {
    digitalWrite(clk, LOW);
    delayMicroseconds(2);
    digitalWrite(dat, (data & 0x80) ? HIGH : LOW);
    digitalWrite(clk, HIGH);
    delayMicroseconds(2);
    data <<= 1;
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
  pinMode(dat, OUTPUT);
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
// 指定地址写数据
void Write_dataClkData(uint8_t addr, uint8_t data)
{
  addr <<= 2;
  digitalWrite(cs_HT, LOW);
  Write_data(0x05, 3);
  Write_addr(addr, 6);
  Write_data(data, 4);
  digitalWrite(cs_HT, HIGH);
}
//连续地址写数据
void Write_dataClkData_(uint8_t addr, uint8_t data)
{
  addr <<= 2;
  digitalWrite(cs_HT, LOW);
  Write_data(0x05, 3);
  Write_addr(addr, 6);
  for (int m = 0; m < 8; m++)
  {
    //data由高到低进行取模
    //Write_data(((data >> m) & 0x01) << 3, 4); //待补充
    // Write_data(0xf0 >> (4 * ((data >> (8 - m)) & 0x01)), 4);
    Write_data((data & (0x01 << m)) >> m, 4);
    // Write_data(data, 4);
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
//显示字符，四位一体
void HT_DIS(uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4)
{
  digitalWrite(cs_HT, LOW);
  Write_data(0x05, 3);
  Write_addr(0 << 2, 6);
  for (int m = 0; m < 8; m++)
  {
    Write_data((num1 & (0x01 << m)) >> m, 4);
  }
  for (int m = 0; m < 8; m++)
  {
    Write_data((num2 & (0x01 << m)) >> m, 4);
  }
  for (int m = 0; m < 8; m++)
  {
    Write_data((num3 & (0x01 << m)) >> m, 4);
  }
  for (int m = 0; m < 8; m++)
  {
    Write_data((num4 & (0x01 << m)) >> m, 4);
  }
  digitalWrite(cs_HT, HIGH);
}
//ADC电量检测
void ADC_INT()
{
  GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_FL_NO_IT);

  ADC1_DeInit(); //ADC1_CONVERSIONMODE_CONTINUOUS

  ADC1_Init(ADC1_CONVERSIONMODE_SINGLE, //单次转换

            ADC1_CHANNEL_4, //通道4

            ADC1_PRESSEL_FCPU_D4, //预定标器选择 分频器  fMASTER 可以被分频 2 到 18

            ADC1_EXTTRIG_TIM, //从内部的TIM1 TRGO事件转换

            DISABLE, //是否使能该触发方式

            ADC1_ALIGN_RIGHT, //对齐方式（可以左右对齐）

            ADC1_SCHMITTTRIG_CHANNEL4, //指定触发通道

            ENABLE); //是否使能指定触发通道

  ADC1_Cmd(ENABLE);
}
uint16_t ADC_GET()
{
  uint16_t value, temph;

  uint8_t templ;

  ADC1_StartConversion();

  // 定义templ存储低8位数据  temph存储高8位数据

  while (!(ADC1->CSR & 0x80))
    ; //等待转换完成

  templ = ADC1->DRL;

  temph = ADC1->DRH; //读取ADC转换  在左对齐和右对齐模式下 读取数据的顺序不同  参考STM8寄存器.PDFP371

  value = (templ | (temph << 8)); //注意是10位的转换精度 value、temph应为unsigned int 变量
  float charing = value * 6.6 * 100 / 1024;
  return charing;
}
int DIS_BAT()
{
  int adc = 0;
  Serial_print_s("The battery is :");
  for (int nA = 0; nA < 8; nA++)
  {
    adc += ADC_GET();
  }
  Serial_println_i(adc >> 3);
  return adc >> 3;
}

//中断函数
void PA1_INT()
{
  if (DIS_FLAG == 9)
  {
    DIS_FLAG = 0;
    Deadline = 0;
  }
  else
  {
    DIS_FLAG += 1;
  }
  if (DIS_FLAG == 4)
  {
    DIS_FLAG = 0;
  }
}
void PB5_INT()
{

  DIS_FLAG = 9;
}
void PC7_INT()
{
  _prepareRead(REG_FLAG[DIS_FLAG]);
  CHANGE_NUMER = _bcd2dec(Read_data() & left_FLAG[DIS_FLAG]);
  _end();
  CHANGE_NUMER += 1;
  _prepareWrite(REG_WP);
  Write_data(0b00000000, 8);
  _end();
  _prepareWrite(REG_FLAG[DIS_FLAG]);
  Write_data(_dec2bcd(CHANGE_NUMER), 8);
  Write_data(0b10000000, 8);
  _end();
}
void PD2_INT()
{
  _prepareRead(REG_FLAG[DIS_FLAG]);
  CHANGE_NUMER = _bcd2dec(Read_data() & left_FLAG[DIS_FLAG]);
  _end();
  CHANGE_NUMER -= 1;
  _prepareWrite(REG_WP);
  Write_data(0b00000000, 8);
  _end();
  _prepareWrite(REG_FLAG[DIS_FLAG]);
  Write_data(_dec2bcd(CHANGE_NUMER), 8);
  Write_data(0b10000000, 8);
  _end();
}
//中断使能
void interrupt_int()
{

  /*
     STM INTERRUPT SETTING
  */
  //PA1
  GPIO_Init(GPIOA, GPIO_PIN_1, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTA & 0xff, PA1_INT, 0);
  //PB5
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTB & 0xff, PB5_INT, 0);
  //PC7
  GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTC & 0xff, PC7_INT, 0);
  //PD2
  GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOD, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTD & 0xff, PD2_INT, 0);
}

void setup()
{
  DS1302_INT();
  HT1621_INT();
  pinMode(Hg, INPUT);
  ADC_INT();
  //DS1302读写测试，平时注释掉即可
  // set_time(0, 55, 11, 2, 11, 5, 19);
  interrupt_int();
  Serial_println_s("aaaaaaaaaaaaa");
}

void loop()
{
  switch (DIS_FLAG)
  {
  case 0:
    get_time();
    //显示时间
    HT_DIS(num_HT[hour / 10], num_HT[hour % 10],
           num_HT[minute / 10], num_HT[minute % 10] + (((second % 2) << 7) & 0x80));
    // HT_DIS(0xff,0xff,0xff,0xff);
    if (second == 23)
    {
      int BAT = DIS_BAT();
      HT_DIS(num_HT[BAT / 100] + 0x80, num_HT[BAT % 100 / 10],
             num_HT[BAT % 100 % 10], 0x0B);
      delay(2000);
    }
    //   Serial_println_i(DIS_BAT());
    delay(998);
    break;
  case 9:
    _prepareRead(REG_FLAG[1]);
    CHANGE_NUMER = _bcd2dec(Read_data() & left_FLAG[1]);
    _end();
    if (Deadline < CHANGE_NUMER * 60)
    {
      HT_DIS(num_HT[(CHANGE_NUMER * 60 - Deadline) / 60 / 10],
             num_HT[(CHANGE_NUMER * 60 - Deadline) / 60 % 10],
             num_HT[(CHANGE_NUMER * 60 - Deadline) % 60 / 10],
             num_HT[(CHANGE_NUMER * 60 - Deadline) % 60 % 10] + 0x80);
    }
    else
    {
      DIS_FLAG = 0;
      Deadline = 0;
    }
    Deadline += 1;
    delay(998);
    break;
  default:
    _prepareRead(REG_FLAG[DIS_FLAG]);
    CHANGE_NUMER = _bcd2dec(Read_data() & left_FLAG[DIS_FLAG]);
    _end();
    pinMode(cs_HT, OUTPUT);
    pinMode(clk, OUTPUT);
    pinMode(dat, OUTPUT);
    HT_DIS(IMG_FLAG[DIS_FLAG], 0x01,
           num_HT[CHANGE_NUMER / 10], num_HT[CHANGE_NUMER % 10]);
    delay(99);
    break;
  }
}

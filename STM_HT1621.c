
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
//变量定义
#define cs_p PB4 /*HT1621*/
#define wr_p PA3
#define data_p PA2
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
#define _pin_ena PC6 /*DS1302*/
#define _pin_clk PA3
#define _pin_dat PA2
#define REG_BURST 0xBE
#define REG_WP 0x8E
int DIS_FLAG = 0; /*DisPlay Flag*/
int CHANGE_NUMER;
uint8_t Light;
//数组
uint8_t REG_FLAG[] = {
    /*8   C     L     S     F     H     d     y     n*/
    0xFF, 0xE2, 0xE4, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8C};
uint8_t left_FLAG[] = {
    /*8   C     L     S     F     H     d     y     n*/
    0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x3F, 0x1F, 0x7F};
uint8_t IMG_FLAG[] = {
    /*8   C     L     S     F     H     d     y     n*/
    0xFF, 0x39, 0x38, 0x6D, 0x71, 0x76, 0x5E, 0x6E, 0x54};
uint8_t second, minute, hour, day, mouth, dow, year;
int debug = 0;
int Deadline = 0;
uint8_t number[] = {
    //0   1     2     3     4     5     6     7     8     9
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
    0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF};

uint8_t num_DIS[] = {
    0x7e, 0x12, 0xBC, 0xB6, 0xD2, 0xE5, 0xEE, 0x32, 0xFE, 0xF5};
//ht1621操作
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
        //HT1621_Write(((data >> m) & 0x01) << 3, 4); //待补充
        HT1621_Write(0xf0 >> (4 * ((data >> (8 - m)) & 0x01)), 4);
    }
}
// 写指令
void HT1621_WriteCmd(uint8_t CMD)
{
    digitalWrite(cs_p, LOW);
    HT1621_Write(0x80, 4); //写入指令ID
    HT1621_Write(CMD, 8);
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
//DS1302操作
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
        if (digitalRead(_pin_dat) == HIGH)
            byte |= 0x01 << b;
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
void _setDirection(int direction)
{
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
void get_time()
{
    _prepareRead(REG_BURST);
    second = _bcd2dec(_readByte() & 0b01111111);
    minute = _bcd2dec(_readByte() & 0b01111111);
    hour = _bcd2dec(_readByte() & 0b00111111);
    day = _bcd2dec(_readByte() & 0b00111111);
    mouth = _bcd2dec(_readByte() & 0b00011111);
    dow = _bcd2dec(_readByte() & 0b00000111);
    year = _bcd2dec(_readByte() & 0b01111111);
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
    _writeByte(0b00000000);
    _end();
    _prepareWrite(REG_BURST);
    _writeByte(_dec2bcd(sec % 60));
    _writeByte(_dec2bcd(minute % 60));
    _writeByte(_dec2bcd(hour % 24));
    _writeByte(_dec2bcd(day % 32));
    _writeByte(_dec2bcd(mouth % 13));
    _writeByte(_dec2bcd(dow % 8));
    _writeByte(_dec2bcd(year % 100));
    _writeByte(0b10000000);
    _end();
}
void set_time_mini(int sec, int minute, int hour)
{
    _prepareWrite(REG_WP);
    _writeByte(0b00000000);
    _end();
    _prepareWrite(REG_BURST);
    _writeByte(_dec2bcd(sec % 60));
    _writeByte(_dec2bcd(minute % 60));
    _writeByte(_dec2bcd(hour % 24));
    _writeByte(0b10000000);
    _end();
}
/*
合并操作
兼容HT1621与DS1302；共用IO线与DAT线，节省IO消耗
区别：
HT：字节位数不定，数据左移；由高至低：01234567
DS：字节位数固定，数据右移；由低至高；76543210
Write_flag作为判断标志位，1位HT1621；0为DS1302
*/
void write_data(uint8_t data, uint8_t Long，bool Write_flag)
{
    if (Write_flag)
    {
        for (int i = 0; i < Long; i++)
        {
            digitalWrite(wr_p, LOW);
            delayMicroseconds(4);
            /* if (data & 0x80)
            {
                digitalWrite(data_p, HIGH);
            }
            else
            {
                digitalWrite(data_p, LOW);
            }*/
            digitalWrite(data_p, (data & 0x01) ? HIGH : LOW);
            digitalWrite(wr_p, HIGH);
            delayMicroseconds(4);
            data <<= 1;
        }
        else
        {
            for (uint8_t b = 0; b < 8; b++)
            {
                digitalWrite(_pin_dat, (data & 0x01) ? HIGH : LOW);
                _nextBit();
                data >>= 1;
            }
        }
    }
}
void dis_num(uint8_t addr, uint8_t data)
{
}
void PA3_INT()
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
    // dot();
    if (DIS_FLAG == 6)
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
    //   dot();
    _prepareRead(REG_FLAG[DIS_FLAG]);
    CHANGE_NUMER = _bcd2dec(_readByte() & left_FLAG[DIS_FLAG]);
    _end();
    CHANGE_NUMER += 1;
    _prepareWrite(REG_WP);
    _writeByte(0b00000000);
    _end();
    _prepareWrite(REG_FLAG[DIS_FLAG]);
    _writeByte(_dec2bcd(CHANGE_NUMER));
    _writeByte(0b10000000);
    _end();
}
void PD2_INT()
{
    // dot();
    _prepareRead(REG_FLAG[DIS_FLAG]);
    CHANGE_NUMER = _bcd2dec(_readByte() & left_FLAG[DIS_FLAG]);
    _end();
    CHANGE_NUMER -= 1;
    _prepareWrite(REG_WP);
    _writeByte(0b00000000);
    _end();
    _prepareWrite(REG_FLAG[DIS_FLAG]);
    _writeByte(_dec2bcd(CHANGE_NUMER));
    _writeByte(0b10000000);
    _end();
}
void INTERRUPT_INT()
{
    /*
     STM INTERRUPT SETTING
  */
    //PA3
    GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_FL_IT);
    disableInterrupts();
    EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, FALLING);
    enableInterrupts();
    attachInterrupt(INT_PORTA & 0xff, PA3_INT, 0);
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
void ADC_INT()
{
    GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_IN_FL_NO_IT);

    ADC1_DeInit(); //ADC1_CONVERSIONMODE_CONTINUOUS

    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE, //单次转换

              ADC1_CHANNEL_2, //通道

              ADC1_PRESSEL_FCPU_D4, //预定标器选择 分频器  fMASTER 可以被分频 2 到 18

              ADC1_EXTTRIG_TIM, //从内部的TIM1 TRGO事件转换

              DISABLE, //是否使能该触发方式

              ADC1_ALIGN_RIGHT, //对齐方式（可以左右对齐）

              ADC1_SCHMITTTRIG_CHANNEL2, //指定触发通道

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
    //Serial_println_i(adc >> 3);
    return adc >> 3;
}
void setup()
{
    Serial_begin(9600);
    HT1621_INT();
    //pinMode(BUZ, OUTPUT);
    pinMode(_pin_ena, OUTPUT);
    pinMode(_pin_clk, OUTPUT);
    pinMode(_pin_dat, INPUT);
    digitalWrite(_pin_ena, LOW);
    digitalWrite(_pin_clk, LOW);
    //pinMode(scl, OUTPUT);
    //pinMode(sda, OUTPUT);
    ADC_INT();
    Serial_println_s("!!!!!!!!!");
    /*
  */
    _prepareWrite(REG_WP);
    _writeByte(0b00000000);
    _end();
    _prepareWrite(REG_FLAG[1]);
    _writeByte(_dec2bcd(5));
    _prepareWrite(REG_FLAG[2]);
    _writeByte(_dec2bcd(1));
    _writeByte(0b10000000);
    _end();
}
void loop()
{
    for (int a = 1; a < 32; a += 2)
    {
        HT1621_WriteClkData(a, 0xff);
        delay(200);
    }

    _prepareRead(REG_FLAG[2]);
    Light = _bcd2dec(_readByte() & left_FLAG[2]) % 8 + 1;
    _end();
    switch (DIS_FLAG)
    {
    case 0:
        get_time();
        //dot();
        // dis_num(number[minute / 10], number[minute % 10], number[second / 10], number[second % 10]);
        dis_num(0x68, number[hour / 10]);
        dis_num(0x6A, number[hour % 10 + 10 * (second % 2)]);
        dis_num(0x6C, number[minute / 10 + 10 * (second % 2)]);
        dis_num(0x6E, number[minute % 10]);
        if ((minute == 0) && (second == 13))
        {
            if (DIS_BAT() < 370)
            {
                dis_num(0x68, 0);
                dis_num(0x6A, 0);
                dis_num(0x6C, 0);
                dis_num(0x6E, 0x01);
                for (uint16_t t = 0; t < 0xFFFF; t++)
                {
                    delay(9000);
                }
            }
        }
        /*else if (second == 23)
    {
      int BAT = DIS_BAT();
      dis_num(0x68, BAT / 100 + 10);
      dis_num(0x6A, BAT % 100 / 10);
      dis_num(0x6C, BAT % 100);
      dis_num(0x6E, 0x62);
    }*/
        delay(998);
        break;
    //倒计时显示
    case 9:
        _prepareRead(REG_FLAG[1]);
        CHANGE_NUMER = _bcd2dec(_readByte() & left_FLAG[1]);
        _end();
        if (Deadline < CHANGE_NUMER * 60)
        {
            dis_num(0x68, number[(CHANGE_NUMER * 60 - Deadline) / 60 / 10]);
            dis_num(0x6A, number[(CHANGE_NUMER * 60 - Deadline) / 60 % 10 + 10]);
            dis_num(0x6C, number[(CHANGE_NUMER * 60 - Deadline) % 60 / 10 + 10]);
            dis_num(0x6E, number[(CHANGE_NUMER * 60 - Deadline) % 60 % 10]);
        }
        else
        {
            DIS_FLAG = 0;
        }
        Deadline += 1;
        delay(998);
        break;
    default:
        _prepareRead(REG_FLAG[DIS_FLAG]);
        CHANGE_NUMER = _bcd2dec(_readByte() & left_FLAG[DIS_FLAG]);
        _end();
        dis_num(0x68, IMG_FLAG[DIS_FLAG]);
        dis_num(0x6A, 0x40);
        dis_num(0x6C, number[CHANGE_NUMER / 10]);
        dis_num(0x6E, number[CHANGE_NUMER % 10]);
        delay(99);
        break;
    }
}

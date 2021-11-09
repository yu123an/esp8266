#include <Arduino.h>

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
#define BUFFERSIZE 12
//字模
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
uint8_t num_DIS[] = {
    0x7e, 0x12, 0xBC, 0xB6, 0xD2, 0xE5, 0xEE, 0x32, 0xFE, 0xF5};
//HT1621初始化
void HT1621_INT()
{
    pinMode(cs_p, OUTPUT);
    pinMode(wr_p, OUTPUT);
    pinMode(data_p, OUTPUT);
    HT1621_begin();
}
//写数据
void HT1621_Write(uint8_t data, uint8_t Long)
{
    for (int i = 0; i < cnt; i++)
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
    digitalWrite(_cs_p, LOW);
    HT1621_Write(0xa0, 3);
    HT1621_Write(addr, 6);
    HT1621_Write(sdata, 4);
    digitalWrite(_cs_p, HIGH);
}
//连续地址写数据
void HT1621_WriteClkData_(uint8_t addr, uint8_t data)
{
    addr <<= 2;
    digitalWrite(_cs_p, LOW);
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
    digitalWrite(_cs_p, LOW);
    wrDATA(0x80, 4); //写入指令ID
    wrDATA(CMD, 8);
    digitalWrite(_cs_p, HIGH);
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

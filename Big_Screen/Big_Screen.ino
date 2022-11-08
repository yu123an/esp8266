#include "SPI.h"
#include "TFT_eSPI.h"
//#include "siyuan_20.h"
//定义颜色
#define c_BL tft.color24to16(0xc0ebd7)
#define c_Line tft.color24to16(0x426666)
#define c_text tft.color24to16(0x003371)
#define c_time tft.color24to16(0x333631)
//定义字体
#define Digi &DS_DIGI32pt7b
#define DejaVu &DejaVu_Sans_Mono_20
//实例化类
TFT_eSPI tft = TFT_eSPI();
void setup() {
  Serial.begin(9600);
tft.begin();
tft.setRotation(1);
draw_Class();
}

void loop() {
  // put your main code here, to run repeatedly:
delay(2000);
}
void draw_Class(){
  int _data = 12;
tft.fillScreen(c_BL);
tft.fillRect(0,0,480,30,c_text);
tft.setFreeFont(DejaVu);
tft.setTextColor(c_BL);
tft.drawString("Today is 2022/12/03",12,5);
tft.setTextColor(c_text);
tft.drawString(String(_data) + " A1 A2 A3 A4 B1 B2 B3 B4 C1 C2 C3",12,35);
for(int i = 0;i<11;i++){
  tft.drawLine(26+12+6+39*i,35,26+12+6+39*i,70,c_Line);
}
tft.drawLine(12,55,468,55,c_Line);
tft.drawString("D2       21    33    31    33 31 21",12,57);
tft.setTextColor(c_time);
tft.setFreeFont(Digi);
tft.drawString("22:33",12,80);
tft.drawRect(10,78,146,64,c_Line);
//String at ="落霞与孤鹜齐飞，秋水共长天一色";
//tft.loadFont(siyuan_20);
//tft.setCursor(121,120);
//tft.println(at);
}

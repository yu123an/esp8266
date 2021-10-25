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
#include <Wire.h>
int i = 0 ;
int light ;
void draw_time(uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4) {
  // int light = (((analogRead(led) / 128) << 4) | 0x01);
  //int light = 0x29;
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
ICACHE_RAM_ATTR void talk() {
  Wire.beginTransmission(0x24);
  Wire.write(0);
  Wire.requestFrom(0x24, 1);
  int b  = Wire.read();
  Serial.println(b, HEX);
  draw_time(0, 0, b / 16, b % 16);
}
ICACHE_RAM_ATTR void setting() {
  for (int j = 0; j < 50; j++) {
    delayMicroseconds(1000);
    ESP.wdtFeed();
  }
  i = 1 - i;
  Wire.beginTransmission(0x24);
  Wire.write(0);
  Wire.requestFrom(0x24, 1);
  int a  = Wire.read();
  /*  Serial.print("+-------------------+");
    Serial.print(a, HEX);
    Serial.println("+-------------------+");*/
  if (i) {
    light = 0x41;
    Serial.println("取消中断功能");
    detachInterrupt(13);
    Wire.beginTransmission(0x24);
    Wire.write(light);
    Wire.endTransmission();
  } else {
    light = 0x49;
    Serial.println("开启中断功能");
    Wire.beginTransmission(0x24);
    Wire.write(light);
    Wire.endTransmission();
    attachInterrupt(digitalPinToInterrupt(13), talk, FALLING);
  }
  draw_time(0xff, 0xff, 0, 0);
}
void setup() {
  light = 0x41;
  Wire.begin(5, 4);
  pinMode(13, INPUT);
  Serial.begin(9600);
  Wire.beginTransmission(0x24);
  Wire.write(0x49);
  Wire.endTransmission();
  attachInterrupt(digitalPinToInterrupt(0), setting, FALLING);
}
void loop() {
  draw_time(0, 0, 0, 0);
  delay(15000);
}

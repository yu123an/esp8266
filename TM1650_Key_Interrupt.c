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
ICACHE_RAM_ATTR void talk() {
  Wire.beginTransmission(0x24);
  Wire.write(0);
  Wire.requestFrom(0x24, 1);
  int a  = Wire.read();
  Serial.println(a, HEX);

}
int i = 0 ;
void setup() {
  Wire.begin(4, 5);
  pinMode(2, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(2),talk,FALLING);
  Serial.begin(9600);
  Wire.beginTransmission(0x24);
  Wire.write(0x49);
  Wire.endTransmission();
}
void loop() {
  i = 1 - i;
  Wire.beginTransmission(0x24);
  Wire.write(0);
  Wire.requestFrom(0x24, 1);
  int a  = Wire.read();
  Serial.print("+-------------------+");
  Serial.print(a, HEX);
  Serial.println("+-------------------+");
  if (i) {
    Serial.println("取消中断功能");
    detachInterrupt(2);
  } else {
    Serial.println("开启中断功能");
    attachInterrupt(digitalPinToInterrupt(2), talk, FALLING);
  }
  delay(5000);
}

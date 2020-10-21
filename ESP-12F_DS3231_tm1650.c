#include <Wire.h>
#define sda 4
#define scl 5
#define led A0
uint8_t num[10] = {
  //0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7f, 0x6f
};
int Net_hour, Net_min, Net_sec;
int ds_hour, ds_min, ds_sec;
int minute1, minute2, hour1, hour2;
void write_time() {
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  int valll = Net_sec;                   //ss
  int dd = (valll / 10 * 16) + (valll % 10);
  Wire.write(dd);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x01);
  int vallll = Net_min;                   //mm
  int ddd = (vallll / 10 * 16) + (vallll % 10);
  Wire.write(ddd);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x02);
  int vall = Net_hour;                   //hh
  int d = (vall / 10 * 16) + (vall % 10);
  Wire.write(d);
  Wire.endTransmission();
}
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
void draw_time( int addr, int timer) {
  int light = (analogRead(led) / 25);
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
  // pinMode(A0,
}

void loop() {
  if (hour2 == 0)
    write_time;
  read_time();
  draw_time(0x34, num[hour1]);
  draw_time(0x35, num[hour2] ^ 0x80);
  draw_time(0x36, num[minute1] ^ 0x80);
  draw_time(0x37, num[minute2]);
  delay(499);
}

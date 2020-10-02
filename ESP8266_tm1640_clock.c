#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
const char *ssid     = "WiFi";
const char *password = "Password";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
int scl = 4;
int sda = 5;
void i2c_start() {
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
}
void i2c_stop() {
  digitalWrite(scl, 0);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
}
void i2c_ack() {
  int v = digitalRead(sda);
  while (v == 1) {
    delay(10);
    Serial.println("wite the call back");
    v = digitalRead(sda);
  }
  Serial.println("Got the call back");
  pinMode(sda, OUTPUT);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
}
void i2c_Write(uint8_t data) {
  pinMode(sda, OUTPUT);
  int t = data;
  int arr[8];
  for (int i = 0; i <= 7; i++) {
    if (data % 2) {
      arr[i] = 1;
    }
    else
    {
      arr[i] = 0;
    }
    data /= 2;
  }
  //if(dot){arr[0] = 1}else{arr[0] = 0}
  // for ( int j = 7; j >= 0; j--) {
  for ( int j = 0; j <= 7; j++) {
    if (arr[j])
    {
      digitalWrite(sda, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(sda, 0);
    } else {
      delayMicroseconds(2);
      digitalWrite(sda, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(scl, 1);
      delayMicroseconds(2);
      digitalWrite(scl, 0);
      delayMicroseconds(2);
      digitalWrite(sda, 0);
    }
    pinMode(sda, OUTPUT);
  }
}
uint8_t num[10] = {
  //0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7f, 0x6f
};
uint8_t dd[8] = {
  0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7
};
void setup() {
  pinMode(scl, OUTPUT);
  pinMode(sda, OUTPUT);
  WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }
    timeClient.begin();
  Serial.begin(115200);
  delay(2000);
  //Serial_println_s("open the windows");
}
void loop() {
  timeClient.update();
  int sec = timeClient.getSeconds();
  int seg = sec / 8;
  int wei = sec % 8;
  i2c_start();
  i2c_Write(0x44);
  i2c_stop();
  i2c_start();
  i2c_Write(0xc8);
  i2c_Write(num[timeClient.getHours() / 10]);
  i2c_Write(num[timeClient.getHours() % 10]);
  i2c_Write(num[timeClient.getMinutes() / 10]);
  i2c_Write(num[timeClient.getMinutes() % 10]);
  i2c_stop();
  i2c_start();
  i2c_Write(0x88);
  i2c_stop();
  delay(50);
    if (seg < 1){
      i2c_start();
  i2c_Write(0x44);
  i2c_stop();
  i2c_start();
  i2c_Write(dd[seg]);
  i2c_Write(0x01 << wei);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_Write(0x00);
  i2c_stop();
  i2c_start();
  i2c_Write(0x88);
  i2c_stop();
    }else{
  i2c_start();
  i2c_Write(0x44);
  i2c_stop();
  i2c_start();
  i2c_Write(dd[seg - 1]);
  i2c_Write(0x00);
  i2c_Write(0x01 << wei);
  i2c_stop();
  i2c_start();
  i2c_Write(0x88);
  i2c_stop();
  }
  delay(50);
  Serial.println(timeClient.getSeconds());
}

#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
const char *ssid     = "Nexus";
const char *password = "13033883439";
#define LED 13
#define SOD A0
#define S_2 14
#define BUG 1
int sound = 0;
int Line = 305;
int hour;
int L = 1;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000);
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    L = 1 - L;
    digitalWrite(LED, L);
    delay ( 500 );
    Serial.print ( "." );
  }
  digitalWrite(LED, 0);
  timeClient.begin();
  timeClient.update();
  hour = timeClient.getHours();
  Serial.println(timeClient.getFormattedTime());
  WiFi.disconnect(1);                     //时间更新完成后，断开连接，保持低功耗；
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("无线终端和接入点的连接已中断");
  }
  else
  {
    Serial.println("未能成功断开连接！");
  }
}


void loop() {
  if (hour > 18 || hour < 6) {
    sound = analogRead(SOD);
    if (sound > Line) {
      digitalWrite(LED, 1);
      delay(30 * 1000);
      digitalWrite(LED, 0);
    }
    delay(200);
    Serial.println("!!!");
  } else {
    timeClient.update();
    hour = timeClient.getHours();
    for (int i = 0; i < 20; i++) {
      delay(60 * 1000);
      Serial.println(timeClient.getFormattedTime());
    }
    WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED ) {
      L = 1 - L;
      digitalWrite(LED, L);
      delay ( 500 );
      Serial.print ( "." );
    }
    digitalWrite(LED, 0);
    timeClient.begin();
    timeClient.update();
    hour = timeClient.getHours();
    Serial.println(timeClient.getFormattedTime());
    WiFi.disconnect(1);                     //时间更新完成后，断开连接，保持低功耗；
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("无线终端和接入点的连接已中断");
    }
    else
    {
      Serial.println("未能成功断开连接！");
    }
  }
}

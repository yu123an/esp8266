#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AirGradient.h>
const char *ssid     = "8888888";
const char *password = "8888888";
#define LED 13
#define SOD A0
#define S_2 14
#define BUG 1
int sound = 0;
int Line = 305;
int hour;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 8, 60000);
AirGradient ag = AirGradient();
void eror() {
  for (int e = 0; e < 100; e++) {
    digitalWrite(LED, 1);
    delay(2000);
    digitalWrite(LED, 0);
    delay(2000);
  }
}
void setup() {
  pinMode(LED,OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
  timeClient.update();
  hour = timeClient.getHours();
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
    }
  }
}

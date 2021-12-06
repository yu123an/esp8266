#include <Wire.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AirGradient.h>
const char *ssid     = "888888";
const char *password = "888888";
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
void setup() {
  Serial.begin(9600);
  Wire.begin();
  ag.TMP_RH_Init(0x44);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
}


void loop() {
  timeClient.update();
  hour = timeClient.getHours();
  if (hour > 18 || hour < 6) {
    sound = analogRead(SOD);
    if (sound > Line) {
      digitalWrite(LED, 1);
      delay(30 * 1000);
      digitalWrite(LED, 0);
    }
    delay(200);
  } else {
    for (int i = 0; i < 60; i++) {
      delay(60 * 1000);
    }
  }
   TMP_RH result = ag.periodicFetchData();
  Serial.print("Humidity: ");
  Serial.print(result.rh_char);
  Serial.print(" Temperature: ");
  Serial.println(result.t_char);
  delay(5000);
}

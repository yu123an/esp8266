#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
int version = 18;
String ssid = "WiFi" ;         //WiFi名称
String password =  "Pass";   //WiFi密码
int i = 0;
String ota_url = "http://Station.xyz/ESP_OTA/ota_test_" + String(version + 1) + "_.bin";
String ota_chack = "http://Station.xyz/ESP_OTA/ota_chack.php";
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);         //联网
  while ( WiFi.status() != WL_CONNECTED ) {
    i++;
    delay ( 500 );
    Serial.print ( "." );
    if (i > 40) {                    //60秒后如果还是连接不上，就判定为连接超时
      Serial.println("");
      Serial.print("连接超时！请检查网络环境");
      Serial.println("");
    } break;
  }
}
void loop() {
  Serial.print("当今版本：");
  Serial.println(version);
  Serial.println("现在准备检查更新最新版本........");
  ota_update();
  delay(20000);
}


void ota_update() {
  HTTPClient Ota_update;
  Ota_update.begin(ota_chack);
  Ota_update.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String post_data = "Version=" + String(version + 1);
  Serial.println("查询新版固件........");
  Ota_update.POST(post_data);
  String Is_updata = Ota_update.getString();
  if ( Is_updata == "OK") {
    Serial.println("发现新版固件，准备更新........");
    ESPhttpUpdate.update(ota_url);
  } else {
    Serial.println("已是最新固件，继续使用");
  }
}

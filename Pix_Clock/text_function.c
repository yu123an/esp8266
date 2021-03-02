#include <EEPROM.h>
int wifi_name_add = 1;
int wifi_pass_add = 21;
int weather_local_add = 41;
int weather_key_add = 61;
String wifi_name[20];
String wifi_pass[20];
String weather_local[20];
String weather_key;
void write_eeprom(int addr,String velue){
  int lenth = velue.length();
  for(int a = 0;a<lenth;a++){
    EEPROM.write(addr + a,velue[a]);
    //delay(50);
  }
  EEPROM.commit();
}
String read_eeprom(int addr,int lenth){
  String Text;
  for(int a = 0;a<lenth;a++){
   Text +=  char(EEPROM.read(addr + a));
   //delay(50);
   
  }
  return Text ;
  Serial.print(Text);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  EEPROM.begin(512);
}
void loop() {
 // write_eeprom(weather_key_add , "b5845***************adb6c");
  //delay(200);
  Serial.println (" data has been written !!!!!");
  weather_key = read_eeprom(weather_key_add ,32);
  Serial.print("now begin to read the key : ");
  Serial.println(weather_key);
  
  delay(2000);
}

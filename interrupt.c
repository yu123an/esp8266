uint8_t GPIO_Pin = D2;
void ICACHE_RAM_ATTR IntCallback();
void setup() {
 Serial.begin(115200);
 pinMode(GPIO_Pin,INPUT);
 attachInterrupt(GPIO_Pin, IntCallback, FALLING);
}

void loop() {
  Serial.println("aaa");
  delay(2000);
}

void IntCallback(){
  Serial.println("all");
}

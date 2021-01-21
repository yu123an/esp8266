void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("----------------------------------------");
  int b = Serial.available();
  Serial.println("I am ok !!! ");
  if (Serial.available() > 0) {
    Serial.print(b);
    Serial.println(" int I get !!!");
    for (int i = 0; i < b; i++) {
      int a = Serial.read();
      Serial.print(a, DEC);
    }
    Serial.println("  ;read over!!!");
  }
  else {
    Serial.println ("I got Nothing ???");
  }
  delay(2000);
}

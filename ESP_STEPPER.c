#include <Stepper.h>
#define STEPS 64
Stepper stepper(STEPS,16,14,12,13);
void setup() {
  // put your setup code here, to run once:
stepper.setSpeed(200);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
stepper.step(2048);
delay(2000);
}

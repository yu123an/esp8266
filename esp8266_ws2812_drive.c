#include <SPI.h>
void write_byte(uint8_t num) {
  for (int i = 0; i <= 7; i++) {
    if (num & 0x80) {
      SPI.transfer(0x7c);
    } else {
      SPI.transfer(0x70);
    }
    num <<= 1;
  }
}
void write_led(uint8_t numa, uint8_t numb, uint8_t numc) {
  write_byte(numa);
  write_byte(numb);
  write_byte(numc);
}
void write_num(uint8_t data) {
  for (int i = 0; i <= 6; i++) {
    if (data<<1 & 0x80) {
      for (int j = 0; j <= 3; j++) {
        write_led(0, 0, 100);
      }
    } else {
      for (int j = 0; j <= 3; j++) {
        write_led(0, 0, 0);
      }
    }
    data <<= 1;
  }
}
void setup() {
SPI.begin();
SPI.setFrequency(8000000);  //8MHzSPI
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int n = 1; n <= 64; n++) {
for (int j = 0; j <= n; j++) {
        write_led(0, 0, 10);
      }
      delay(200);
}
  for (int n = 1; n <= 64; n++) {
for (int j = 0; j <= n; j++) {
        write_led(0, 10, 0);
      }
      delay(200);
}
  for (int n = 1; n <= 64; n++) {
for (int j = 0; j <= n; j++) {
        write_led(10, 0, 0);
      }
      delay(200);
}
}

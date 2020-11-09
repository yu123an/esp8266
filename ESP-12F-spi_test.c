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
void setup() {
    SPI.begin();
    SPI.setFrequency(8000000);  //8MHzSPI
}
void loop() {
  for(int i = 0;i<=320;i++){
    for(int j = 0;j<=i;j++){
      write_led(50, 0, 0);
    }
    delay(50);
  }
  for(int i = 0;i<=320;i++){
    for(int j = 0;j<=i;j++){
      write_led(0, 50, 0);
    }
    delay(50);
  }
  for(int i = 0;i<=320;i++){
    for(int j = 0;j<=i;j++){
      write_led( 0, 0, 50);
    }
    delay(50);
  }
}

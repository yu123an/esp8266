#include <SPI.h>
#define ADC_R PC4
#define ADC_G PD2
#define ADC_B PD3
void write_byte(uint8_t num) {
  for (int i = 0; i <= 7; i++) {
    if (num & 0x80) {
      SPI_transfer(0x7c);
    } else {
      SPI_transfer(0x70);
    }
    num <<= 1;
  }
}
void write_led(uint8_t numa, uint8_t numb, uint8_t numc) {
  write_byte(numa);
  write_byte(numb);
  write_byte(numc);
}
void write_num(uint8_t R,uint8_t G,uint8_t B ) {
  for (int i = 0; i < 12; i++) {
        write_led(G, R, B);
      }
}
void setup() {
  // put your setup code here, to run once:
Serial_begin(9600);
SPI_begin();
}

void loop() {
  // put your main code here, to run repeatedly:
 int ADC_RR = analogRead(ADC_R) / 2;
 int ADC_GG = analogRead(ADC_G) / 2;
 int ADC_BB = analogRead(ADC_B) / 2;
 /*Serial_print_s( "Color R is ");
 Serial_print_i(ADC_RR);
 Serial_print_s( " V ; Color G is ");
 Serial_print_i(ADC_GG);
 Serial_print_s( " V ; Color B is ");
 Serial_print_i(ADC_BB);
 Serial_println_s( " V ; ");*/
 write_num(ADC_RR,ADC_GG,ADC_BB);
//write_num(25,25,25);
delay(20);
}

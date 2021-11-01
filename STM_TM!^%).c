#define scl PB4
#define sda PC3
uint8_t number[] = {
  //0   1     2     3     4     5     6     7     8     9
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
  0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF
};
void i2c_start() {
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
}
void i2c_stop() {
  digitalWrite(scl, 0);
  delayMicroseconds(2);
  digitalWrite(sda, 0);
  delayMicroseconds(2);
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
}
void i2c_ack() {
  int v = digitalRead(sda);
  while (v == 1) {
    delay(10);
    Serial_println_s("wite the call back");
    v = digitalRead(sda);
  }
  Serial_println_s("Got the call back");
  pinMode(sda, OUTPUT);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
  digitalWrite(sda, 1);
  delayMicroseconds(2);
  digitalWrite(scl, 1);
  delayMicroseconds(2);
  digitalWrite(scl, 0);
  delayMicroseconds(2);
}
void i2c_write(uint8_t data) {
  pinMode(sda, OUTPUT);
  for ( int j = 7; j >= 0; j--) {
      digitalWrite(sda, (data >> j) & 0x01);
      delayMicroseconds(1);
      digitalWrite(scl, 0);
      delayMicroseconds(1);
      digitalWrite(scl, 1);
      delayMicroseconds(1);
      digitalWrite(scl, 0);
      delayMicroseconds(1);
      digitalWrite(sda, 0);
    }
   
}
void setup() {
  pinMode(scl, OUTPUT);
  pinMode(sda, OUTPUT);
  Serial_begin(9600);
}

void loop() {
  for (int i = 0; i < 20; i++) {
    i2c_start();
    i2c_write(0x48);
    i2c_ack();
    i2c_write(0x41);
    i2c_ack();
    i2c_stop();
    i2c_start();
    i2c_write(0x68);
    i2c_ack();
    i2c_write(number[i]);
    i2c_ack();
    i2c_stop();
    i2c_start();
    i2c_write(0x6a);
    i2c_ack();
    i2c_write(number[i]);
    i2c_ack();
    i2c_stop();
    i2c_start();
    i2c_write(0x6c);
    i2c_ack();
    i2c_write(number[i]);
    i2c_ack();
    i2c_stop();
    i2c_start();
    i2c_write(0x6e);
    i2c_ack();
    i2c_write(number[i]);
    i2c_ack();
    i2c_stop();
    delay(200);
  }
  delay(2000);
}

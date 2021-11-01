#define _pin_ena  PA1/*DS1302*/
#define _pin_clk  PC6
#define _pin_dat  PA2
#define scl PB4
#define sda PC3
#define BUZ PD4
#define REG_BURST             0xBE
#define REG_WP                0x8E
uint8_t second, minute, hour, day, mouth, dow, year;
int debug = 1;
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
     delayMicroseconds(1);
    // Serial_println_s("wite the call back");
    v = digitalRead(sda);
  }
  //Serial_println_s("Got the call back");
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
/*
  DS1302驱动库
*/
void _nextBit()
{
  digitalWrite(_pin_clk, HIGH);
  delayMicroseconds(1);

  digitalWrite(_pin_clk, LOW);
  delayMicroseconds(1);
}
uint8_t _readByte()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    if (digitalRead(_pin_dat) == HIGH) byte |= 0x01 << b;
    _nextBit();
  }

  return byte;
}
void _writeByte(uint8_t value)
{
  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(_pin_dat, (value & 0x01) ? HIGH : LOW);
    _nextBit();
    value >>= 1;
  }
}
void _setDirection(int direction) {
  pinMode(_pin_dat, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000001 | address;
  _writeByte(command);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(_pin_ena, HIGH);
  uint8_t command = 0b10000000 | address;
  _writeByte(command);
}
void _end()
{
  digitalWrite(_pin_ena, LOW);
}

uint8_t _dec2bcd(uint8_t dec)
{
  return ((dec / 10 * 16) + (dec % 10));
}

uint8_t _bcd2dec(uint8_t bcd)
{
  return ((bcd / 16 * 10) + (bcd % 16));
}
void get_time() {
  _prepareRead(REG_BURST);
  second = _bcd2dec(_readByte() & 0b01111111);
  minute = _bcd2dec(_readByte() & 0b01111111);
  hour   = _bcd2dec(_readByte() & 0b00111111);
  day    = _bcd2dec(_readByte() & 0b00111111);
  mouth  = _bcd2dec(_readByte() & 0b00011111);
  dow    = _bcd2dec(_readByte() & 0b00000111);
  year   = _bcd2dec(_readByte() & 0b01111111);
  _end();
  if (debug) {
    Serial_print_s("Now time is ");
    Serial_print_i(year);
    Serial_print_s("-");
    Serial_print_i(mouth);
    Serial_print_s("-");
    Serial_print_i(day);
    Serial_print_s("-");
    Serial_print_i(hour);
    Serial_print_s(":");
    Serial_print_i(minute);
    Serial_print_s(":");
    Serial_println_i(second);
  }
}
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  _writeByte(_dec2bcd(sec % 60 ));
  _writeByte(_dec2bcd(minute % 60 ));
  _writeByte(_dec2bcd(hour   % 24 ));
  _writeByte(_dec2bcd(day    % 32 ));
  _writeByte(_dec2bcd(mouth  % 13 ));
  _writeByte(_dec2bcd(dow    % 8  ));
  _writeByte(_dec2bcd(year   % 100));
  _writeByte(0b10000000);
  _end();
}
void set_time_mini(int sec, int minute, int hour) {
  _prepareWrite(REG_WP);
  _writeByte(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  _writeByte(_dec2bcd(sec % 60 ));
  _writeByte(_dec2bcd(minute % 60 ));
  _writeByte(_dec2bcd(hour   % 24 ));
  _writeByte(0b10000000);
  _end();
}
void dot() {
  for (int i = 0; i < 20; i++) {
    digitalWrite(BUZ, 1);
    delay(2);
    digitalWrite(BUZ, 0);
    delay(2);
  }
  //delay(1000);
}
void dis_num(uint8_t num1, uint8_t num2, uint8_t num3, uint8_t num4) {
  i2c_start();
  i2c_write(0x48);
  i2c_ack();
  i2c_write(0x41);
  i2c_ack();
  i2c_stop();
  i2c_start();
  i2c_write(0x68);
  i2c_ack();
  i2c_write(num1);
  i2c_ack();
  i2c_stop();
  i2c_start();
  i2c_write(0x6a);
  i2c_ack();
  i2c_write(num2);
  i2c_ack();
  i2c_stop();
  i2c_start();
  i2c_write(0x6c);
  i2c_ack();
  i2c_write(num3);
  i2c_ack();
  i2c_stop();
  i2c_start();
  i2c_write(0x6e);
  i2c_ack();
  i2c_write(num4);
  i2c_ack();
  i2c_stop();
}
void PBB(){
  dot();
  dis_num(0xff,0,0x0f,0xf0);
  delayMicroseconds(50*1000);
}
void setup() {
  Serial_begin(9600);
  pinMode(BUZ, OUTPUT);
  pinMode(_pin_ena, OUTPUT);
  pinMode(_pin_clk, OUTPUT);
  pinMode(_pin_dat, INPUT);
  digitalWrite(_pin_ena, LOW);
  digitalWrite(_pin_clk, LOW);
  pinMode(scl, OUTPUT);
  pinMode(sda, OUTPUT);
  /*
     STM INTERRUPT SETTING
  */
  //PA3
  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOA, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTA & 0xff, PBB, 0);
  //PB5
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOB, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTB & 0xff, PBB, 0);
   //PC7
  GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOC, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTC & 0xff, PBB, 0);
  //PD2
  GPIO_Init(GPIOD, GPIO_PIN_2, GPIO_MODE_IN_FL_IT);
  disableInterrupts();
  EXTI_SetExtIntSensitivity( EXTI_PORT_GPIOD, FALLING);
  enableInterrupts();
  attachInterrupt(INT_PORTD & 0xff, PBB, 0);
}
void loop() {
  get_time();
  //dot();
  dis_num(number[minute / 10], number[minute % 10], number[second / 10], number[second % 10]);
  delay(998);
}

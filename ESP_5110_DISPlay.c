#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//宏定义
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
//引脚声明
#define DIS_CLK 0 
#define DIS_SDA 2 
#define DIS_DC 4 
#define DIS_CE 5 
#define DIS_RST 16 
#define RTC_SDA 2 
#define RTC_SCL 0 
#define RTC_CE 5 
#define BUZ 15 
#define KEY_1 14 
#define KEY_2 13 
#define KEY_3 12 
#define debug 1
#define REG_BURST 0xBE
#define REG_WP 0x8E
uint8_t second, minute, hour, day, mouth, dow, year;
Adafruit_PCD8544 display = Adafruit_PCD8544(DIS_CLK, DIS_SDA, DIS_DC, DIS_CE, DIS_RST);
void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    //if ((i > 0) && (i % 14 == 0))
      //display.println();
  }    
  display.display();
}

//以下为DS1302操作函数
void Write_data(uint8_t data)
{
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(RTC_SCL, LOW);
    delayMicroseconds(2);
    digitalWrite(RTC_SDA, (data & 0x01) ? HIGH : LOW);
    digitalWrite(RTC_SCL, HIGH);
    delayMicroseconds(2);
    data >>= 1;
  }
}
uint8_t Read_data()
{
  uint8_t byte = 0;

  for (uint8_t b = 0; b < 8; b++)
  {
    digitalWrite(RTC_SCL, LOW);
    delayMicroseconds(2);
    if (digitalRead(RTC_SDA) == HIGH)
      byte |= 0x01 << b;
    digitalWrite(RTC_SCL, HIGH);
    delayMicroseconds(2);
  }
  return byte;
}
void _setDirection(int direction)
{
  pinMode(RTC_SDA, direction);
}
void _prepareRead(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(RTC_CE , HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000001 | address;
  Write_data(command);
  _setDirection(INPUT);
}
void _prepareWrite(uint8_t address)
{
  _setDirection(OUTPUT);
  digitalWrite(RTC_CE , HIGH);
  //digitalWrite(cs_HT, HIGH);
  uint8_t command = 0b10000000 | address;
  Write_data(command);
}
void _end()
{
  digitalWrite(RTC_CE , LOW);
  pinMode(RTC_SDA, OUTPUT);
}
uint8_t _dec2bcd(uint8_t dec)
{
  return ((dec / 10 * 16) + (dec % 10));
}
uint8_t _bcd2dec(uint8_t bcd)
{
  return ((bcd / 16 * 10) + (bcd % 16));
}
void get_time()
{
  _prepareRead(REG_BURST);
  second = _bcd2dec(Read_data() & 0b01111111);
  minute = _bcd2dec(Read_data() & 0b01111111);
  hour = _bcd2dec(Read_data() & 0b00111111);
  day = _bcd2dec(Read_data() & 0b00111111);
  mouth = _bcd2dec(Read_data() & 0b00011111);
  dow = _bcd2dec(Read_data() & 0b00000111);
  year = _bcd2dec(Read_data() & 0b01111111);
  _end();
  if (debug)
  {
    Serial.print("Now time is ");
    Serial.print(year);
    Serial.print("-");
    Serial.print(mouth);
    Serial.print("-");
    Serial.print(day);
    Serial.print("-");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
  }
}
void set_time(int sec, int minute, int hour, int day, int mouth, int dow, int year)
{
  _prepareWrite(REG_WP);
  Write_data(0b00000000);
  _end();
  _prepareWrite(REG_BURST);
  Write_data(_dec2bcd(sec % 60));
  Write_data(_dec2bcd(minute % 60));
  Write_data(_dec2bcd(hour % 24));
  Write_data(_dec2bcd(day % 32));
  Write_data(_dec2bcd(mouth % 13));
  Write_data(_dec2bcd(dow % 8));
  Write_data(_dec2bcd(year % 100));
  Write_data(0b10000000);
  _end();
}
void DS1302_INT()
{
  pinMode(RTC_CE , OUTPUT);
  pinMode(RTC_SCL, OUTPUT);
  pinMode(RTC_SDA, INPUT);
  digitalWrite(RTC_CE , LOW);
  digitalWrite(RTC_SCL, LOW);
}

void buzzer()
{
  for (int t = 0; t <2; t++)
  {
    digitalWrite(BUZ, 1);
     for(int i = 0;i<10;i++){
      delayMicroseconds(1000);
    }
    digitalWrite(BUZ, 0);
    for(int i = 0;i<30;i++){
      delayMicroseconds(1000);
    }
  }
}
ICACHE_RAM_ATTR void KEY_O()
{
  Serial.println("The button Key_1 has been press !!!");
  buzzer();
}
ICACHE_RAM_ATTR void KEY_T()
{
  for(int i = 0;i<100;i++){
      delayMicroseconds(1000);
    }
  Serial.println("The button Key_2 has been press !!!");
}
ICACHE_RAM_ATTR void KEY_TH()
{
  for(int i = 0;i<100;i++){
      delayMicroseconds(1000);
    }
  Serial.println("The button Key_3 has been press !!!");
}
void testdrawline() {  
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, BLACK);
    display.display();
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, BLACK);
    display.display();
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, BLACK);
    display.display();
  }
  for (int8_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, BLACK);
    display.display();
  }
  delay(250);
  
  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, BLACK);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, BLACK);
    display.display();
  }
  delay(250);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, BLACK);
    display.display();
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, BLACK); 
    display.display();
  }
  delay(250);
}
void setup()
{
  display.begin();
  DS1302_INT();
  Serial.begin(9600);
  pinMode(KEY_1, INPUT_PULLUP);
  pinMode(KEY_2, INPUT_PULLUP);
  pinMode(KEY_3, INPUT_PULLUP);
  pinMode(BUZ, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(KEY_1), KEY_O, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEY_2), KEY_T, FALLING);
  attachInterrupt(digitalPinToInterrupt(KEY_3), KEY_TH, FALLING);
}
void loop()
{
  display.setContrast(75);

  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer

  // draw a single pixel
  display.drawPixel(10, 10, BLACK);
  display.display();
  delay(2000);
  display.clearDisplay();

  // draw many lines
  testdrawline();
  display.display();
  delay(2000);
  display.clearDisplay();
  get_time();
}

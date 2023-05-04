// 注释区
/*
  {
  屏幕引脚使用，复制到C:\Users\aa\Documents\Arduino\libraries\TFT_eSPI/User_Setup.h
  #define TFT_MOSI  11  // In some display driver board, it might be written as "SDA" and so on.
  #define TFT_SCLK  12
  #define TFT_CS  47   // Chip select control pin
  #define TFT_DC  14   // Data Command control pin
  #define TFT_RST   9  // Reset pin (could connect to Arduino RESET pin)
  #define TFT_BL     21  // LED back-light
  #define TFT_BACKLIGHT_ON HIGH
  }
  {
  功能列表：
  蜂鸣器----aaa
  充放电----aaa
  电量检测----aaa
  DS1307时钟----aaa
  旋转编码器----aaa
  触摸支持----aaa
  板载SD卡----aaa
  SHT30温湿度----aaa
  板载EEPROM----aaa
  板载扬声器---aaa
  板载麦克风---aaa
  }
  {
   引脚使用情况：
  IIC:
  sda:5,SCL:4
  ENcoder:
  A:6,B:15,S:7
  麦克风：
  WS:16;SCK:17,SD:18
  BAT:8
  Touch:19
  Touch_CS:20
  SD_CS:10
  BUZZER:48
  MIC_EN:1
  扬声器：
  使能：2;检测：41;DO:40;BAK:39;WS:38
  充电检测:42
  }
*/
//加载库
#include "msg.h"         //敏感信息
#include <NTPClient.h>   //网络校时
#include <HTTPClient.h>  //http请求
#include "TFT_eSPI.h"    //屏幕驱动
#include <RtcDS1307.h>
#include <Wire.h>     //DS1307驱动
#include <WiFiUdp.h>  //wifi支持
#include <WiFi.h>
#include <ArduinoJson.h>           //JSON解析
#include <WiFiClientSecure.h>      //https请求
#include "OneButton.h"             //按钮启用
#include <Ticker.h>                //定时任务
#include "ClosedCube_SHT31D.h"     //SHT30
#include "AiEsp32RotaryEncoder.h"  //旋转编码器
#include <JPEGDecoder.h>           //加载jpg图片
#include <EEPROM.h>                // 片上EEPROM
#include "FS.h"                    // SD卡读写
#include "SD.h"
#include "SPI.h"
//特殊引脚
#define sdSelectPin 10
#define IIC_SDA 5
#define IIC_SCL 4
#define BLPin 21
#define Encoder_A 15
#define Encoder_B 6
#define Encoder_S 7
#define RE_VCC_PIN -1
#define RE_STEPS 4
#define Buzzer 48
#define Battery 8
#define Charge 42
#define Touch_Interrupt 19
#define Touch_CS 20
#define MIC_EN 1
#define MIC_WS 16
#define MIC_SCK 17
#define MIC_SD 18
#define I2S_EN 2
#define I2S_IS 41
#define I2S_DO 40
#define I2S_BAK 39
#define I2S_WS 38
//颜色列表
#define c_BL 0xFFFF
#define c_Line tft.color24to16(0x426666)
#define c_text tft.color24to16(0x003371)
#define c_time tft.color24to16(0x333631)

//字体列表
#define Digi &DS_DIGI32pt7b          // 数码管字体
#define DejaVu &DejaVu_Sans_Mono_20  // 等宽字体
#define PAPL &PAPL_125pt7b           // 等宽数字
// String fontname = "siyuan20";  //思源宋体
// String fontname = "fangzheng20";  //方正幼黑
// String fontname = "sisong20";  //思源宋体
String fontname = "FZFWZhuZiAYuanJWR20";  // 方正细金陵简体

//变量列表
String temp, hump, windDir, wind, _weather, _date; // 天气状况
int Temp_in, Humidity_in;                          // 室内温湿度
int _day, _hour, _minute, _second;                 // 时间更新
int update_flag = 0;                               // 更新标志位
int Light = 127;                                   // 亮度控制
int encoder = 127;                                 // 编码器
float v_bat = 0;                                   // 电量检测
String JsonMsg;                                    // json串解析
#define MSG_BUFFER_SIZE (2000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<2000> Mqtt_Sub;
unsigned long lastMsg = 0;

//实例化类
TFT_eSPI tft = TFT_eSPI();             // TFT显示
TFT_eSprite Stime = TFT_eSprite(&tft); // 添加刷新缓存
TFT_eSprite ShiCi = TFT_eSprite(&tft); // 添加刷新缓存
WiFiUDP ntpUDP;
WiFiClientSecure espClient;     // WiFi
NTPClient timeClient(ntpUDP, "ntp.ntsc.ac.cn", 8 * 3600, 60000);
RtcDS1307<TwoWire> Rtc(Wire);                                                                                       // DS1302
ClosedCube_SHT31D sht3xd;                                                                                           // SHT30
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(Encoder_A, Encoder_B, Encoder_S, RE_VCC_PIN, RE_STEPS); // 编码器
Ticker uptime;                                                                                                      // 更新时间
Ticker ReadEncoder;
//功能函数---编码器
void rotary_onButtonClick()
{
  static unsigned long lastTimePressed = 0;
  // ignore multiple press in that time milliseconds
  if (millis() - lastTimePressed < 500)
  {
    return;
  }
  lastTimePressed = millis();
  Serial.print("button pressed ");
  Serial.print(millis());
  Serial.println(" milliseconds after restart");
}
void rotary_loop()
{
  // dont print anything unless value changed
  if (rotaryEncoder.encoderChanged())
  {
    Serial.print("Value: ");
    Serial.println(rotaryEncoder.readEncoder());
    encoder = rotaryEncoder.readEncoder();
    analogWrite(BLPin, encoder);
    buzzer();
  }
  if (rotaryEncoder.isEncoderButtonClicked())
  {
    rotary_onButtonClick();
  }


}
void IRAM_ATTR readEncoderISR()
{
  rotaryEncoder.readEncoder_ISR();
}
// 功能函数---传感器
//TF卡使能
void sd_en()
{
  if (!SD.begin(sdSelectPin))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
}
// 更新时间
void time_update()
{
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  _second = timeClient.getSeconds();
  if (_minute != 0) {
    Serial.println("成功获取时间，准备更新时间。。。。");
    Rtc._SetDateTime(_second, _minute, _hour);
  }
}
// 获取SHT30温湿度
void get_sht30(String text, SHT31D result)
{
  if (result.error == SHT3XD_NO_ERROR)
  {
    Temp_in = result.t;
    Humidity_in = result.rh;
  }
  else
  {
    Serial.print(text);
    Serial.print(": [ERROR] Code #");
    Serial.println(result.error);
  }
}
// 蜂鸣器
void buzzer()
{
  ledcAttachPin(Buzzer, 0); // The piezo speaker is attached to GPIO26
  ledcWriteTone(0, 1000);
  delay(150);
  ledcDetachPin(Buzzer);
  ledcWrite(0, 0);
}
// 获取电量
void get_bat()
{
  double _bat = 0;
  for (int i = 0; i < 8; i++)
  {
    _bat += analogRead(Battery);
  }
  //_bat > 3;
  v_bat = _bat * 2 / 4096 / 8;
}
//功能函数---功能类
// 获取古诗词
void drawShici()
{
  get_net(web_sc, 1);
  // ShiCi.createSprite(280, 70);
  // ShiCi.fillScreen(c_BL);
  // //sd_en();
  // ShiCi.loadFont("sisong20", SD); // 加载字体
  // ShiCi.setCursor(5, 5);
  // ShiCi.setTextColor(c_text);
  // ShiCi.print(JsonMsg);
  // ShiCi.pushSprite(10, 250);
  // ShiCi.unloadFont();
  // ShiCi.deleteSprite();
  Stime.createSprite(280, 70);
  Stime.fillScreen(c_BL);
  Stime.loadFont("sisong20", SD); // 加载字体
  Stime.setCursor(0, 5);
  Stime.setTextColor(c_text);
  Stime.print(Mqtt_Sub["content"].as<String>());
  Stime.pushSprite(10, 250);
  Stime.unloadFont();
  Stime.deleteSprite();
  //SD.end();
}
// 绘制主屏幕
void drawHomePage()
{
  sd_en();
  get_sht30("Periodic Mode", sht3xd.periodicFetchData());
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  get_net(web_hf, 1);
  _date = Mqtt_Sub["now"]["obsTime"].as<String>().substring(5, 10);
  String _Day = Mqtt_Sub["now"]["obsTime"].as<String>().substring(8, 10);
  String _icon = Mqtt_Sub["now"]["icon"].as<String>();
  temp = Mqtt_Sub["now"]["temp"].as<String>() + "°C ";
  hump = Mqtt_Sub["now"]["humidity"].as<String>() + "% ";
  get_net(classlist, 1);
  _day = timeClient.getDay();
  String CL = Mqtt_Sub["Single"][_day]["no"].as<String>();
  get_bat();
  tft.fillScreen(c_BL);
  tft.fillRect(0, 0, 480, 30, c_text); // 顶部栏
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  // 绘制顶部栏信息
  tft.drawString(_date, 12, 5);
  tft.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) + ":" + String(now.Second() / 10) + String(now.Second() % 10), 188, 5);
  tft.drawString(String(v_bat), 416, 5);
  tft.setTextColor(c_text);
  // 绘制课程表
  tft.drawLine(12, 55, 468, 55, c_Line);
  tft.drawString(_Day + " A1 A2 A3 A4 B1 B2 B3 B4 C1 C2 C3", 12, 35);
  tft.drawString(CL, 12, 57);
  for (int i = 0; i < 12; i++)//此处有改动11->12
  {
    tft.drawLine(26 + 12 + 6 + 39 * i, 35, 26 + 12 + 6 + 39 * i, 70, c_Line);
  }

  drawSdJpeg(("/64/" + _icon + ".png.jpg").c_str(), 288, 256);
  tft.drawString(String(Temp_in) + "/" + temp, 354, 258);
  tft.drawString(String(Humidity_in) + "/" + hump, 354, 290);
  drawShici();
 // drawToDo();
 SD.end();
}
// http请求
void get_net(String web, bool isdecode)
{
  HTTPClient http;
  if (http.begin(web))
  {
    Serial.println("HTTPclient setUp done!");
  }
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      if (isdecode)
      {
        deserializeJson(Mqtt_Sub, payload);
      }
      else
      {
        JsonMsg = payload;
      }
    }
  }
  else
  {

    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
// http字符串解析
void JsonEecoed(String json)
{
  deserializeJson(Mqtt_Sub, json);
}
void drawTime()
{
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  // 绘制顶部时间栏
  RtcDateTime now = Rtc.GetDateTime();
  Stime.createSprite(104, 25);
  Stime.setFreeFont(DejaVu);
  Stime.fillScreen(c_text);
  Stime.setTextColor(c_BL);
  Stime.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) + ":" + String(now.Second() / 10) + String(now.Second() % 10), 0, 0);
  Stime.pushSprite(188, 5);
  Stime.deleteSprite();
  // 更新温湿度
  get_sht30("Periodic Mode", sht3xd.periodicFetchData());
  Stime.createSprite(126, 62);
  Stime.fillScreen(c_BL);
  Stime.setFreeFont(DejaVu);
  Stime.setTextColor(c_text);
  Stime.drawString(String(Temp_in) + "/" + temp, 0, 0);
  Stime.drawString(String(Humidity_in) + "/" + hump, 0, 32);
  Stime.pushSprite(354, 258);
  Stime.deleteSprite();
  // 绘制屏幕时间栏
  Stime.createSprite(236, 50);
  Stime.setFreeFont(Digi);
  Stime.fillScreen(c_BL);
  Stime.setTextColor(c_text);
  Stime.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) + ":" + String(now.Second() / 10) + String(now.Second() % 10), 0, 0);
  Stime.pushSprite(12, 80);
  Stime.deleteSprite();
}
void update_flag_change()
{
  update_flag = 1;
}
void drawToDo(){
 get_net(todolist, 1);
 int totle = Mqtt_Sub["num"].as<int>();
  ShiCi.createSprite(192,176);
  ShiCi.fillScreen(c_BL);
  ShiCi.loadFont("sisong20",SD);
  ShiCi.setTextColor(c_text);
  ShiCi.setCursor(0,5);
  ShiCi.print("今日待办：");
  ShiCi.pushSprite(288,80);
  ShiCi.unloadFont();
  ShiCi.deleteSprite();
}
void _drawToDo()
{
  //sd_en();
  get_net(todolist, 1);
  int totle = Mqtt_Sub["num"].as<int>();
  tft.loadFont("siyuan20", SD); // 加载字体
  tft.setCursor(288, 85); 
  tft.fillRect(288, 80, 192, 176, c_BL);
  tft.setTextColor(c_text);
  tft.print("今日待办：");
  for (int i = 0; i < totle; i++)
  {
    tft.setCursor(288, 115 + i * 30);
    tft.print(String(i + 1) + "." + Mqtt_Sub["list"][i]["no"].as<String>());
  }
  tft.unloadFont();
  //SD.end();
}
// JPG 绘制
void drawSdJpeg(const char *filename, int xpos, int ypos)
{

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open(filename, FILE_READ); // or, file handle reference for SD library

  if (!jpegFile)
  {
    Serial.print("ERROR: File \"");
    Serial.print(filename);
    Serial.println("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: ");
  Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile); // Pass the SD file handle to the decoder,
  // bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded)
  {
    // print information about the image to the serial port
    jpegInfo();
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else
  {
    Serial.println("Jpeg file format not supported!");
  }
}
void jpegRender(int xpos, int ypos)
{

  // jpegInfo(); // Print information from the JPEG file (could comment this line out)

  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  bool swapBytes = tft.getSwapBytes();
  tft.setSwapBytes(true);

  // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
  // Typically these MCUs are 16x16 pixel blocks
  // Determine the width and height of the right and bottom edge image blocks
  uint32_t min_w = jpg_min(mcu_w, max_x % mcu_w);
  uint32_t min_h = jpg_min(mcu_h, max_y % mcu_h);

  // save the current image block size
  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  // record the current time so we can measure how long it takes to draw an image
  uint32_t drawTime = millis();

  // save the coordinate of the right and bottom edges to assist image cropping
  // to the screen size
  max_x += xpos;
  max_y += ypos;

  // Fetch data from the file, decode and display
  while (JpegDec.read())
  { // While there is more data in the file
    pImg = JpegDec.pImage; // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x)
      win_w = mcu_w;
    else
      win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y)
      win_h = mcu_h;
    else
      win_h = min_h;

    // copy pixels into a contiguous block
    if (win_w != mcu_w)
    {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++)
      {
        p += mcu_w;
        for (int w = 0; w < win_w; w++)
        {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    // calculate how many pixels must be drawn
    uint32_t mcu_pixels = win_w * win_h;
    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ((mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);
}
void jpegInfo()
{
  Serial.println("JPEG image info");
  Serial.println("===============");
  Serial.print("Width      :");
  Serial.println(JpegDec.width);
  Serial.print("Height     :");
  Serial.println(JpegDec.height);
  Serial.print("Components :");
  Serial.println(JpegDec.comps);
  Serial.print("MCU / row  :");
  Serial.println(JpegDec.MCUSPerRow);
  Serial.print("MCU / col  :");
  Serial.println(JpegDec.MCUSPerCol);
  Serial.print("Scan type  :");
  Serial.println(JpegDec.scanType);
  Serial.print("MCU width  :");
  Serial.println(JpegDec.MCUWidth);
  Serial.print("MCU height :");
  Serial.println(JpegDec.MCUHeight);
  Serial.println("===============");
  Serial.println("");
}
void setup() {
  // pinMode(I2S_EN, OUTPUT);
  // pinMode(I2S_EN, 0);
  // pinMode(I2S_DO,OUTPUT);
  // pinMode(I2S_DO,0);
  // pinMode(I2S_BAK,OUTPUT);
  // pinMode(I2S_BAK,0);
  // pinMode(I2S_WS,OUTPUT);
  // pinMode(I2S_WS,0);
  //disableCore0WDT();//应对供电不足的问题
  Serial.begin(115200);
  ledcSetup(0, 500, 8);
  sd_en(); // SD使能
  Wire.begin(IIC_SDA, IIC_SCL); //IIC初始化
  // LCD初始化
  tft.begin();
  analogWrite(BLPin, 128);
  tft.setRotation(1);
  tft.fillScreen(c_BL);
  WiFi.begin(ssidhome, passhome); // 连接网络
  Serial.print("Connecting to ");
  Serial.print(ssidhome);
  while (WiFi.status() != WL_CONNECTED)
  { // 等待连接
    delay(500);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  // get_net();
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  // 时间更新
  Rtc.Begin(); // DS1307时间读写
  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  time_update();
  // 编码器
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  bool circleValues = false;
  rotaryEncoder.setBoundaries(0, 240, circleValues);
  rotaryEncoder.setEncoderValue(128);
  // rotaryEncoder.setAcceleration(250);
  ReadEncoder.attach_ms(50, rotary_loop);
  sht3xd.begin(0x44);
  if (sht3xd.periodicStart(SHT3XD_REPEATABILITY_HIGH, SHT3XD_FREQUENCY_10HZ) != SHT3XD_NO_ERROR)
    Serial.println("[ERROR] Cannot start periodic mode");
  drawHomePage();
  uptime.attach(600, update_flag_change);
  //upTouch.attach_ms(50, GetTouch);
}

void loop() {
  drawTime();
  if (update_flag)
  {
    drawHomePage();
    update_flag = 0;
  }
  delay(990);
}

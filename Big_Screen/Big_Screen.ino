//加载库
#include "msg.h"        //敏感信息
#include <NTPClient.h>  //网络校时
#include <HTTPClient.h> //http请求
#include "SPI.h"
#include "TFT_eSPI.h" //屏幕驱动
//#include "siyuan_20.h"//中文字库
#include <RtcDS1307.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <ArduinoJson.h>      //JSON解析
#include <WiFiClientSecure.h> //https请求
#include "OneButton.h"        //按钮启用
#include <Ticker.h>           //定时任务
#include <JPEGDecoder.h>//加载jpg图片
// SD卡读写
#include "FS.h"
#include "SD.h"
#include "SPI.h"
//定义颜色
#define c_BL 0xFFFF
#define c_Line tft.color24to16(0x426666)
#define c_text tft.color24to16(0x003371)
#define c_time tft.color24to16(0x333631)
//定义字体
#define Digi &DS_DIGI32pt7b         // 数码管字体
#define DejaVu &DejaVu_Sans_Mono_20 //等宽字体
#define PAPL &PAPL_125pt7b //等宽数字
String shici;                       //古诗词api返回诗句
String weak = "Double" /*Double or Single*/;
String temp, hump, windDir, wind, _weather, _date;
int _day, _hour, _minute, _second;
// 特殊引脚
#define WeakFlag 13//周选择
#define sdSelectPin 25//sd卡
#define SCL 22
#define SDA 21
// json串解析
#define MSG_BUFFER_SIZE (20000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<20000> Mqtt_Sub; // JSON解析缓存
// 实例化类
TFT_eSPI tft = TFT_eSPI();
#include "support_functions.h"//加载png图片
WiFiUDP ntpUDP;
WiFiClientSecure espClient;
NTPClient timeClient(ntpUDP, "ntp2.aliyun.com", 8 * 3600, 60000);
RtcDS1307<TwoWire> Rtc(Wire);
OneButton Buton1(34, 1, 0);
OneButton Buton2(35, 1, 0);
OneButton Buton3(36, 1, 1);
Ticker button_wdg;
Ticker uptime;
void setup()
{
  pinMode(WeakFlag, INPUT);
  Serial.begin(9600);
  sd_en();
  //开机，显示开机画面
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(c_BL);
  tft.setTextColor(c_time);
  tft.setFreeFont(Digi);
  tft.drawString("Chwhsn", 50, 60);
  tft.drawString("Start.....", 50, 120);
  Scan_WiFi();
  WiFi.begin(ssid, password); // 连接网络
  Serial.print("Connecting to ");
  Serial.print(ssid);
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
  timeClient.begin();
  timeClient.update();
  _day = timeClient.getDay();
  _hour = timeClient.getHours();
  _minute = timeClient.getMinutes();
  Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  drawClass();
  time_update();
  uptime.attach(600, time_update);
}

void loop()
{
  // delay(10000);
  // time_update();
  // if (_minute / 10 == 1)
  // {
  //   drawClass();
  //   time_update();
  //   delay(60 * 1000);
  // }
  drawTime();
  delay(1000);
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
  Rtc._SetDateTime(_second, _minute, _hour);
}
void drawTime() {
  Rtc.Begin();  // DS1307时间读写
  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }
  RtcDateTime now = Rtc.GetDateTime();
  //tft.setTextColor(c_time,c_BL,1);
  tft.setFreeFont(PAPL);
  tft.setTextColor(c_time);
  tft.fillRect(175, 78, 65, 45, c_BL);
  if (now.Second() == 0) {
    tft.fillRect(95, 78, 65, 45, c_BL);
    if (now.Minute() == 0) {
      tft.fillRect(05, 78, 75, 45, c_BL);
    }
  }
  tft.drawString(String(now.Hour() / 10) + String(now.Hour() % 10) + ":" + String(now.Minute() / 10) + String(now.Minute() % 10) + ":" + String(now.Second() / 10) + String(now.Second() % 10), 12, 80);
}
// http请求
void get_net(String web)
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
      deserializeJson(Mqtt_Sub, payload);
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
// 扫描wifi
void Scan_WiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if (WiFi.SSID(0) == "weixingban 3lou  bangongshi")
  {
    ssid = "weixingban 3lou  bangongshi";
    password = passsyucai;
  }
  else if (WiFi.SSID(0) == "Nexus")
  {
    ssid = "Nexus";
    password = passhome;
  }
  tft.fillScreen(c_BL);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_Line);
  tft.drawString("Begin scan WiFi:", 25, 30);
  tft.setTextColor(c_text);
  for (int i = 0; i < 5; i++)
  {
    tft.drawString(WiFi.SSID(i), 25, 50 + 20 * i);
  }
  tft.drawString("We will connect the \n" + WiFi.SSID(0), 25, 170);
}
// 绘制课表
void drawClass()
{
  get_net(web_hf);
  // 处理http返回信息
  temp = Mqtt_Sub["now"]["temp"].as<String>() + "°C ";
  hump = Mqtt_Sub["now"]["humidity"].as<String>() + "% ";
  windDir = Mqtt_Sub["now"]["windDir"].as<String>();
  wind = Mqtt_Sub["now"]["windScale"].as<String>();
  _weather = Mqtt_Sub["now"]["text"].as<String>();
  _date = Mqtt_Sub["now"]["obsTime"].as<String>().substring(0, 10) + " ";
  String _Day = Mqtt_Sub["now"]["obsTime"].as<String>().substring(8, 10);
  String _icon = Mqtt_Sub["now"]["icon"].as<String>();
  tft.fillScreen(c_BL);
  tft.fillRect(0, 0, 480, 30, c_text);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  tft.setTextColor(c_text);
  tft.drawString(_Day + " A1 A2 A3 A4 B1 B2 B3 B4 C1 C2 C3", 12, 35);
  drawSdJpeg(("/128/" + _icon + ".png.jpg").c_str(), 0, 120);
  //setPngPosition(0,120);
  //load_file(SD, ("/weather_jpg/" + _icon + ".png").c_str());
  //load_file(SD, "/64/100.png");
  for (int i = 0; i < 11; i++)
  {
    tft.drawLine(26 + 12 + 6 + 39 * i, 35, 26 + 12 + 6 + 39 * i, 70, c_Line);
  }
  tft.drawLine(12, 55, 468, 55, c_Line);
  deserializeJson(Mqtt_Sub, _class);
  String CL;
  if (digitalRead(WeakFlag))
  {
    CL = Mqtt_Sub["Single"][_day]["no"].as<String>();
  }
  else
  {
    CL = Mqtt_Sub["Double"][_day]["no"].as<String>();
  }
  tft.drawString(CL, 12, 57);
  tft.setFreeFont(DejaVu);
  tft.setTextColor(c_BL);
  tft.drawString(_date + temp + hump + wind, 12, 5);
  time_update();
  //setPngPosition(180,20);
  // load_png(map_http.c_str());
}
void sd_en() {
  if (!SD.begin(sdSelectPin)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
}
void drawSdJpeg(const char *filename, int xpos, int ypos) {

  // Open the named file (the Jpeg decoder library will close it)
  File jpegFile = SD.open( filename, FILE_READ);  // or, file handle reference for SD library

  if ( !jpegFile ) {
    Serial.print("ERROR: File \""); Serial.print(filename); Serial.println ("\" not found!");
    return;
  }

  Serial.println("===========================");
  Serial.print("Drawing file: "); Serial.println(filename);
  Serial.println("===========================");

  // Use one of the following methods to initialise the decoder:
  bool decoded = JpegDec.decodeSdFile(jpegFile);  // Pass the SD file handle to the decoder,
  //bool decoded = JpegDec.decodeSdFile(filename);  // or pass the filename (String or character array)

  if (decoded) {
    // print information about the image to the serial port
    jpegInfo();
    // render the image onto the screen at given coordinates
    jpegRender(xpos, ypos);
  }
  else {
    Serial.println("Jpeg file format not supported!");
  }
}

//####################################################################################################
// Draw a JPEG on the TFT, images will be cropped on the right/bottom sides if they do not fit
//####################################################################################################
// This function assumes xpos,ypos is a valid screen coordinate. For convenience images that do not
// fit totally on the screen are cropped to the nearest MCU size and may leave right/bottom borders.
void jpegRender(int xpos, int ypos) {

  //jpegInfo(); // Print information from the JPEG file (could comment this line out)

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
  while (JpegDec.read()) {    // While there is more data in the file
    pImg = JpegDec.pImage ;   // Decode a MCU (Minimum Coding Unit, typically a 8x8 or 16x16 pixel block)

    // Calculate coordinates of top left corner of current MCU
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    // check if the image block size needs to be changed for the right edge
    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    // check if the image block size needs to be changed for the bottom edge
    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

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
    if (( mcu_x + win_w ) <= tft.width() && ( mcu_y + win_h ) <= tft.height())
      tft.pushImage(mcu_x, mcu_y, win_w, win_h, pImg);
    else if ( (mcu_y + win_h) >= tft.height())
      JpegDec.abort(); // Image has run off bottom of screen so abort decoding
  }

  tft.setSwapBytes(swapBytes);

  showTime(millis() - drawTime); // These lines are for sketch testing only
}
void jpegInfo() {
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
void showTime(uint32_t msTime) {
  Serial.print(F(" JPEG drawn in "));
  Serial.print(msTime);
  Serial.println(F(" ms "));
}

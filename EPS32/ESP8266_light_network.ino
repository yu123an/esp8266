//加载库
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include "Ticker.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
//敏感信息
const char *ssid = "wei xing ban ban gong shi";
const char *password = "weixing1234+-*/";
const char *mqtt_server = "*********";
//信息缓存
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (20000)
char msg[MSG_BUFFER_SIZE];
StaticJsonDocument<20000> Mqtt_Sub; // JSON解析缓存
//引脚定义
//双色温LED对应PWM引脚
#define WhiteLed 5
#define YellowLed 4
//联网设置引脚
#define WiFi_State 4
//联网标志位
int WiFiFlag;
// WS2812控制引脚，灯珠数
#define PIN 15
#define NUMPIXELS 11
//编码器引脚
#define ROTARY_ENCODER_A_PIN 12
#define ROTARY_ENCODER_B_PIN 14
#define ROTARY_ENCODER_BUTTON_PIN 13
#define ROTARY_ENCODER_VCC_PIN -1
//未知参数
#define ROTARY_ENCODER_STEPS 4
//变量声明
int ButtonFlag = 1; //按钮功能标志位；1调整亮度；0调整色温
//亮度值
int LedBrightness = 128;
//双灯珠色温
int ColorTempWhite = 127;
int ColorTempYellow = 128;
//双灯珠亮度
int WhiteBrightness = 127;
int YellowBrightness = 128;
//亮度，色温记忆功能
int LastLedBrightness;
int LastColorTempWhite;
//实例对象
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN,
														  ROTARY_ENCODER_B_PIN,
														  ROTARY_ENCODER_BUTTON_PIN,
														  ROTARY_ENCODER_VCC_PIN,
														  ROTARY_ENCODER_STEPS);
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Ticker ticker;
WiFiClient espClient;
PubSubClient client(espClient);
void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}
void setup()
{
	Serial.begin(9600);
// put your setup code here, to run once:
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
	clock_prescale_set(clock_div_1);
#endif
	pixels.begin();
	pinMode(WiFi_State, INPUT);
	pinMode(WhiteLed, OUTPUT);
	pinMode(YellowLed, OUTPUT);
	WiFiFlag = digitalRead(WiFi_State);
	if (WiFiFlag)
	{
		//不联网
		//离线模式
		Serial.println("Begin with no newwork");
	}
	else
	{
		//联网操作
		WiFi.begin(ssid, password); // 连接网络
		while (WiFi.status() != WL_CONNECTED)
		{ // 等待连接
			delay(500);
			Serial.print('.');
		}
		Serial.println('\n');
		Serial.print("Connecting to ");
		Serial.print(ssid);
		Serial.print("IP address:\t");
		Serial.println(WiFi.localIP());
		client.setServer(mqtt_server, 1383);
		client.setCallback(callback);
		if (!client.connected())
		{
			reconnect();
		}
		client.loop();
	}
	// we must initialize rotary encoder
	rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
	bool circleValues = false;
	rotaryEncoder.setBoundaries(0, 255, circleValues); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
	rotaryEncoder.setAcceleration(250);				   // or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
	// pwm频率
	analogWriteFreq(10000); // 10KHZ
	//设置编码器初始值
	rotaryEncoder.setEncoderValue(127);
	//设置亮度值
	analogWrite(WhiteLed, 0);
	analogWrite(YellowLed, 0);
	// ec11检测
	ticker.attach_ms(500, rotary_loop);
}

void loop()
{
	// put your main code here, to run repeatedly:
	if (WiFiFlag)
	{
		//不联网
		//离线模式
		Serial.println("Begin with no newwork");
	}
	else
	{
		Pub_msg();
	}
	delay(3000);
}
//编码器函数
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
	//循环设置亮度与色温值
	//读取亮度，色温记忆值
	ButtonFlag -= 1;
	if (ButtonFlag)
	{
		//调节亮度
		rotaryEncoder.setEncoderValue(LastLedBrightness);
	}
	else
	{
		rotaryEncoder.setEncoderValue(LastColorTempWhite);
	}
}
void rotary_loop()
{
	//先行调解亮度和色温，
	//单灯珠亮度等于总亮度*色温÷256
	//最终得出每种色温灯珠的亮度值
	// dont print anything unless value changed
	if (rotaryEncoder.encoderChanged())
	{
		Serial.print("Value: ");
		Serial.println(rotaryEncoder.readEncoder());
		if (ButtonFlag)
		{ //调节亮度
			LedBrightness = rotaryEncoder.readEncoder();
			LastLedBrightness = LedBrightness;
		}
		else
		{ //调节色温
			ColorTempWhite = rotaryEncoder.readEncoder();
			ColorTempYellow = 255 - ColorTempWhite;
			LastColorTempWhite = ColorTempWhite;
		}
		WhiteBrightness = LedBrightness * ColorTempWhite >> 8;
		YellowBrightness = LedBrightness * ColorTempYellow >> 8;
		analogWrite(WhiteLed, WhiteBrightness);
		analogWrite(YellowLed, YellowBrightness);
	}
	if (rotaryEncoder.isEncoderButtonClicked())
	{
		rotary_onButtonClick();
	}
}
// MQTT相关函数
//订阅主题：LightIn
//发布主题：LightOut
void callback(char *topic, byte *payload, unsigned int length)
{
	payload[length] = 0;
	deserializeJson(Mqtt_Sub, String((char *)payload)); //对接收到的MQTT_Message进行JSON解析
	// JSON文件格式：https://github.com/yu123an/esp8266/blob/master/EPS32/json.log
	//"Type": "weather",//消息类型，包含：weather，message，gif，
	String type = Mqtt_Sub["Type"];
	if (type == "Light")
	{
		LedBrightness = Mqtt_Sub["LedBrightness"];
		LastLedBrightness = LedBrightness;
		rotaryEncoder.setEncoderValue(LastLedBrightness);
		WhiteBrightness = LedBrightness * ColorTempWhite >> 8;
		YellowBrightness = LedBrightness * ColorTempYellow >> 8;
		analogWrite(WhiteLed, WhiteBrightness);
		analogWrite(YellowLed, YellowBrightness);
	}
	else if (type == "ColorTemp")
	{
		ColorTempWhite = Mqtt_Sub["ColorTempWhite"];
		ColorTempYellow = 255 - ColorTempWhite;
		LastColorTempWhite = ColorTempWhite;
		rotaryEncoder.setEncoderValue(LastColorTempWhite);
		WhiteBrightness = LedBrightness * ColorTempWhite >> 8;
		YellowBrightness = LedBrightness * ColorTempYellow >> 8;
		analogWrite(WhiteLed, WhiteBrightness);
		analogWrite(YellowLed, YellowBrightness);
	}
}
void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection...");
		// Create a random client ID
		String clientId = "ESP8266-Light";
		// Attempt to connect
		if (client.connect(clientId.c_str()))
		{
			Serial.println("connected");
			// Once connected, publish an announcement...
			// client.publish("outTopic", "hello world");
			// ... and resubscribe
			client.subscribe("LightIn");
		}
		else
		{
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}
void Pub_msg()
{
	String A = "{\"Type\":\"EspLight\",\"Brightness\":";
	String B = ",\"WhiteBrightness\":";
	String C = ",\"YellowBrightness\":";
	String G = "}";
	String ALL = A + String(LedBrightness) +
				 B + String(ColorTempWhite) +
				 C + String(ColorTempWhiteYellow) + G;
	char _ALL[800];
	ALL.toCharArray(_ALL, 800);
	client.publish("LightOut", _ALL);
}

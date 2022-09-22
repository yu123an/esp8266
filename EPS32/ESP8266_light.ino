//加载库
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include "Ticker.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
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
void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}
void setup()
{
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
	}
	else
	{
		//联网操作
	}
	Serial.begin(9600);

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
			LastLedBrightness = rotaryEncoder.readEncoder();
		}
		else
		{ //调节色温
			ColorTempWhite = rotaryEncoder.readEncoder();
			ColorTempYellow = 255 - ColorTempWhite;
			LastColorTempWhite = rotaryEncoder.readEncoder();
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
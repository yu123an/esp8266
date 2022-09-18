//加载库
#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#include "Ticker.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
//引脚定义
#define WhiteLed 5
#define YellowLed 4
#define WiFi_State 4
#define PIN 15
#define NUMPIXELS 11
//编码器引脚
#define ROTARY_ENCODER_A_PIN 12
#define ROTARY_ENCODER_B_PIN 14
#define ROTARY_ENCODER_VCC_PIN -1
#define ROTARY_ENCODER_STEPS 4
//变量声明
int ButtonFlag = 1;
int LedBrightness = 128;
int ColorTempWhite = 127;
int ColorTempYellow = 128;
int WhiteBrightness = 127;
int YellowBrightness = 128;
#define ROTARY_ENCODER_BUTTON_PIN 13
//联网标志位
int WiFiFlag;
//实例对象
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
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
	analogWriteFreq(10000);							   // 10KHZ
	analogWrite(WhiteLed, 0);
	analogWrite(YellowLed, 0);
	ticker.attach_ms(500,rotary_loop);
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
	ButtonFlag -= 1;
}
void rotary_loop()
{
	// dont print anything unless value changed
	if (rotaryEncoder.encoderChanged())
	{
		Serial.print("Value: ");
		Serial.println(rotaryEncoder.readEncoder());
		if (ButtonFlag)
		{ //调节亮度
			LedBrightness = rotaryEncoder.readEncoder();
		}
		else
		{ //调节色温
			ColorTempWhite = rotaryEncoder.readEncoder();
			ColorTempYellow = 255 - ColorTempWhite;
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

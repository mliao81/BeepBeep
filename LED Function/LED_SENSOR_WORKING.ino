#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#define LED_COUNT 300
#define Pin 9

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, Pin, NEO_GRB + NEO_KHZ800);
Adafruit_TSL2591 lightSensor = Adafruit_TSL2591(2591);

int R = 255;
int G = 100;
int B = 150;
int brightness = 32;

void setup()
{
  Serial.begin(9600);

  leds.begin();
  clearLEDs();

  if (!lightSensor.begin())
  {
    Serial.println("light sensor not found");
    while (1)
      ;
  }

  lightSensor.setGain(TSL2591_GAIN_MED);
  lightSensor.setTiming(TSL2591_INTEGRATIONTIME_100MS);
}

void loop()
{
  uint16_t lightLevel = lightSensor.getFullLuminosity();

  Serial.print("Light level: ");
  Serial.println(lightLevel);

  uint16_t threshold = 200;

  if (lightLevel < threshold)
  {
    for (int i = 0; i < LED_COUNT; i++)
    {
      leds.setPixelColor(i, R, G, B);
      leds.setBrightness(brightness);
      leds.show();
    }
  }
  else
  {
    clearLEDs();
  }
}

void clearLEDs()
{
  for (int i = 0; i < LED_COUNT; i++)
  {
    leds.setPixelColor(i, 0);
    leds.show();
  }
}
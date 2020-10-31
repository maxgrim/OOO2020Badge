#include "RGB.h"
#include "Badge.h"
#include "Definitions.h"
#include "Debug.h"

#include <Adafruit_NeoPixel.h>

static void handleBlinkLed();

int blinkLed, blinkTimes, blinkTimesExecuted;
uint32_t blinkColor;
Task taskBlinkLed(150, TASK_FOREVER, &handleBlinkLed);

static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(RGB_N_LEDS, PIN_LED_RGB, NEO_GRB + NEO_KHZ800);

static void handleBlinkLed()
{
    if (taskBlinkLed.getRunCounter() % 2 == 0)
    {
        pixels.clear();
        pixels.setPixelColor(blinkLed, blinkColor);
        pixels.show();
    }
    else
    {
        pixels.clear();
        pixels.show();
    }

    if (++blinkTimesExecuted > (blinkTimes * 2))
    {
        taskBlinkLed.disable();
    }
}

void rgbSetSingleLed(int led, uint32_t color)
{
    pixels.setPixelColor(led, color);
}

void rgbShow()
{
    pixels.show();
}

void rgbBlinkSingleLed(int led, int times, uint32_t color, TaskOnDisable doneCallback)
{
    blinkLed = led;
    blinkTimes = times;
    blinkColor = color;
    blinkTimesExecuted = 0;

    taskBlinkLed.setOnDisable(doneCallback);
    taskBlinkLed.restart();
}

void rgbSetAllLeds(uint32_t color)
{
    for (uint16_t i = 0; i < pixels.numPixels(); i++)
    {
        pixels.setPixelColor(i, color);
    }

    pixels.show();
}

void rgbClear()
{
    pixels.clear();
}

void rgbSetup()
{
    pixels.begin();
    pixels.clear();
    pixels.setBrightness(10);
    pixels.show();

    badgeTaskScheduler.addTask(taskBlinkLed);
}
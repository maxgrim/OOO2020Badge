#include "RGB.h"
#include "Badge.h"
#include "Definitions.h"
#include "Debug.h"

#include <Adafruit_NeoPixel.h>

static void handleBlinkLed();

uint8_t blinkNr;
uint16_t blinkTimes, blinkTimesExecuted;
uint32_t blinkColor;
Task taskBlinkLed(150, TASK_FOREVER, &handleBlinkLed);

static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(RGB_N_LEDS, PIN_LED_RGB, NEO_GRB + NEO_KHZ800);

static uint8_t convertNrToLed(uint8_t nr)
{
    return (nr + 9) % 12;
}

static void handleBlinkLed()
{
    if (taskBlinkLed.getRunCounter() % 2 == 0)
    {
        pixels.clear();
        pixels.setPixelColor(convertNrToLed(blinkNr), blinkColor);
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

void rgbSetSingleLed(uint8_t nr, uint32_t color)
{
    pixels.setPixelColor(convertNrToLed(nr), color);
}

void rgbShow()
{
    pixels.show();
}

void rgbBlinkSingleLed(uint8_t nr, uint16_t times, uint32_t color, TaskOnDisable doneCallback)
{
    blinkNr = nr;
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
}

void rgbSetBrightness(uint8_t brightness)
{
    pixels.setBrightness(brightness);
}

void rgbClear()
{
    pixels.clear();
}

void rgbSetup()
{
    pixels.begin();

    rgbClear();
    rgbShow();

    badgeTaskScheduler.addTask(taskBlinkLed);
}
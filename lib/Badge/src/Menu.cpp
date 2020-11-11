#include "Menu.h"
#include "RGB.h"
#include "Definitions.h"
#include "Debug.h"
#include "Badge.h"

// External programs to execute
#include "Challenges/CatchTheLed.h"
#include "Challenges/CombinationLock.h"
#include "Challenges/Morse.h"
#include "Challenges/SpaceMaze.h"
#include "Challenges/Webserver.h"
// ----------------------------

#include <Arduino.h>

#define MENU_POSITIONS 12

static int currentMenuPosition = 0;
static bool readingL, readingR, lastStateL, lastStateR;

static void detectButtonChange();
static Task tDetectButtonChange(TASK_IMMEDIATE, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(TASK_IMMEDIATE, TASK_FOREVER, &verifyButtonsLow);

static void showMenuAnimation();
static Task tShowMenuAnimation(50, TASK_FOREVER, &showMenuAnimation);

static uint8_t effectMaxBrightness = 25;
static uint8_t effectMinBrightness = 1;
static uint16_t effectCurrentBrightness = 0;
static int8_t effectBrightnessModifier = 1;

static const uint32_t colorActive = 0x0080FF;
static const uint32_t colorEffect = 0x00FF00;

static DoneCallback executionDone;

static void showMenuAnimation()
{
    // Only modify brightness every 2nd execution
    effectCurrentBrightness += effectBrightnessModifier;

    if (effectCurrentBrightness >= effectMaxBrightness)
    {
        effectCurrentBrightness = effectMaxBrightness;
        effectBrightnessModifier = -effectBrightnessModifier;
    }

    if (effectCurrentBrightness <= effectMinBrightness)
    {
        effectCurrentBrightness = effectMinBrightness;
        effectBrightnessModifier = -effectBrightnessModifier;
    }

    rgbSetBrightness(effectCurrentBrightness);

    for (int i = 0; i < RGB_N_LEDS; i++)
    {
        rgbSetSingleLed(i, i == currentMenuPosition ? colorActive : colorEffect);
    }

    rgbShow();
}

static void startCombinationLock()
{
    combinationLockSetup(&menuActivate);
}

static void startSpaceMaze()
{
    spaceMazeSetup(&menuActivate);
}

static void startWebserver()
{
    webserverSetup(&menuActivate);
}

static void startMorseCode()
{
    morseCodeSetup(&menuActivate);
}

static void startCatchTheLed()
{
    catchTheLedSetup(&menuActivate);
}

static void enterCurrentMenuPosition()
{
    menuDeactivate();
    executionDone = badgeRequestExecution(MENU);

    void (*blinkDoneCallback)();

    switch (currentMenuPosition)
    {
    case 0:
        blinkDoneCallback = &startCombinationLock;
        break;
    case 1:
        blinkDoneCallback = &startSpaceMaze;
        break;
    case 2:
        blinkDoneCallback = &startMorseCode;
        break;
    case 3:
        blinkDoneCallback = &startWebserver;
        break;
    case 4:
        blinkDoneCallback = &startCatchTheLed;
        break;
    default:
        blinkDoneCallback = &menuActivate;
        break;
    }

    rgbBlinkSingleLed(currentMenuPosition, 3, colorActive, blinkDoneCallback);
}

static void detectButtonChange()
{
    readingL = digitalRead(PIN_BUTTON_L);
    readingR = digitalRead(PIN_BUTTON_R);

    if (readingL != lastStateL || readingR != lastStateR)
    {
        tVerifyButtonChange.restartDelayed();
    }

    lastStateL = readingL;
    lastStateR = readingR;
}

static void verifyButtonChange()
{
    tDetectButtonChange.disable();

    if (readingL && readingR)
    {
        enterCurrentMenuPosition();
    }
    else if (readingL)
    {
        // Move menu left
        currentMenuPosition--;
        if (currentMenuPosition < 0)
        {
            currentMenuPosition = 11;
        }
    }
    else if (readingR)
    {
        // Move menu right
        currentMenuPosition++;
        if (currentMenuPosition > 11)
        {
            currentMenuPosition = 0;
        }
    }

    tVerifyButtonsLow.enable();
}

static void verifyButtonsLow()
{
    if (!digitalRead(PIN_BUTTON_L) && !digitalRead(PIN_BUTTON_R))
    {
        tVerifyButtonsLow.disable();
        tDetectButtonChange.enable();
    }
}

void menuActivate()
{
    if (executionDone != NULL)
    {
        executionDone();
        executionDone = NULL;
    }

    rgbClear();
    rgbSetSingleLed(currentMenuPosition, 0xFFFFFF);
    rgbShow();

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tShowMenuAnimation);

    tVerifyButtonsLow.enable();
    tShowMenuAnimation.enable();
}

void menuDeactivate()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tShowMenuAnimation);

    tVerifyButtonsLow.disable();
    tShowMenuAnimation.disable();
}

void menuSetup()
{
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    menuActivate();
}
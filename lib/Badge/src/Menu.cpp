#include "Menu.h"
#include "RGB.h"
#include "Definitions.h"
#include "Debug.h"
#include "Badge.h"

// External programs to execute
#include "Challenges/CombinationLock.h"
#include "Challenges/RasterDungeon.h"
// ----------------------------

#include <Arduino.h>

#define MENU_POSITIONS 12

static int currentMenuPosition = 0;
static bool readingL, readingR, lastStateL, lastStateR;

static void detectButtonChange();
static Task tDetectButtonChange(0, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(0, TASK_FOREVER, &verifyButtonsLow);

static void showMenuAnimation();
static Task tShowMenuAnimation(50, TASK_FOREVER, &showMenuAnimation);

uint8_t maxBrightness = 35;
uint8_t minBrightness = 5;
uint16_t currentBrightness = 0;
int8_t brightnessModifier = 1;

static const uint32_t colorActive = 0x0080FF;
static const uint32_t colorEffect = 0x00FF00;

static int convertToLedNr(int led)
{
    return (led + 9) % 12;
}

static void showMenuAnimation()
{
    // Only modify brightness every 2nd execution
    currentBrightness += brightnessModifier;

    if (currentBrightness >= maxBrightness)
    {
        currentBrightness = maxBrightness;
        brightnessModifier = -brightnessModifier;
    }

    if (currentBrightness <= minBrightness)
    {
        currentBrightness = minBrightness;
        brightnessModifier = -brightnessModifier;
    }

    rgbSetBrightness(currentBrightness);

    for (int i = 0; i < RGB_N_LEDS; i++)
    {
        rgbSetSingleLed(convertToLedNr(i), i == currentMenuPosition ? colorActive : colorEffect);
    }

    rgbShow();
}

static void activateMenu()
{
    rgbClear();
    rgbSetSingleLed(convertToLedNr(currentMenuPosition), 0xFFFFFF);
    rgbShow();

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tShowMenuAnimation);

    tDetectButtonChange.enable();
    tShowMenuAnimation.enable();
}

static void deactivateMenu()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tShowMenuAnimation);
}

static void startCombinationLock()
{
    combinationLockSetup(&activateMenu);
}

static void startDungeonSetup()
{
    rasterDungeonSetup(&activateMenu);
}

static void enterCurrentMenuPosition()
{
    deactivateMenu();

    void (*doneCallback)() = &activateMenu;

    switch (currentMenuPosition)
    {
    case 1:
        doneCallback = &startCombinationLock;
        break;
    case 2:
        doneCallback = &startDungeonSetup;
        break;
    }

    rgbBlinkSingleLed(convertToLedNr(currentMenuPosition), 3, colorActive, doneCallback);
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

void menuSetup()
{
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    activateMenu();
}
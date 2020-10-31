#include "Menu.h"
#include "RGB.h"
#include "Definitions.h"
#include "Debug.h"
#include "Badge.h"

// External programs to execute
#include "Challenges/CombinationLock.h"
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

static int convertToLedNr(int led)
{
    return (led + 9) % 12;
}

static void activateMenu()
{
    rgbSetSingleLed(convertToLedNr(currentMenuPosition), 0xFFFFFF);

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);

    tDetectButtonChange.enable();
}

static void deactivateMenu()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
}

static void startCombinationLock()
{
    combinationLockSetup(&activateMenu);
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
    }

    rgbBlinkSingleLed(convertToLedNr(currentMenuPosition), 3, 0xFF0000, doneCallback);
}

void detectButtonChange()
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

        rgbSetSingleLed(convertToLedNr(currentMenuPosition), 0xFFFFFF);
    }
    else if (readingR)
    {
        // Move menu right
        currentMenuPosition++;
        if (currentMenuPosition > 11)
        {
            currentMenuPosition = 0;
        }

        rgbSetSingleLed(convertToLedNr(currentMenuPosition), 0xFFFFFF);
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
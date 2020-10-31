#include "CombinationLock.h"

#include "../Badge.h"
#include "../Debug.h"
#include "../Definitions.h"
#include "../RGB.h"

#include <Arduino.h>

static void detectButtonChange();
static Task tDetectButtonChange(0, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(0, TASK_FOREVER, &verifyButtonsLow);

static bool readingL, readingR, lastStateL, lastStateR;

static void (*doneCallbackF)();

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
        DEBUG_PRINTLN("CombinationLock: confirm");
    }
    else if (readingL)
    {
        DEBUG_PRINTLN("CombinationLock: left button pressed, moved to position");
    }
    else if (readingR)
    {
        DEBUG_PRINTLN("CombinationLock: right button pressed, moved to position");
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

static void deactivateCombinationLock() {
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);

    doneCallbackF();
}

void combinationLockSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    Serial.printf("Lock combination challenge\r\n");
    rgbSetAllLeds(0);

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);

    tDetectButtonChange.enable();
}
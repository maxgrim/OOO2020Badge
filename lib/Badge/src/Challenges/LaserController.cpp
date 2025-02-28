#include "LaserController.h"

#include "../Badge.h"
#include "../Debug.h"
#include "../Eyes.h"
#include "../Definitions.h"
#include "../FlagCrypto.h"
#include "../RGB.h"

#include <Arduino.h>

static void detectButtonChange();
static Task tDetectButtonChange(TASK_IMMEDIATE, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(TASK_IMMEDIATE, TASK_FOREVER, &verifyButtonsLow);

static void updateRGB();
static Task tUpdateRGB(25, TASK_FOREVER, &updateRGB);

static void playWinAnimation();
static Task tPlayWinAnimation(200, 18, &playWinAnimation);

static void playFailAnimation();
static Task tPlayFailAnimation(200, 7, &playFailAnimation);

static bool readingL, readingR, lastStateL, lastStateR;

static uint8_t codeLength = 4, currentCodePosition = 0;
static uint8_t correctCode[] = {6, 11, 9, 2};
static uint8_t currentAnswers[] = {0, 0, 0, 0};
static int8_t currentCodeAnswer = 0;

static int8_t rgbBrigtnessModifier = 1;
static uint8_t rgbMinBrightness = 1, rgbMaxBrightness = 25;
static uint16_t rgbCurrentBrightness = 0;

static void (*doneCallbackF)();

static void deactivateLaserController()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tUpdateRGB);
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

static void blinkDone()
{
    tVerifyButtonsLow.enable();
    tUpdateRGB.enable();
}

static void verifyButtonChange()
{
    tDetectButtonChange.disable();

    if (readingL && readingR)
    {
        currentAnswers[currentCodePosition++] = currentCodeAnswer;
        Serial.print(currentCodeAnswer);
        Serial.printf(" ");

        if (currentCodePosition >= codeLength)
        {
            bool correct = false;
            for (uint8_t i = 0; i < codeLength; i++)
            {
                if (currentAnswers[i] != correctCode[i])
                {
                    correct = false;
                    break;
                }

                correct = true;
            }

            if (correct)
            {
                Serial.print(F("\r\n\r\n  >> ACCESS GRANTED <<\r\n\r\nLaser does pew pew pew! "));

                char flag[38];
                cryptoGetFlag(&flag[0], sizeof(flag), 16);
                Serial.println(flag);

                deactivateLaserController();
                badgeTaskScheduler.addTask(tPlayWinAnimation);
                tPlayWinAnimation.restart();
            }
            else
            {
                Serial.print(F("\r\n\r\n  >> ACCESS DENIED <<\r\n\r\n"));
                Serial.print(F("Code: "));

                badgeTaskScheduler.addTask(tPlayFailAnimation);
                tPlayFailAnimation.restart();
            }

            currentCodePosition = 0;
        }
        else
        {
            tUpdateRGB.disable();
            rgbBlinkSingleLed(currentCodeAnswer, 3, 0xFFFFFF, &blinkDone);
        }
    }
    else if (readingL)
    {
        currentCodeAnswer--;
        if (currentCodeAnswer < 0)
        {
            currentCodeAnswer = 11;
        }

        tVerifyButtonsLow.enable();
    }
    else if (readingR)
    {
        currentCodeAnswer++;
        if (currentCodeAnswer > 11)
        {
            currentCodeAnswer = 0;
        }

        tVerifyButtonsLow.enable();
    }
    else
    {
        tVerifyButtonsLow.enable();
    }
}

static void verifyButtonsLow()
{
    if (!digitalRead(PIN_BUTTON_L) && !digitalRead(PIN_BUTTON_R))
    {
        tVerifyButtonsLow.disable();
        tDetectButtonChange.enable();
    }
}

static void playWinAnimation()
{
    if (tPlayWinAnimation.isFirstIteration())
    {
        eyesOff();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayWinAnimation.isLastIteration())
    {
        eyesOn();
        badgeTaskScheduler.deleteTask(tPlayWinAnimation);

        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }

        return;
    }

    if (tPlayWinAnimation.getRunCounter() <= RGB_N_LEDS)
    {
        for (int i = 0; i < tPlayWinAnimation.getRunCounter(); i++)
        {
            rgbSetSingleLed(i, 0x00FF00);
        }

        rgbShow();
    }
}

static void playFailAnimation()
{

    if (tPlayFailAnimation.isFirstIteration())
    {
        tUpdateRGB.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayFailAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateRGB.enable();
        badgeTaskScheduler.deleteTask(tPlayFailAnimation);
        tVerifyButtonsLow.enable();
        return;
    }

    unsigned long counter = tPlayFailAnimation.getRunCounter();
    if (counter % 2 == 1)
    {
        eyesOff();
        rgbClear();
        rgbShow();
    }
    else
    {
        eyesOn();
        rgbSetAllLeds(0xFF0000);
        rgbShow();
    }
}

static void updateRGB()
{
    // Only modify brightness every 2nd execution
    if (tUpdateRGB.getRunCounter() % 2 == 1)
    {
        rgbCurrentBrightness += rgbBrigtnessModifier;

        if (rgbCurrentBrightness >= rgbMaxBrightness)
        {
            rgbCurrentBrightness = rgbMaxBrightness;
            rgbBrigtnessModifier = -rgbBrigtnessModifier;
        }

        if (rgbCurrentBrightness <= rgbMinBrightness)
        {
            rgbCurrentBrightness = rgbMinBrightness;
            rgbBrigtnessModifier = -rgbBrigtnessModifier;
        }

        rgbSetBrightness(rgbCurrentBrightness);
    }

    rgbSetAllLeds(0xFF0000);
    rgbSetSingleLed(currentCodeAnswer, 0xFFFFFF);
    rgbShow();
}

void laserControllerSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    rgbClear();
    rgbShow();

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tUpdateRGB);

    Serial.print(F("\r\n\r\n\
    __                            ______            __             ____         \r\n\
   / /   ____ _________  _____   / ____/___  ____  / /__________  / / /__  _____\r\n\
  / /   / __ `/ ___/ _ \\/ ___/  / /   / __ \\/ __ \\/ __/ ___/ __ \\/ / / _ \\/ ___/\r\n\
 / /___/ /_/ (__  )  __/ /     / /___/ /_/ / / / / /_/ /  / /_/ / / /  __/ /    \r\n\
/_____/\\__,_/____/\\___/_/      \\____/\\____/_/ /_/\\__/_/   \\____/_/_/\\___/_/     \r\n\
                                                                                \r\n\
\r\n\r\nShoot! The laser is locked. Can you crack the code? Use the buttons\r\n\r\n"));

    Serial.print(F("Code: "));

    tDetectButtonChange.enable();
    tUpdateRGB.enable();
}
#include "CatchTheLed.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../Definitions.h"
#include "../FlagCrypto.h"
#include "../RGB.h"

#include <Arduino.h>

static void (*doneCallbackF)();

static uint8_t score = 0, currentPixel = 0, hitPixel = 6;
static ushort startInterval = 600;
static bool currentDirection;

static void updateCatchTheLed();
static Task tUpdateCatchTheLed(startInterval, TASK_FOREVER, &updateCatchTheLed);

static void detectButtonChange();
static Task tDetectButtonChange(TASK_IMMEDIATE, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(10, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(TASK_IMMEDIATE, TASK_FOREVER, &verifyButtonsLow);

static void playDeadAnimation();
static Task tPlayDeadAnimation(200, 7, &playDeadAnimation);

static void playHitAnimation();
static Task tPlayHitAnimation(400, 3, &playHitAnimation);

static bool readingL, readingR, lastStateL, lastStateR;

static void deactivateCatchTheLed()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tUpdateCatchTheLed);
}

static void playDeadAnimation()
{
    if (tPlayDeadAnimation.isFirstIteration())
    {
        eyesOff();
        tUpdateCatchTheLed.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayDeadAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateCatchTheLed.enable();
        badgeTaskScheduler.deleteTask(tPlayDeadAnimation);
        tVerifyButtonsLow.enable();
        return;
    }

    unsigned long counter = tPlayDeadAnimation.getRunCounter();
    if (counter % 2 == 1)
    {
        rgbClear();
        rgbShow();
    }
    else
    {
        rgbSetAllLeds(0xFF0000);
        rgbShow();
    }
}

static void playHitAnimation()
{
    if (tPlayHitAnimation.isFirstIteration())
    {
        tUpdateCatchTheLed.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayHitAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateCatchTheLed.enable();
        badgeTaskScheduler.deleteTask(tPlayDeadAnimation);
        tVerifyButtonsLow.enable();
        return;
    }

    unsigned long counter = tPlayHitAnimation.getRunCounter();
    if (counter % 2 == 1)
    {
        eyesOff();
        rgbClear();
        rgbShow();
    }
    else
    {
        eyesOn();
        rgbSetAllLeds(0x00FF00);
        rgbShow();
    }
}

static void updateCatchTheLed()
{
    rgbClear();

    currentPixel += (currentDirection) ? -1 : 1;

    if (currentPixel <= 0 || currentPixel >= RGB_N_LEDS - 1)
    {
        currentDirection = !currentDirection;
    }

    if (currentPixel == hitPixel)
    {
        rgbSetSingleLed(hitPixel, 0xFFFF00);
    }
    else
    {
        rgbSetSingleLed(hitPixel, 0xFF0000);
        rgbSetSingleLed(currentPixel, 0x00FF00);
    }

    rgbShow();

    if (currentPixel == hitPixel)
    {
        Serial.printf("Shoot now!\r\n");
    }
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

    if (readingL || readingR)
    {
        if (currentPixel == hitPixel)
        {
            Serial.printf("Hit\r\n");

            long modifier;

            if (tUpdateCatchTheLed.getInterval() <= 100)
            {
                modifier = 20;
            }
            else if (tUpdateCatchTheLed.getInterval() <= 200)
            {
                modifier = 25;
            }
            else if (tUpdateCatchTheLed.getInterval() <= 400)
            {
                modifier = 50;
            }
            else
            {
                modifier = 100;
            }

            tUpdateCatchTheLed.setInterval(tUpdateCatchTheLed.getInterval() - modifier);

            if (tUpdateCatchTheLed.getInterval() == 0)
            {
                Serial.printf("You won, here is your flag: ");
                const char *encryptedFlag = "qiF+XICLazJqo48fojtb6GALju7VHdF6muaMw6OUSi4sp9LjE1GWEhXQvSTsPhmghZncJkwnsOrq789P9JP9ad4Y3yDQmCnkKoM=";

                const uint8_t aesKey[AES_BLOCK_SIZE] = {0xc2, 0xce, 0x75, 0x9a, 0x23, 0x2e, 0x44, 0xff, 0xd0, 0x53, 0x53, 0x35, 0x94, 0x5f, 0xa7, 0x16};
                const uint8_t aesIV[AES_BLOCK_SIZE] = {0x14, 0x3e, 0xc0, 0x07, 0x4c, 0x73, 0x57, 0x58, 0x84, 0xf8, 0xcb, 0x6a, 0xe2, 0x30, 0x6b, 0x7c};
                char destination[strlen(encryptedFlag)];

                cryptoGetFlagAES(aesKey, aesIV, encryptedFlag, destination);

                deactivateCatchTheLed();
                if (doneCallbackF != NULL)
                {
                    doneCallbackF();
                }
            }

            badgeTaskScheduler.addTask(tPlayHitAnimation);
            tPlayHitAnimation.restart();
        }
        else
        {
            Serial.printf("Miss\r\n");
            badgeTaskScheduler.addTask(tPlayDeadAnimation);
            tPlayDeadAnimation.restart();
        }
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

void catchTheLedSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    eyesOff();

    rgbClear();
    rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    rgbShow();

    Serial.printf("\r\n");
    Serial.printf("     _______..______      ___       ______  _______                                                                          \r\n");
    Serial.printf("    /       ||   _  \\    /   \\     /      ||   ____|                                                                         \r\n");
    Serial.printf("   |   (----`|  |_)  |  /  ^  \\   |  ,----'|  |__                                                                            \r\n");
    Serial.printf("    \\   \\    |   ___/  /  /_\\  \\  |  |     |   __|                                                                           \r\n");
    Serial.printf(".----)   |   |  |     /  _____  \\ |  `----.|  |____                                                                          \r\n");
    Serial.printf("|_______/    | _|    /__/     \\__\\ \\______||_______|                                                                         \r\n");
    Serial.printf("                                                                                                                             \r\n");
    Serial.printf("      ___   ___ .___________. _______ .______      .___  ___.  __  .__   __.      ___   .___________.  ______   .______      \r\n");
    Serial.printf("      \\  \\ /  / |           ||   ____||   _  \\     |   \\/   | |  | |  \\ |  |     /   \\  |           | /  __  \\  |   _  \\     \r\n");
    Serial.printf("       \\  V  /  `---|  |----`|  |__   |  |_)  |    |  \\  /  | |  | |   \\|  |    /  ^  \\ `---|  |----`|  |  |  | |  |_)  |    \r\n");
    Serial.printf("        >   <       |  |     |   __|  |      /     |  |\\/|  | |  | |  . `  |   /  /_\\  \\    |  |     |  |  |  | |      /     \r\n");
    Serial.printf("       /  .  \\      |  |     |  |____ |  |\\  \\----.|  |  |  | |  | |  |\\   |  /  _____  \\   |  |     |  `--'  | |  |\\  \\----.\r\n");
    Serial.printf("      /__/ \\__\\     |__|     |_______|| _| `._____||__|  |__| |__| |__| \\__| /__/     \\__\\  |__|      \\______/  | _| `._____|\r\n");
    Serial.printf("                                                                                                                             \r\n");
    Serial.printf("\r\n");

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tUpdateCatchTheLed);

    tUpdateCatchTheLed.enable();
    tDetectButtonChange.enable();
}
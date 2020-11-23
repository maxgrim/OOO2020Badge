#include "SpaceXTerminator.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../Definitions.h"
#include "../FlagCrypto.h"
#include "../RGB.h"

#include <Arduino.h>

static void (*doneCallbackF)();

static uint8_t score = 0, currentPixel = 0, hitPixel = 6;
static uint16_t startInterval = 600;
static bool currentDirection;

static void updateSpaceXTerminator();
static Task tUpdateSpaceXTerminator(startInterval, TASK_FOREVER, &updateSpaceXTerminator);

static void detectButtonChange();
static Task tDetectButtonChange(TASK_IMMEDIATE, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(10, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(TASK_IMMEDIATE, TASK_FOREVER, &verifyButtonsLow);

static void playDeadAnimation();
static Task tPlayDeadAnimation(200, 7, &playDeadAnimation);

static void playWinAnimation();
static Task tPlayWinAnimation(200, 7, &playWinAnimation);

static void playHitAnimation();
static Task tPlayHitAnimation(400, 3, &playHitAnimation);

static bool readingL, readingR, lastStateL, lastStateR;

static const char *encryptedFlag = "\xaa\x21\x7e\x5c\x80\x8b\x6b\x32\x6a\xa3\x8f\x1f\xa2\x3b\x5b\xe8\x60\x0b\x8e\xee\xd5\x1d\xd1\x7a\x9a\xe6\x8c\xc3\xa3\x94\x4a\x2e\x2c\xa7\xd2\xe3\x13\x51\x96\x12\x15\xd0\xbd\x24\xec\x3e\x19\xa0\x85\x99\xdc\x26\x4c\x27\xb0\xea\xea\xef\xcf\x4f\xf4\x93\xfd\x69\xde\x18\xdf\x20\xd0\x98\x29\xe4\x2a\x83";

static void deactivateSpaceXTerminator()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tUpdateSpaceXTerminator);
}

static void playDeadAnimation()
{
    if (tPlayDeadAnimation.isFirstIteration())
    {
        eyesOff();
        tUpdateSpaceXTerminator.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayDeadAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateSpaceXTerminator.enable();
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

static void playWinAnimation()
{
    if (tPlayWinAnimation.isFirstIteration())
    {
        eyesOff();
        tUpdateSpaceXTerminator.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayWinAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateSpaceXTerminator.enable();
        badgeTaskScheduler.deleteTask(tPlayWinAnimation);

        deactivateSpaceXTerminator();
        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }

        return;
    }

    unsigned long counter = tPlayWinAnimation.getRunCounter();
    if (counter % 2 == 1)
    {
        rgbClear();
        rgbShow();
    }
    else
    {
        rgbSetAllLeds(0x00FF00);
        rgbShow();
    }
}

static void playHitAnimation()
{
    if (tPlayHitAnimation.isFirstIteration())
    {
        tUpdateSpaceXTerminator.disable();
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayHitAnimation.isLastIteration())
    {
        eyesOn();
        tUpdateSpaceXTerminator.enable();
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

static void updateSpaceXTerminator()
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
        Serial.println(F("Shoot now!"));
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
            Serial.println(F("Hit"));

            long modifier;

            if (tUpdateSpaceXTerminator.getInterval() <= 200)
            {
                modifier = 20;
            }
            else if (tUpdateSpaceXTerminator.getInterval() <= 400)
            {
                modifier = 50;
            }
            else
            {
                modifier = 100;
            }

            tUpdateSpaceXTerminator.setInterval(tUpdateSpaceXTerminator.getInterval() - modifier);

            if (tUpdateSpaceXTerminator.getInterval() <= 0)
            {
                Serial.print(F("You won, here is your flag: "));

                const uint8_t aesKey[AES_BLOCK_SIZE] = {0xc2, 0xce, 0x75, 0x9a, 0x23, 0x2e, 0x44, 0xff, 0xd0, 0x53, 0x53, 0x35, 0x94, 0x5f, 0xa7, 0x16};
                const uint8_t aesIV[AES_BLOCK_SIZE] = {0x14, 0x3e, 0xc0, 0x07, 0x4c, 0x73, 0x57, 0x58, 0x84, 0xf8, 0xcb, 0x6a, 0xe2, 0x30, 0x6b, 0x7c};
                char destination[strlen(encryptedFlag)];

                cryptoGetFlagAES(aesKey, aesIV, encryptedFlag, destination);
                Serial.println(destination);

                badgeTaskScheduler.addTask(tPlayWinAnimation);
                tPlayWinAnimation.restart();

                return;
            }

            badgeTaskScheduler.addTask(tPlayHitAnimation);
            tPlayHitAnimation.restart();
        }
        else
        {
            Serial.printf("Miss\r\n");
            tUpdateSpaceXTerminator.setInterval(startInterval);
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

void spaceXTerminatorSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    rgbClear();
    rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    rgbShow();

    Serial.print(F("\r\n\
     _______..______      ___       ______  _______                                                                          \r\n\
    /       ||   _  \\    /   \\     /      ||   ____|                                                                         \r\n\
   |   (----`|  |_)  |  /  ^  \\   |  ,----'|  |__                                                                            \r\n\
    \\   \\    |   ___/  /  /_\\  \\  |  |     |   __|                                                                           \r\n\
.----)   |   |  |     /  _____  \\ |  `----.|  |____                                                                          \r\n\
|_______/    | _|    /__/     \\__\\ \\______||_______|                                                                         \r\n\
                                                                                                                             \r\n\
      ___   ___ .___________. _______ .______      .___  ___.  __  .__   __.      ___   .___________.  ______   .______      \r\n\
      \\  \\ /  / |           ||   ____||   _  \\     |   \\/   | |  | |  \\ |  |     /   \\  |           | /  __  \\  |   _  \\     \r\n\
       \\  V  /  `---|  |----`|  |__   |  |_)  |    |  \\  /  | |  | |   \\|  |    /  ^  \\ `---|  |----`|  |  |  | |  |_)  |    \r\n\
        >   <       |  |     |   __|  |      /     |  |\\/|  | |  | |  . `  |   /  /_\\  \\    |  |     |  |  |  | |      /     \r\n\
       /  .  \\      |  |     |  |____ |  |\\  \\----.|  |  |  | |  | |  |\\   |  /  _____  \\   |  |     |  `--'  | |  |\\  \\----.\r\n\
      /__/ \\__\\     |__|     |_______|| _| `._____||__|  |__| |__| |__| \\__| /__/     \\__\\  |__|      \\______/  | _| `._____|\r\n\
                                                                                                                             \r\n\
\r\n"));

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tUpdateSpaceXTerminator);

    tUpdateSpaceXTerminator.enable();
    tDetectButtonChange.enable();
}
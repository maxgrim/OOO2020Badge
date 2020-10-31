#include "RasterDungeon.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../RGB.h"
#include "../FlagCrypto.h"
#include "../Definitions.h"

#include <Arduino.h>

static void (*doneCallbackF)();

static uint8_t posX = 5, posY = 15;
static int8_t currentDirection = 0;
static bool readingL, readingR, lastStateL, lastStateR;
static const char characters[4] = {' ', 'X', '+', 'V'};

static const uint32_t colorOff = 0;
static const uint32_t colorDisabled = 0xFF0000;
static const uint32_t colorBase = 0xFFFF00;
static const uint32_t colorDirection = 0x00FF00;
static const uint32_t colorDead = 0xFF0000;

static const uint8_t dungeon[20][20] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 2, 1, 1, 2, 0, 1, 2, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 0, 0, 0, 2, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 1, 2, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 0, 1, 0, 0, 2, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 0, 0, 1, 2, 0, 2, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 2, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 2, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 2, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 2, 1, 0, 0, 0, 2, 1, 3, 0, 0, 0, 1, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

static void detectButtonChange();
static Task tDetectButtonChange(0, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(0, TASK_FOREVER, &verifyButtonsLow);

static void playWinAnimation();
static Task tPlayWinAnimation(200, 18, &playWinAnimation);

static void playDeadAnimation();
static Task tPlayDeadAnimation(200, 7, &playDeadAnimation);

static void playWinAnimation()
{
    if (tPlayWinAnimation.isFirstIteration())
    {
        eyesOff();
        rgbSetBrightness(10);
    }

    if (tPlayWinAnimation.isLastIteration())
    {
        doneCallbackF();
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

static void playDeadAnimation()
{
    if (tPlayDeadAnimation.isFirstIteration())
    {
        rgbSetBrightness(10);
    }

    if (tPlayDeadAnimation.isLastIteration())
    {
        doneCallbackF();
        return;
    }

    unsigned long counter = tPlayDeadAnimation.getRunCounter();
    if (counter % 2 == 1)
    {
        eyesOff();
        rgbClear();
        rgbShow();
    }
    else
    {
        eyesOn();
        rgbSetAllLeds(colorDead);
        rgbShow();
    }
}

static void deactivateRasterDungeon()
{
    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
}

static void printDungeon()
{
    for (char y = 0; y < 20; y++)
    {
        for (char x = 0; x < 20; x++)
        {
            if (x == posX && y == posY)
            {
                Serial.print("<>");
            }
            else
            {
                Serial.printf("%c", characters[dungeon[y][x]]);
                Serial.printf("%c", characters[dungeon[y][x]]);
            }
        }

        Serial.printf("\r\n");
    }
}

uint8_t getNextY(uint8_t posY, int8_t direction)
{
    uint8_t nextPosY = posY;
    switch (direction)
    {
    case 0:
        // south
        nextPosY++;
        break;
    case 2:
        // north
        nextPosY--;
        break;
    }

    return nextPosY;
}

uint8_t getNextX(uint8_t posX, int8_t direction)
{
    uint8_t nextPosX = posX;
    switch (direction)
    {
    case 1:
        // west
        nextPosX--;
        break;
    case 3:
        // east
        nextPosX++;
        break;
    }

    return nextPosX;
}

int8_t getNextDirection(int8_t currentDirection, uint8_t posX, uint8_t posY, bool clockWise)
{
    for (int i = 0; i <= 3; i++)
    {
        if (clockWise)
        {
            currentDirection++;
            currentDirection %= 4;
        }
        else
        {
            currentDirection--;
            if (currentDirection < 0)
            {
                currentDirection = 3;
            }
        }

        uint8_t nextPosX = getNextX(posX, currentDirection);
        uint8_t nextPosY = getNextY(posY, currentDirection);

        Serial.printf("%c at x: %d, y: %d\r\n", characters[dungeon[nextPosY][nextPosX]], nextPosX, nextPosY);
        if (dungeon[nextPosY][nextPosX] != 1)
        {
            break;
        }
    }

    return currentDirection;
}

static void updateRgbLeds()
{
    rgbClear();
    rgbSetBrightness(10);

    rgbSetSingleLed(0, (dungeon[posY][posX + 1] == 1) ? colorDisabled : colorOff);
    rgbSetSingleLed(1, colorBase);
    rgbSetSingleLed(2, colorBase);
    rgbSetSingleLed(3, (dungeon[posY + 1][posX] == 1) ? colorDisabled : colorOff);
    rgbSetSingleLed(4, colorBase);
    rgbSetSingleLed(5, colorBase);
    rgbSetSingleLed(6, (dungeon[posY][posX - 1] == 1) ? colorDisabled : colorOff);
    rgbSetSingleLed(7, colorBase);
    rgbSetSingleLed(8, colorBase);
    rgbSetSingleLed(9, (dungeon[posY - 1][posX] == 1) ? colorDisabled : colorOff);
    rgbSetSingleLed(10, colorBase);
    rgbSetSingleLed(11, colorBase);

    // Overwrite 1 LED with currentDirection indicator
    rgbSetSingleLed(((currentDirection * 3) + 6) % 12, colorDirection);

    rgbShow();

    Serial.printf("My position x: %d, y: %d\r\n", posX, posY);
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
        posX = getNextX(posX, currentDirection);
        posY = getNextY(posY, currentDirection);

        printDungeon();

        // Handle dying
        switch (dungeon[posY][posX])
        {
        case 2:
            // Dead
            Serial.printf("Oh no, you ran into a monster. Dead!\r\n");
            deactivateRasterDungeon();
            badgeTaskScheduler.addTask(tPlayDeadAnimation);
            tPlayDeadAnimation.enable();
            return;
        case 3:
            char *encryptedFlag = "ElDz3zZlWVmLHJwwIY92a37WOZqP8/gneZYW+16C+8zR8hTAesWvV96GSmxGrwaH";
            char destination[strlen(encryptedFlag)];
            cryptoGetFlagAES(encryptedFlag, destination);
            Serial.printf("You've escaped the dungeon! This is for you: %s\r\n", destination);

            deactivateRasterDungeon();
            badgeTaskScheduler.addTask(tPlayWinAnimation);
            tPlayWinAnimation.enable();
        }

        // Check if the direction is still valid
        uint8_t nextPosX = getNextX(posX, currentDirection);
        uint8_t nextPosY = getNextY(posY, currentDirection);

        if (dungeon[nextPosY][nextPosX] == 1)
        {
            currentDirection = getNextDirection(currentDirection, posX, posY, true);
        }

        updateRgbLeds();
    }
    else if (readingL || readingR)
    {
        currentDirection = getNextDirection(currentDirection, posX, posY, readingR);
        updateRgbLeds();
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

void rasterDungeonSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    printDungeon();
    updateRgbLeds();

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);

    tDetectButtonChange.enable();
}
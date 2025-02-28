#include "SpaceMaze.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../RGB.h"
#include "../FlagCrypto.h"
#include "../Definitions.h"

#include <Arduino.h>

static void (*doneCallbackF)();

static uint8_t originalPosX = 5, originalPosy = 15;
static uint8_t posX = 5, posY = 15;
static int8_t currentDirection = 0;
static bool readingL, readingR, lastStateL, lastStateR;
static const char characters[4] = {' ', 'X', '+', 'V'};

static const uint32_t colorOpen = 0;
static const uint32_t colorWall = 0xFF0000;
static const uint32_t colorDirection = 0x0080FF;

static const uint32_t colorEffectBase = 0xFFFF00;
static const uint32_t colorEffectDead = 0xFF0000;

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
static Task tDetectButtonChange(TASK_IMMEDIATE, TASK_FOREVER, &detectButtonChange);

static void verifyButtonChange();
static Task tVerifyButtonChange(DEBOUNCE_BUTTON, TASK_ONCE, &verifyButtonChange);

static void verifyButtonsLow();
static Task tVerifyButtonsLow(TASK_IMMEDIATE, TASK_FOREVER, &verifyButtonsLow);

static void playWinAnimation();
static Task tPlayWinAnimation(200, 18, &playWinAnimation);

static void playDeadAnimation();
static Task tPlayDeadAnimation(200, 7, &playDeadAnimation);

static void showDungeonAnimation();
static Task tShowDungeonAnimation(25, TASK_FOREVER, &showDungeonAnimation);

static uint8_t effectMaxBrightness = 25;
static uint8_t effectMinBrightness = 1;
static uint16_t effectCurrentBrightness = 0;
static int8_t effectBrightnessModifier = 1;

static const uint32_t colorActive = 0x0080FF;
static const uint32_t colorEffect = 0x00FF00;

static void showDungeonAnimation()
{
    // Only modify brightness every 2nd execution
    if (tShowDungeonAnimation.getRunCounter() % 2 == 1)
    {
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
    }

    rgbSetSingleLed(0, (dungeon[posY - 1][posX] == 1) ? colorWall : colorOpen);
    rgbSetSingleLed(3, (dungeon[posY][posX + 1] == 1) ? colorWall : colorOpen);
    rgbSetSingleLed(6, (dungeon[posY + 1][posX] == 1) ? colorWall : colorOpen);
    rgbSetSingleLed(9, (dungeon[posY][posX - 1] == 1) ? colorWall : colorOpen);

    rgbSetSingleLed(((currentDirection * 3) + 6) % 12, colorDirection);

    rgbSetSingleLed(1, colorEffectBase);
    rgbSetSingleLed(2, colorEffectBase);
    rgbSetSingleLed(4, colorEffectBase);
    rgbSetSingleLed(5, colorEffectBase);
    rgbSetSingleLed(7, colorEffectBase);
    rgbSetSingleLed(8, colorEffectBase);
    rgbSetSingleLed(10, colorEffectBase);
    rgbSetSingleLed(11, colorEffectBase);

    rgbShow();
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

static void playDeadAnimation()
{
    if (tPlayDeadAnimation.isFirstIteration())
    {
        rgbSetBrightness(RGB_DEFAULT_BRIGHTNESS);
    }

    if (tPlayDeadAnimation.isLastIteration())
    {
        if (doneCallbackF != NULL)
        {
            doneCallbackF();
        }
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
        rgbSetAllLeds(colorEffectDead);
        rgbShow();
    }
}

static void deactivateRasterDungeon()
{
    posX = originalPosX;
    posY = originalPosy;

    badgeTaskScheduler.deleteTask(tDetectButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonChange);
    badgeTaskScheduler.deleteTask(tVerifyButtonsLow);
    badgeTaskScheduler.deleteTask(tShowDungeonAnimation);
}

static void printMaze()
{
#ifdef BADGE_DEBUG
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
#endif
}

uint8_t getNextY(uint8_t posY, int8_t direction)
{
    uint8_t nextPosY = posY;
    switch (direction)
    {
    case 0:
        nextPosY++;
        break;
    case 2:
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
        nextPosX--;
        break;
    case 3:
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

        if (dungeon[nextPosY][nextPosX] != 1)
        {
            break;
        }
    }

    return currentDirection;
}

static void updateRgbLeds()
{
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

static void handleMove()
{
    posX = getNextX(posX, currentDirection);
    posY = getNextY(posY, currentDirection);

    printMaze();

    switch (dungeon[posY][posX])
    {
    case 2:
        Serial.printf("Oh no, you hit an asteroid!\r\n");
        deactivateRasterDungeon();
        badgeTaskScheduler.addTask(tPlayDeadAnimation);
        tPlayDeadAnimation.restart();
        return;
    case 3:
        const char *encryptedFlag = "\x12\x50\xf3\xdf\x36\x65\x59\x59\x8b\x1c\x9c\x30\x21\x8f\x76\x6b\x7e\xd6\x39\x9a\x8f\xf3\xf8\x27\x79\x96\x16\xfb\x5e\x82\xfb\xcc\xd1\xf2\x14\xc0\x7a\xc5\xaf\x57\xde\x86\x4a\x6c\x46\xaf\x06\x87";
        const uint8_t aesKey[AES_BLOCK_SIZE] = {0xa3, 0x18, 0xe9, 0x26, 0x9a, 0x14, 0xdd, 0x91, 0x63, 0xe8, 0x25, 0x51, 0x75, 0x1b, 0x8f, 0x0b};
        const uint8_t aesIV[AES_BLOCK_SIZE] = {0x58, 0x12, 0xa9, 0x24, 0x46, 0xdc, 0x64, 0x76, 0x51, 0x42, 0x46, 0x1c, 0xf8, 0x1d, 0x54, 0x41};
        char destination[strlen(encryptedFlag)];

        cryptoGetFlagAES(aesKey, aesIV, encryptedFlag, destination);
        Serial.printf("Welcome home, son! This is for you %s\r\n", destination);

        deactivateRasterDungeon();
        badgeTaskScheduler.addTask(tPlayWinAnimation);
        tPlayWinAnimation.restart();
        return;
    }

    uint8_t nextPosX = getNextX(posX, currentDirection);
    uint8_t nextPosY = getNextY(posY, currentDirection);

    if (dungeon[nextPosY][nextPosX] == 1)
    {
        currentDirection = getNextDirection(currentDirection, posX, posY, true);
    }

    updateRgbLeds();
    tVerifyButtonsLow.enable();
    tShowDungeonAnimation.enable();
}

static void verifyButtonChange()
{
    tDetectButtonChange.disable();

    if (readingL && readingR)
    {
        tShowDungeonAnimation.disable();
        rgbBlinkSingleLed(((currentDirection * 3) + 6) % 12, 2, colorDirection, &handleMove);
    }
    else if (readingL || readingR)
    {
        currentDirection = getNextDirection(currentDirection, posX, posY, readingR);
        updateRgbLeds();
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

void spaceMazeSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    Serial.print(F("\r\n\
________                               ______  ___                  \r\n\
__  ___/_____________ ___________      ___   |/  /_____ ___________ \r\n\
_____ \\___  __ \\  __ `/  ___/  _ \\     __  /|_/ /_  __ `/__  /_  _ \r\n\
____/ /__  /_/ / /_/ // /__ /  __/     _  /  / / / /_/ /__  /_/  __/\r\n\
/____/ _  .___/\\__,_/ \\___/ \\___/      /_/  /_/  \\__,_/ _____/\\___/ \r\n\
       /_/                                                          \r\n\
\r\n\r\n"));

    printMaze();
    updateRgbLeds();

    Serial.printf("You are at x: %d, y: %d\r\n", posX, posY);

    badgeTaskScheduler.addTask(tDetectButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonChange);
    badgeTaskScheduler.addTask(tVerifyButtonsLow);
    badgeTaskScheduler.addTask(tShowDungeonAnimation);

    tDetectButtonChange.enable();
    tShowDungeonAnimation.enable();
}
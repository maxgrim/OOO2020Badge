#include "Menu.h"
#include "RGB.h"
#include "Definitions.h"
#include "Debug.h"
#include "Badge.h"

#include <Arduino.h>

#define MENU_POSITIONS 12

int currentMenuPosition = 0;
bool readingL, readingR, lastStateL, lastStateR;
static void handleButtons();

static Task tHandleButtons(DEBOUNCE_BUTTON, TASK_ONCE, &handleButtons);

static int convertToLedNr(int led)
{
    return (led + 9) % 12;
}

static void moveToMenuPosition(int position)
{
    rgbSetSingleLed(convertToLedNr(position), 0xFFFFFF);
}

static void activateMenu()
{
    moveToMenuPosition(currentMenuPosition);
    badgeTaskScheduler.addTask(tHandleButtons);
}

static void deactivateMenu()
{
    badgeTaskScheduler.deleteTask(tHandleButtons);
}

static void moveMenuLeft()
{
    currentMenuPosition--;
    if (currentMenuPosition < 0)
    {
        currentMenuPosition = 11;
    }

    moveToMenuPosition(currentMenuPosition);
}

static void moveMenuRight()
{
    currentMenuPosition++;
    if (currentMenuPosition > 11)
    {
        currentMenuPosition = 0;
    }

    moveToMenuPosition(currentMenuPosition);
}

static void confirmCurrentMenuPosition()
{
    deactivateMenu();
    rgbBlinkSingleLed(convertToLedNr(currentMenuPosition), 3, 0xFF0000, &activateMenu);
}

static void handleButtons()
{
    if (readingL && readingR)
    {
        confirmCurrentMenuPosition();
        DEBUG_PRINTLN("Menu: confirm");
    }
    else if (readingL)
    {
        moveMenuLeft();
        DEBUG_PRINTF("Menu: left button pressed, moved to position %d\r\n", currentMenuPosition);
    }
    else if (readingR)
    {
        moveMenuRight();
        DEBUG_PRINTF("Menu: right button pressed, moved to position %d\r\n", currentMenuPosition);
    }
}

void menuSetup()
{
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    activateMenu();
}

void menuLoop()
{
    // Button handling
    readingL = digitalRead(PIN_BUTTON_L);
    readingR = digitalRead(PIN_BUTTON_R);

    if (readingL != lastStateL || readingR != lastStateR)
    {
        tHandleButtons.restartDelayed();
    }

    lastStateL = readingL;
    lastStateR = readingR;
}
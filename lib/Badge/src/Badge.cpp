#include <Arduino.h>
#include <EEPROM.h>

#include "Badge.h"
#include "Debug.h"
#include "FlagCrypto.h"
#include "Definitions.h"
#include "Eyes.h"
#include "Menu.h"
#include "RGB.h"
#include "SerialPrompt.h"
#include "Settings.h"
#include "Webserver.h"
#include "WiFi.h"

#include <TaskScheduler.h>

Scheduler badgeTaskScheduler;

static bool buttonsPressed()
{
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    return digitalRead(PIN_BUTTON_L) && digitalRead(PIN_BUTTON_R);
}

static void printWelcome()
{
    Serial.println(F("\r\n          .     .  .      +     .      .          .\r\n     .       .      .     #       .           .\r\n        .      .         ###            .      .      .\r\n      .      .   \"#:. .:##\"##:. .:#\"  .      .\r\n          .      . \"####\"###\"####\"  .\r\n       .     \"#:.    .:#\"###\"#:.    .:#\"  .        .       .\r\n  .             \"#########\"#########\"        .        .\r\n        .    \"#:.  \"####\"###\"####\"  .:#\"   .       .\r\n     .     .  \"#######\"\"##\"##\"\"#######\"                  .\r\n                .\"##\"#####\"#####\"##\"           .      .\r\n    .   \"#:. ...  .:##\"###\"###\"##:.  ... .:#\"     .\r\n      .     \"#######\"##\"#####\"##\"#######\"      .     .\r\n    .    .     \"#####\"\"#######\"\"#####\"    .      .    _==_ _\r\n            .     \"      000      \"    .     .      _,(\\\",)|_|\r\n       .         .   .   000     .        .          \\/. \\-|\r\n.. .. ..................O000O........................( :  )|...."));
    Serial.println(F("\r\n   ____  ________                _             \r\n  / __ \\/ __/ __/__  ____  _____(_)   _____     \r\n / / / / /_/ /_/ _ \\/ __ \\/ ___/ / | / / _ \\    \r\n/ /_/ / __/ __/  __/ / / (__  ) /| |/ /  __/    \r\n\\____/_/ /_/__\\___/_/ /_/____/_/ |___/\\___/     \r\n  / __ \\/ __/ __/____(_) /____                  \r\n / / / / /_/ /_/ ___/ / __/ _ \\                 \r\n/ /_/ / __/ __(__  ) / /_/  __/                 \r\n\\____/_/ /_/ /____/_/\\__/\\___/ __  _            \r\n  / __ \\____  ___  _________ _/ /_(_)___  ____  \r\n / / / / __ \\/ _ \\/ ___/ __ `/ __/ / __ \\/ __ \\ \r\n/ /_/ / /_/ /  __/ /  / /_/ / /_/ / /_/ / / / / \r\n\\____/ .___/\\___/_/   \\__,_/\\__/_/\\____/_/ /_/  \r\n   _/_/ ____ ___  ____                         \r\n  |__ \\/ __ \\__ \\/ __ \\                         \r\n  __/ / / / /_/ / / / /                         \r\n / __/ /_/ / __/ /_/ /                         \r\n/____|____/____|____/         Winter Edition    \r\n                                               \r\n"));
}

static void printEasyFlag()
{
    char flag[38];
    cryptoGetFlag(&flag[0], sizeof(flag), 64);

    Serial.print(F("Here is a flag for you young serial Jedi: "));
    Serial.printf("%s\r\n", flag);
}

void badgeFatalError(const char *message)
{
    Serial.println(message);
    while (true)
    {
        yield();
    };
}

bool badgeSetup(size_t eepromSize, size_t eepromStartAddress)
{
    eyesSetup();

    // Setup serial output
    Serial.begin(115200);

    // Setup settings
    settingsSetup(eepromSize, eepromStartAddress);
    Settings settings = settingsGetSettings();

    // Badge mode code
    if (buttonsPressed())
    {
        settings.badgeMode = settings.badgeMode == 0 ? 255 : 0;
        settingsSetSettings(settings);
        DEBUG_PRINTLN(F("Buttons pressed, switching modes"));
    }

    if (settings.badgeMode == 255)
    {
        eyesOff();
        Serial.flush();
        return false;
    }

    eyesOn();

    // Setup RGB
    rgbSetup();

    // Setup menu
    menuSetup();

    // WiFi
    wifiSetup();

    // Print out the banner and the first flag over serial
    printWelcome();
    printEasyFlag();

    // Serial prompt
    serialPromptSetup();

    return true;
}

void badgeLoop()
{
    serialPromptLoop();

    // TODO: when menu or serialPrompt is executed, it should disable the other..
    badgeTaskScheduler.execute();
}
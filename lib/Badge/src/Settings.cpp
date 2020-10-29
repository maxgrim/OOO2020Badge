#include <Arduino.h>

#include "Settings.h"
#include "Badge.h"
#include "Definitions.h"
#include <EEPROM.h>

int startAddress;

void settingsSetup(size_t eepromSize, size_t eepromStartAddress)
{
    EEPROM.begin(eepromSize);
    startAddress = eepromStartAddress;
}

Settings settingsGetSettings()
{
    Settings settings = {};
    EEPROM.get(2900, settings);
    return settings;
}

void settingsSetSettings(Settings settings)
{
    EEPROM.put(2900, settings);
    EEPROM.commit();
}
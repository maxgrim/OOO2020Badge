#include <Arduino.h>

#include "Settings.h"
#include "Badge.h"
#include "Definitions.h"
#include <EEPROM.h>

size_t startAddress;

void settingsSetup(size_t eepromSize, size_t eepromStartAddress)
{
    EEPROM.begin(eepromSize);
    startAddress = eepromStartAddress;
}

Settings settingsGetSettings()
{
    Settings settings = {};
    EEPROM.get(startAddress, settings);
    return settings;
}

void settingsSetSettings(Settings settings)
{
    EEPROM.put(startAddress, settings);
    EEPROM.commit();
}
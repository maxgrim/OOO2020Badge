#pragma once

#include "wled.h"
#include "FX.h"

#define EEPROM_DEFAULTS_SET 123

struct Settings
{
  uint8_t defaultsSet;
};

class ConfigureDefaultsBadgeUsermod : public Usermod
{
public:
  void setup()
  {
    Settings settings = {};
    EEPROM.get(2800, settings);

    if (settings.defaultsSet != EEPROM_DEFAULTS_SET)
    {
      settings.defaultsSet = EEPROM_DEFAULTS_SET;
      EEPROM.put(2800, settings);

      // Christmas effect
      bootPreset = 1;
      effectCurrent = FX_MODE_MERRY_CHRISTMAS;
      bri = 20;
      effectIntensity = 32;
      effectSpeed = 64;

      saveSettingsToEEPROM();
      savePreset(1, true);
      ESP.restart();
    }
  }

  void connected() {}
  void loop() {}
};
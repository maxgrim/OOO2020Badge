#pragma once

#include "wled.h"
#include "FX.h"

#define EEPROM_DEFAULTS_SET 123
#define PIN_BUTTON_L 12
#define PIN_BUTTON_R 16
#define MAX_PRESETS 16
#define DEFAULT_BRIGHTNESS 20

unsigned long lastDebounceTime = 0;                 // the last time the output pin was toggled
unsigned long debounceDelay = 50;                   // the debounce time; increase if the output flickers
int buttonStateL, buttonStateR;                     // the current reading from the input pin
int lastButtonStateL = LOW, lastButtonStateR = LOW; // the previous reading from the input pin
int buttonCurrentPreset = 1;

struct Settings
{
  uint8_t defaultsSet;
};

class OOO2020BadgeUsermod : public Usermod
{
public:
  void setup()
  {
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);

    Settings settings = {};
    EEPROM.get(2800, settings);

    if (settings.defaultsSet != EEPROM_DEFAULTS_SET)
    {
      settings.defaultsSet = EEPROM_DEFAULTS_SET;
      EEPROM.put(2800, settings);
      bootPreset = 1;
      bri = DEFAULT_BRIGHTNESS;
      saveSettingsToEEPROM();

      WS2812FX::Segment &seg = strip.getSegment(0);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_MERRY_CHRISTMAS;
      effectIntensity = 64;
      effectSpeed = 48;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(1, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_MERRY_CHRISTMAS;
      effectIntensity = 32;
      effectSpeed = 48;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(2, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_MERRY_CHRISTMAS;
      effectIntensity = 16;
      effectSpeed = 48;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(3, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_MERRY_CHRISTMAS;
      effectIntensity = 0;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(4, true);

      col[0] = 255;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_STATIC;
      effectIntensity = 0;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(5, true);

      col[0] = 0;
      col[1] = 255;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_STATIC;
      effectIntensity = 0;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(6, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 255;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_STATIC;
      effectIntensity = 0;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(7, true);

      col[0] = 255;
      col[1] = 255;
      col[2] = 255;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_STATIC;
      effectIntensity = 0;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(8, true);

      col[0] = 255;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_SINELON_DUAL;
      effectIntensity = 128;
      effectSpeed = 128;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(9, true);

      col[0] = 255;
      col[1] = 200;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_CANDLE;
      effectIntensity = 192;
      effectSpeed = 8;
      bri = DEFAULT_BRIGHTNESS * 3;
      savePreset(10, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_SOLID_GLITTER;
      effectIntensity = 12;
      effectSpeed = 0;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(11, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_DISSOLVE_RANDOM;
      effectIntensity = 64;
      effectSpeed = 192;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(12, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_PALETTE;
      effectIntensity = 0;
      effectSpeed = 32;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(13, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_RAINBOW_CYCLE;
      effectIntensity = 8;
      effectSpeed = 12;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(14, true);

      col[0] = 0;
      col[1] = 0;
      col[2] = 0;
      colSec[0] = 0;
      colSec[1] = 0;
      colSec[2] = 0;
      effectCurrent = FX_MODE_RAINBOW_CYCLE;
      effectIntensity = 0;
      effectSpeed = 192;
      bri = DEFAULT_BRIGHTNESS;
      savePreset(15, true);

      ESP.restart();
    }
  }

  void connected() {}

  bool presetUp()
  {
    int nextPreset = ++buttonCurrentPreset;

    if (nextPreset > MAX_PRESETS)
    {
      buttonCurrentPreset = nextPreset = 1;
    }

    Serial.println(nextPreset);

    if (applyPreset(nextPreset))
    {
      colorUpdated(NOTIFIER_CALL_MODE_FX_CHANGED);
      return true;
    }

    return false;
  }

  bool presetDown()
  {
    int nextPreset = --buttonCurrentPreset;

    if (nextPreset < 1)
    {
      buttonCurrentPreset = nextPreset = MAX_PRESETS;
    }

    Serial.println(nextPreset);

    if (applyPreset(nextPreset))
    {
      colorUpdated(NOTIFIER_CALL_MODE_FX_CHANGED);
      return true;
    }

    return false;
  }

  void loop()
  {
    int readingL = digitalRead(PIN_BUTTON_L);
    int readingR = digitalRead(PIN_BUTTON_R);

    if (readingL != lastButtonStateL || readingR != lastButtonStateR)
    {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      if (readingL != buttonStateL)
      {
        buttonStateL = readingL;

        if (readingL)
        {
          for (int i = 0; i < MAX_PRESETS; i++)
          {
            if (presetDown())
            {
              break;
            }
          }
        }
      }

      if (readingR != buttonStateR)
      {
        buttonStateR = readingR;

        if (readingR)
        {
          for (int i = 0; i < MAX_PRESETS; i++)
          {
            if (presetUp())
            {
              break;
            }
          }
        }
      }
    }

    lastButtonStateL = readingL;
    lastButtonStateR = readingR;
  }
};

#pragma once

#include "wled.h"
#include "FX.h"

#define EEPROM_DEFAULTS_SET 123
#define PIN_BUTTON_L 12
#define PIN_BUTTON_R 16
#define MAX_PRESETS 16

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
unsigned long debounceDelay = 50;   // the debounce time; increase if the output flickers
int buttonState;                    // the current reading from the input pin
int lastButtonState = LOW;          // the previous reading from the input pin

struct Settings
{
  uint8_t defaultsSet;
};

class OOO2020BadgeUsermod : public Usermod
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
      effectSpeed = 48;

      saveSettingsToEEPROM();
      savePreset(1, true);
      ESP.restart();
    }
  }

  void connected()
  {
    pinMode(PIN_BUTTON_L, INPUT);
    pinMode(PIN_BUTTON_R, INPUT);
  }

  void loop()
  {
    // read the state of the switch into a local variable:
    int reading = digitalRead(PIN_BUTTON_R);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState)
    {
      // reset the debouncing timer
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
      // whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      // if the button state has changed:
      if (reading != buttonState)
      {
        buttonState = reading;

        // only toggle the LED if the new button state is HIGH
        if (buttonState == HIGH)
        {
          Serial.println("Button pressed!");

          for (int i = 1; i <= MAX_PRESETS; i++)
          {
            int nextPreset = currentPreset == -1 ? 1 : ((currentPreset + i) % MAX_PRESETS) + 1;

            Serial.print("Current: ");
            Serial.println(currentPreset);

            Serial.print("Next: ");
            Serial.println(nextPreset);

            if (applyPreset(nextPreset))
            {
              Serial.println("Preset exists!");
              colorUpdated(NOTIFIER_CALL_MODE_FX_CHANGED);
              break;
            }
            else
            {
              Serial.println("Preset does not exist!");
            }
          }
        }
      }
    }

    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState = reading;
  }
};
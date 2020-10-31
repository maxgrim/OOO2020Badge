#include "Eyes.h"
#include "Definitions.h"

#include <Arduino.h>

void eyesOn() {
    digitalWrite(PIN_LED_EYES, HIGH);
}

void eyesOff() {
    digitalWrite(PIN_LED_EYES, LOW);
}

void eyesSetup() {
    pinMode(PIN_LED_EYES, OUTPUT);
}

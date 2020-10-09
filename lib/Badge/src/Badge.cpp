#include <Arduino.h>

void badgeSetup() {
    Serial.begin(115200);
    Serial.println("Badge");
}
#include "WiFi.h"

#include <ESP8266WiFi.h>

void wifiSetup() {
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}
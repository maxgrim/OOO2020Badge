#include "WiFi.h"

void wifiOff()
{
    WiFi.persistent(false);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}

void wifiOn(IPAddress localIp, IPAddress gateway, IPAddress subnet, const char *ssid, const char *password)
{
    WiFi.mode(WIFI_AP);
    Serial.print(F("\r\nSetting up "));
    Serial.print(ssid);
    Serial.println(F("..."));

    if (!WiFi.softAPConfig(localIp, gateway, subnet))
    {
        Serial.println(F("AP configuration failed !"));
    }

    if (!WiFi.softAP(ssid, password))
    {
        Serial.println(F("AP failed!"));
    }

    Serial.print(F("IP address: "));
    Serial.println(WiFi.softAPIP().toString().c_str());
}
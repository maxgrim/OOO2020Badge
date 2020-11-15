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
    Serial.printf("Setting up %s ...\r\n", ssid);

    if (!WiFi.softAPConfig(localIp, gateway, subnet))
    {
        Serial.printf("AP configuration failed!\r\n");
    }

    if(!WiFi.softAP(ssid, password)) {
        Serial.printf("AP failed!\r\n");
    }

    Serial.printf("IP address: %s\r\n", WiFi.softAPIP().toString().c_str());
}
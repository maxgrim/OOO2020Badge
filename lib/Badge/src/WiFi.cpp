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
    Serial.printf("Setting soft-AP ...\r\n");
    Serial.printf(WiFi.softAPConfig(localIp, gateway, subnet) ? "AP configured\r\n" : "AP configuration failed!\r\n");
    Serial.printf(WiFi.softAP(ssid, password) ? "AP ready\r\n" : "AP failed!\r\n");
    Serial.printf("IP address: %s\r\n", WiFi.softAPIP().toString().c_str());
}
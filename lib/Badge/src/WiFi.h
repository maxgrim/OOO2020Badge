#ifndef WIFI_H_SEEN
#define WIFI_H_SEEN

#include <ESP8266WiFi.h>

void wifiOff();
void wifiOn(IPAddress localIp, IPAddress gateway, IPAddress subnet, const char *ssid, const char *password);

#endif
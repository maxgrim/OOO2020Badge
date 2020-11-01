#include "Webserver.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../RGB.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

static ESP8266WebServer server(80);

static void handleServerClient();
static Task tHandleServerClient(TASK_IMMEDIATE, TASK_FOREVER, &handleServerClient);

IPAddress local_IP(1, 3, 3, 7);
IPAddress gateway(1, 3, 3, 7);
IPAddress subnet(255, 255, 255, 0);

void handleServerClient() {
    server.handleClient();
}

void handleRoot()
{
    server.send(200, "text/plain", "hello from esp8266!");
}

// Still have to determine when to leave this mode...
static void deactivateWebserver()
{
    badgeTaskScheduler.deleteTask(tHandleServerClient);
}

void webserverSetup(void (*doneCallback)())
{
    Serial.printf("webserverSetup\r\n");
    
    eyesOff();
    rgbClear();

    WiFi.mode(WIFI_AP);
      Serial.print("Setting soft-AP configuration ... ");
    Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

    Serial.print("Setting soft-AP ... ");
    Serial.println(WiFi.softAP("BadgeAP") ? "Ready" : "Failed!");

    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.begin();

    badgeTaskScheduler.addTask(tHandleServerClient);
    tHandleServerClient.enable();
}
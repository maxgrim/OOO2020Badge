#include "Webserver.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../RGB.h"
#include "../FlagCrypto.h"
#include "../WiFi.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

static void (*doneCallbackF)();

static ESP8266WebServer server(80);

static void handleServerClient();
static Task tHandleServerClient(TASK_IMMEDIATE, TASK_FOREVER, &handleServerClient);

static char *password = "SuperSecretWiFiPassword";

IPAddress localIp(1, 3, 3, 7);
IPAddress gateway(1, 3, 3, 7);
IPAddress subnet(255, 255, 255, 0);

static void deactivateWebserver()
{
    badgeTaskScheduler.deleteTask(tHandleServerClient);
    wifiOff();
    doneCallbackF();
}

static void handleServerClient()
{
    server.handleClient();
}

static void handleRoot()
{
    server.send(200, "text/plain", "Hi!");
}

static void handleSecretPage()
{
    char flag[38];
    cryptoGetFlag(&flag[0], sizeof(flag), 32);
    server.send(200, "text/plain", flag);
}

static void handleStop() {
    deactivateWebserver();
}

void webserverSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    rgbClear();
    rgbShow();

    wifiOn(localIp, gateway, subnet, "StarshipAP", password);

    server.on("/", handleRoot);
    server.on("/youwillneverguessthis", handleSecretPage);
    server.on("/stop", handleStop);
    server.begin();

    badgeTaskScheduler.addTask(tHandleServerClient);
    tHandleServerClient.enable();
}
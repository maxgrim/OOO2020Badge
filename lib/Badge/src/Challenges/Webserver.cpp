#include "Webserver.h"

#include "../Badge.h"
#include "../Eyes.h"
#include "../RGB.h"
#include "../FlagCrypto.h"
#include "../WiFi.h"

#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

static void (*doneCallbackF)();

static ESP8266WebServer server(80);

static void handleServerClient();
static Task tHandleServerClient(TASK_IMMEDIATE, TASK_FOREVER, &handleServerClient);

static char *password = "SuperSecretWiFiPassword";
static const char TEXT_PLAIN[] PROGMEM = "text/plain";

IPAddress localIp(1, 3, 3, 7);
IPAddress gateway(1, 3, 3, 7);
IPAddress subnet(255, 255, 255, 0);

static DNSServer dnsServer;

static void deactivateWebserver()
{
    badgeTaskScheduler.deleteTask(tHandleServerClient);
    wifiOff();

    if (doneCallbackF != NULL)
    {
        doneCallbackF();
    }
}

static void handleServerClient()
{
    dnsServer.processNextRequest();
    server.handleClient();
}

static void handleStop()
{
    deactivateWebserver();
}

void replyNotFound(String msg)
{
    server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void handleNotFound()
{
    String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

    String message;
    message.reserve(100);
    message = F("Error: File not found\n\nURI: ");
    message += uri;
    message += F("\nMethod: ");
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += server.args();
    message += '\n';

    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += F(" NAME:");
        message += server.argName(i);
        message += F("\n VALUE:");
        message += server.arg(i);
        message += '\n';
    }

    message += "path=";
    message += server.arg("path");
    message += '\n';

    return replyNotFound(message);
}

void webserverSetup(void (*doneCallback)())
{
    doneCallbackF = doneCallback;

    if (!SPIFFS.begin())
    {
        Serial.println(F("An error has occurred while mounting SPIFFS"));
        return;
    }

    rgbClear();
    rgbShow();

    wifiOn(localIp, gateway, subnet, "StarshipAP", password);

    /* Setup the DNS server redirecting all the domains to the localIp */
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", localIp);

    server.on("/stop", handleStop);
    server.serveStatic("/", SPIFFS, "/index.html", "max-age=31536000,public");
    server.serveStatic("/s", SPIFFS, "/s", "max-age=31536000,public");

    server.onNotFound(handleNotFound);

    server.begin();

    badgeTaskScheduler.addTask(tHandleServerClient);
    tHandleServerClient.enable();
}
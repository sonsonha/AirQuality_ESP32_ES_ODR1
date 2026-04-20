#include "TaskServer.h"
#include "../common/log.h"

static const char *TAG = "SERVER";

AsyncWebServer server(httpPort);
AsyncWebSocket ws("/ws");

bool isServerRunning = false;

static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                    AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        LOG_I(TAG, "WS client #%u connected from %s", client->id(), client->remoteIP().toString().c_str());
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        LOG_I(TAG, "WS client #%u disconnected", client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_TEXT)
        {
            String message;
            message += String((char *)data).substring(0, len);
            parseJson(message, true);
        }
    }
}

void connectWSV(void)
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });
    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/script.js", "application/javascript"); });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles.css", "text/css"); });
    server.begin();
    isServerRunning = true;
    LOG_I(TAG, "HTTP + WebSocket server started on port %d", httpPort);
}

void stopServer(void)
{
    if (isServerRunning)
    {
        ws.closeAll();
        server.end();
        isServerRunning = false;
        LOG_W(TAG, "Server stopped");
    }
}

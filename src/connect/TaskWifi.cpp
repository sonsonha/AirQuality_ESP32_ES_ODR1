#include "TaskWifi.h"
#include "TaskMQTT.h"
#include "TaskServer.h"
#include "../common/sensor_data.h"
#include "../common/log.h"
#include <math.h>

static const char *TAG = "CONNECT";

#define WIFI_BACKOFF_MIN_MS     1000
#define WIFI_BACKOFF_MAX_MS     60000
#define MQTT_BACKOFF_MIN_MS     2000
#define MQTT_BACKOFF_MAX_MS     30000
#define QUEUE_POLL_TIMEOUT_MS   500

static bool wifiConnect(void)
{
    if (WiFi.status() == WL_CONNECTED)
        return true;

    if (WIFI_SSID.isEmpty())
    {
        LOG_E(TAG, "No SSID configured");
        return false;
    }

    WiFi.mode(WIFI_STA);

    if (WIFI_PASS.isEmpty())
        WiFi.begin(WIFI_SSID.c_str());
    else
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

    uint32_t backoff = WIFI_BACKOFF_MIN_MS;
    while (WiFi.status() != WL_CONNECTED)
    {
        LOG_W(TAG, "WiFi connecting... (retry in %lu ms)", backoff);
        vTaskDelay(pdMS_TO_TICKS(backoff));
        backoff = (backoff * 2 > WIFI_BACKOFF_MAX_MS) ? WIFI_BACKOFF_MAX_MS : backoff * 2;
    }

    LOG_I(TAG, "WiFi connected: %s (RSSI %d)", WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
}

static void publishSensorData(const SensorData_t *msg)
{
    if (msg->type == SENSOR_TYPE_ENVIRONMENT)
    {
        const auto &env = msg->data.env;

        if (xWsMutex != NULL && xSemaphoreTake(xWsMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (ws.count() > 0)
            {
                JsonDocument doc;
                doc["gauge_sound"]    = String(env.sound, 2);
                doc["gauge_pressure"] = String(env.pressure, 2);
                doc["gauge_temp"]     = String(env.temperature, 2);
                doc["gauge_humi"]     = String(env.humidity, 2);
                doc["gauge_pm2p5"]    = String(env.pm2p5, 2);
                doc["gauge_pm10"]     = String(env.pm10, 2);
                String json;
                serializeJson(doc, json);
                ws.textAll(json);
            }
            xSemaphoreGive(xWsMutex);
        }

        if (tb.connected())
        {
            publishData("telemetry", "sound",       env.sound);
            publishData("telemetry", "pressure",    env.pressure);
            publishData("telemetry", "humidity",    env.humidity);
            publishData("telemetry", "temperature", env.temperature);
            publishData("telemetry", "light",       env.light);
            publishData("telemetry", "pm2.5",       env.pm2p5);
            publishData("telemetry", "pm10",        env.pm10);
        }
    }
    else if (msg->type == SENSOR_TYPE_VIBRATION)
    {
        float val = msg->data.vibration.value;

        if (xWsMutex != NULL && xSemaphoreTake(xWsMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (ws.count() > 0)
            {
                JsonDocument doc;
                doc["gauge_vibration"] = String(val, 2);
                String json;
                serializeJson(doc, json);
                ws.textAll(json);
            }
            xSemaphoreGive(xWsMutex);
        }

        if (tb.connected())
        {
            publishData("telemetry", "vibration", val);
        }
    }
}

void TaskConnect(void *pvParameters)
{
    LOG_I(TAG, "TaskConnect started");

    wifiConnect();

    uint32_t mqttBackoff = MQTT_BACKOFF_MIN_MS;

    for (;;)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            stopServer();
            wifiConnect();
            continue;
        }

        if (!isServerRunning)
        {
            connectWSV();
        }

        if (!tb.connected())
        {
            if (mqttConnect())
            {
                mqttBackoff = MQTT_BACKOFF_MIN_MS;
            }
            else
            {
                LOG_W(TAG, "MQTT retry in %lu ms", mqttBackoff);
                vTaskDelay(pdMS_TO_TICKS(mqttBackoff));
                mqttBackoff = (mqttBackoff * 2 > MQTT_BACKOFF_MAX_MS) ? MQTT_BACKOFF_MAX_MS : mqttBackoff * 2;
                continue;
            }
        }

        mqttLoop();

        SensorData_t msg;
        if (xSensorQueue != NULL &&
            xQueueReceive(xSensorQueue, &msg, pdMS_TO_TICKS(QUEUE_POLL_TIMEOUT_MS)) == pdPASS)
        {
            publishSensorData(&msg);
        }
    }
}

void connect_init(void)
{
    xTaskCreate(TaskConnect, "TaskConnect", 8192, NULL, 3, NULL);
}

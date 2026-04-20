#include "TaskMQTT.h"
#include "../common/sensor_data.h"
#include "../common/log.h"

static const char *TAG = "MQTT";

constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint16_t MQTT_RECONNECT_INTERVAL_MS = 5000U;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

constexpr char LED_STATE_ATTR[] = "ledState";

constexpr std::array<const char *, 1U> SHARED_ATTRIBUTES_LIST = {
    LED_STATE_ATTR,
};

static void processSharedAttributes(const Shared_Attribute_Data &data)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0)
        {
            bool state = it->value().as<bool>();
            LOG_I(TAG, "LED state attribute updated: %d", state);
        }
    }
}

static RPC_Response setLedSwitchValue(const RPC_Data &data)
{
    bool newState = data;
    LOG_I(TAG, "RPC setLedSwitch: %d", newState);
    return RPC_Response("setLedSwitchValue", newState);
}

const std::array<RPC_Callback, 1U> callbacks = {
    RPC_Callback{"setLedSwitchValue", setLedSwitchValue}};

const Shared_Attribute_Callback attributes_callback(
    &processSharedAttributes,
    SHARED_ATTRIBUTES_LIST.cbegin(),
    SHARED_ATTRIBUTES_LIST.cend());

const Attribute_Request_Callback attribute_shared_request_callback(
    &processSharedAttributes,
    SHARED_ATTRIBUTES_LIST.cbegin(),
    SHARED_ATTRIBUTES_LIST.cend());

void publishData(const char *mode, const char *feed, float value)
{
    if (xMqttMutex != NULL && xSemaphoreTake(xMqttMutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        if (strcmp(mode, "attribute") == 0)
        {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", value);
            tb.sendAttributeData(feed, buf);
        }
        else if (strcmp(mode, "telemetry") == 0)
        {
            tb.sendTelemetryData(feed, value);
        }
        xSemaphoreGive(xMqttMutex);
    }
    else
    {
        LOG_W(TAG, "Could not acquire MQTT mutex for '%s'", feed);
    }
}

void publishAttributeString(const char *key, const char *value)
{
    if (xMqttMutex != NULL && xSemaphoreTake(xMqttMutex, pdMS_TO_TICKS(200)) == pdTRUE)
    {
        tb.sendAttributeData(key, value);
        xSemaphoreGive(xMqttMutex);
    }
}

bool mqttConnect(void)
{
    if (tb.connected())
        return true;

    LOG_I(TAG, "Connecting to %s:%u ...", MQTT_SERVER, MQTT_PORT);

    if (!tb.connect(MQTT_SERVER, TOKEN.c_str(), MQTT_PORT))
    {
        LOG_E(TAG, "Connection failed");
        return false;
    }

    LOG_I(TAG, "Connected. Subscribing...");

    publishAttributeString("macAddress", WiFi.macAddress().c_str());
    publishAttributeString("localIp", WiFi.localIP().toString().c_str());

    if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend()))
    {
        LOG_E(TAG, "RPC subscribe failed");
        return false;
    }

    if (!tb.Shared_Attributes_Subscribe(attributes_callback))
    {
        LOG_E(TAG, "Shared attribute subscribe failed");
        return false;
    }

    if (!tb.Shared_Attributes_Request(attribute_shared_request_callback))
    {
        LOG_E(TAG, "Shared attribute request failed");
        return false;
    }

    LOG_I(TAG, "Subscriptions active");
    return true;
}

void mqttLoop(void)
{
    if (xMqttMutex != NULL && xSemaphoreTake(xMqttMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        tb.loop();
        xSemaphoreGive(xMqttMutex);
    }
}

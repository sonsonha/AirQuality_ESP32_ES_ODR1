#include "globals.h"
#include "common/sensor_data.h"
#include "common/log.h"
#include "task/TaskHealth.h"

static const char *TAG = "MAIN";

QueueHandle_t xSensorQueue = NULL;
SemaphoreHandle_t xWsMutex = NULL;
SemaphoreHandle_t xMqttMutex = NULL;

void setup()
{
    Serial.begin(115200);
    LOG_I(TAG, "Air Quality Monitor booting...");

    if (!LittleFS.begin(true))
    {
        LOG_E(TAG, "LittleFS mount failed");
    }

    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, sizeof(SensorData_t));
    xWsMutex     = xSemaphoreCreateMutex();
    xMqttMutex   = xSemaphoreCreateMutex();

    if (xSensorQueue == NULL || xWsMutex == NULL || xMqttMutex == NULL)
    {
        LOG_E(TAG, "Failed to create RTOS primitives");
        return;
    }

    if (check_info())
    {
        device_init();
        task_init();
        connect_init();
        health_init();
        LOG_I(TAG, "All tasks launched");
    }
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}

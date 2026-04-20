#include "taskVibration.h"
#include "../common/sensor_data.h"
#include "../common/log.h"

static const char *TAG = "VIBRATION";

#define VIBRATION_PIN       1
#define VIBRATION_ADC_MAX   4095
#define VIBRATION_SCALE     (5.0f / VIBRATION_ADC_MAX * 10.0f)
#define VIBRATION_THRESHOLD 1.0f
#define VIBRATION_PERIOD_MS 200

static float previousValue = 0.0f;

static void TaskVibration(void *pvParameters)
{
    LOG_I(TAG, "Vibration task started on GPIO %d", VIBRATION_PIN);

    for (;;)
    {
        int rawValue = analogRead(VIBRATION_PIN);
        float value = (float)rawValue * VIBRATION_SCALE;

        bool thresholdExceeded = fabsf(value - previousValue) > VIBRATION_THRESHOLD;
        previousValue = value;

        SensorData_t msg;
        msg.type = SENSOR_TYPE_VIBRATION;
        msg.data.vibration.value = value;
        msg.timestamp_ms = (uint32_t)millis();

        if (xSensorQueue != NULL)
        {
            if (xQueueSend(xSensorQueue, &msg, pdMS_TO_TICKS(50)) != pdPASS)
            {
                LOG_W(TAG, "Sensor queue full, dropping vibration reading");
            }
        }

        if (thresholdExceeded)
        {
            LOG_D(TAG, "Threshold exceeded: %.2f (raw=%d)", value, rawValue);
        }

        vTaskDelay(pdMS_TO_TICKS(VIBRATION_PERIOD_MS));
    }
}

void vibration_init(void)
{
    xTaskCreate(TaskVibration, "TaskVibration", 4096, NULL, 2, NULL);
}

#include "TaskVibration.h"

// #define SOUND_PIN 15

// #define VIBRATION_PIN 13
#define VIBRATION_PIN 2

float vibration_value = 0;

void Vibration_sensor()
{
    const int numSamples = 50;
    int count = 0;
    long total = 0;

    int rawValue = analogRead(VIBRATION_PIN);

    // for (int i = 0; i < numSamples; i++)
    // {

    //     if (rawValue > 0)
    //     {
    //         total += rawValue;
    //         count += 1;
    //     }
    // }

    // if (count > 0)
    // {
    //     vibration_value = float(rawValue / count) * 5 / 4095;
    // }
    // else
    // {
    //     Serial.println("No valid data for VIBRATION sensor");
    // }
    vibration_value = float(rawValue) * 5 / 4095;
    Serial.println("Vibration raw: "+String(rawValue));
}

void send_Value()
{
    if (ws.count() > 0)
    {
        DynamicJsonDocument doc(512);
        doc["gauge_vibration"] = String(vibration_value);
        String jsonData;
        serializeJson(doc, jsonData);
        ws.textAll(jsonData);
    }

    if (tb.connected())
    {
        publishData("telemetry", "vibration", String(vibration_value));
        Serial.println("Vibration: "+String(vibration_value));
    }
    // sensor.setMOREMeasurementResult(vibration_value);
}

void TaskMore(void *pvParameters)
{
    while (true)
    {
        Vibration_sensor();
        send_Value();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void More_init()
{
    xTaskCreate(TaskMore, "TaskMore", 8192, NULL, 1, NULL);
}
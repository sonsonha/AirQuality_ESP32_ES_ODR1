#include "TaskVibration.h"

// #define SOUND_PIN 15

// #define VIBRATION_PIN 13
#define VIBRATION_PIN 1
#define VIBRATION_THRESHOLD 1

float vibration_value = 0;
float previous_vibration_value = 0;

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

void Vibration_sensor()
{
    const int numSamples = 50;
    int count = 0;
    long total = 0;

    int rawValue = analogRead(VIBRATION_PIN);

    // vibration_value = (float(rawValue) / 4095) * 2.0; 
    // vibration_value = vibration_value * 9.81;

    vibration_value = (float(rawValue) * 5 / 4095) * 10;
    // vibration_value = float(rawValue) * 5 / 4095;

    Serial.println("Vibration raw: "+String(rawValue));

    if (abs(vibration_value - previous_vibration_value) > VIBRATION_THRESHOLD) {
        send_Value();
    }

    previous_vibration_value = vibration_value;
}

void TaskMore(void *pvParameters)
{
    while (true)
    {
        Vibration_sensor();
        send_Value();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void More_init()
{
    xTaskCreate(TaskMore, "TaskMore", 8192, NULL, 1, NULL);
}
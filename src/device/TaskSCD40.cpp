#include "TaskSCD40.h"

SensirionI2CScd4x scd4x;

void SCD40_sensor()
{
    bool isDataReady = false;
    uint16_t error = scd4x.getDataReadyFlag(isDataReady);
    if (error)
    {
        Serial.printf("getDataReadyFlag Error: %u\n", error);
        return;
    }

    if (!isDataReady)
    {
        Serial.println("Data not ready yet...");
        return;
    }

    uint16_t co2_raw;
    float temperature;
    float humidity;

    error = scd4x.readMeasurement(co2_raw, temperature, humidity);
    if (error)
    {
        Serial.printf("readMeasurement Error: %u\n", error);
        return;
    }

    float co2_ppm = (float)co2_raw;

    if (WiFi.status() == WL_CONNECTED && ws.count() > 0)
    {
        DynamicJsonDocument doc(512);
        doc["co2_ppm"] = String(co2_ppm, 2);
        doc["gauge_temp"] = String(temperature, 2);
        doc["gauge_humi"] = String(humidity, 2);

        String jsonData;
        serializeJson(doc, jsonData);
        ws.textAll(jsonData);
    }
    if (tb.connected())
    {
        publishData("telemetry", "co2", String(co2_ppm, 2));
        publishData("telemetry", "temperature", String(temperature, 2));
        publishData("telemetry", "humidity", String(humidity, 2));
    }
}

void TaskSCD40(void *pvParameters)
{
    uint16_t error;
    scd4x.begin(Wire);
    scd4x.stopPeriodicMeasurement();
    error = scd4x.startPeriodicMeasurement();
    if (error)
    {
        Serial.printf("startPeriodicMeasurement() failed: %u\n", error);
        vTaskDelete(NULL);
    }
    Serial.println("SCD40 started successfully!");
    while (true)
    {
        SCD40_sensor();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void SCD40_init()
{
    xTaskCreate(TaskSCD40, "TaskSCD40", 4096, NULL, 1, NULL);
}

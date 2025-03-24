#include "Sensor.h"

Sensor sensor;

void Sensor::ESIntegrateODR01Sensor(
    float noise,
    float pressure,
    float humidity,
    float temperature,
    float light)
{
    senDR01.noise = noise;
    senDR01.atmosphericPressure = pressure;
    senDR01.ambientHumidity = humidity;
    senDR01.ambientTemperature = temperature;
    senDR01.light = light;
}


void Sensor::sensor_init()
{
    senDR01.noise = 0.0f;
    senDR01.atmosphericPressure = 0.0f;
    senDR01.ambientHumidity = 0.0f;
    senDR01.ambientTemperature = 0.0f;
    senDR01.light = 0.0f;
    
    log_i("Sensor initialized.");
}


void TaskSENSOR(void *pvParameters)
{
    sensor.sensor_init();
    while (true)
    {
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void SENSOR_init()
{
    xTaskCreate(TaskSENSOR, "TaskSENSOR", 4096, NULL, 1, NULL);
}
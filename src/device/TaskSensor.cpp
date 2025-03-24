#include "TaskSensor.h"

HardwareSerial RS485Serial(1);

float noise = 0.0;
float pressure = 0.0;
float humidity = 0.0;
float temperature = 0.0;
float light = 0.0;
float pm2_5 = 0.0;
float pm10 = 0.0;

void sendRS485Command(byte *command, int commandSize, byte *response, int responseSize)
{
    RS485Serial.write(command, commandSize);
    RS485Serial.flush();
    delay(100);
    if (RS485Serial.available() >= responseSize)
    {
        RS485Serial.readBytes(response, responseSize);
    }
}

void _sensor()
{
    uint8_t NoiseRequest[] = {0x06, 0x03, 0x01, 0xF6, 0x00, 0x01, 0x64, 0x73};
    uint8_t PressureRequest[] = {0x06, 0x03, 0x01, 0xF9, 0x00, 0x01, 0x54, 0x70};
    uint8_t PM2_5Request[] = {0x06, 0x03, 0x01, 0xF7, 0x00, 0x01, 0x35, 0xB3};
    uint8_t PM10Request[] = {0x06, 0x03, 0x01, 0xF8, 0x00, 0x01, 0x05, 0xB0};
    uint8_t LightRequest[] = {0x06, 0x03, 0x01, 0xFA, 0x00, 0x01, 0xA4, 0x70};

    uint8_t TemperatureRequest[] = {0x04, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0x9F};
    uint8_t HumidityRequest[] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x5F};

    uint8_t response[7];
    
    sendRS485Command(NoiseRequest, sizeof(NoiseRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        noise = (response[3] << 8) | response[4];
        noise /= 10.0;
    }

    sendRS485Command(PressureRequest, sizeof(PressureRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pressure = (response[3] << 8) | response[4];
        pressure /= 10.0;
    }

    sendRS485Command(PM2_5Request, sizeof(PM2_5Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm2_5 = (response[3] << 8) | response[4];
        pm2_5 /= 10.0;
    }

    sendRS485Command(PM10Request, sizeof(PM10Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm10 = (response[3] << 8) | response[4];
        pm10 /= 10.0;
    }

    sendRS485Command(LightRequest, sizeof(LightRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        light = (response[3] << 8) | response[4];
        light /= 10.0;
    }

    sendRS485Command(TemperatureRequest, sizeof(TemperatureRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        temperature = (response[3] << 8) | response[4];
        temperature /= 10.0;
    }

    sendRS485Command(HumidityRequest, sizeof(HumidityRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        humidity = (response[3] << 8) | response[4];
        humidity /= 10.0;
    }

    Serial.println(noise);
    Serial.println(pressure);
    Serial.println(humidity);
    Serial.println(temperature);
    Serial.println(light);

    if (ws.count() > 0)
    {
        DynamicJsonDocument doc(512);
        doc["gauge_noise"] = String(noise, 2);
        doc["gauge_pressure"] = String(pressure, 2);
        doc["gauge_temp"] = String(temperature, 2);
        doc["gauge_humi"] = String(humidity, 2);
        doc["gauge_light"] = String(light, 2);

        String jsonData;
        serializeJson(doc, jsonData);
        ws.textAll(jsonData);
    }

    if (tb.connected())
    {
        publishData("telemetry", "noise", String(noise, 2));
        publishData("telemetry", "pressure", String(pressure, 2));
        publishData("telemetry", "humidity", String(humidity, 2));
        publishData("telemetry", "temperature", String(temperature, 2));
        publishData("telemetry", "light", String(light, 2));
    }
}

void TaskSensor(void *pvParameters)
{
    while (true)
    {
        _sensor();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void tasksensor_init()
{
    xTaskCreate(TaskSensor, "TaskSensor", 4096, NULL, 1, NULL);
}

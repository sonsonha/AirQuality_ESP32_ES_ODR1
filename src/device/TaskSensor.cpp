#include "TaskSensor.h"

HardwareSerial RS485Serial(1);  

float noise = 0.0;
float pressure = 0.0;
float humidity = 0.0;
float temperature = 0.0;
// float light_high = 0.0;
float light = 0.0;
float pm2p5 = 0.0;
float pm10 = 0.0;


void sendRS485Command(byte *command, int commandSize, byte *response, int responseSize)
{
    RS485Serial.write(command, commandSize);
    RS485Serial.flush();
    delay(100);
    if (RS485Serial.available() >= responseSize)
    {
        RS485Serial.readBytes(response, responseSize);

        // In phản hồi byte nhận được từ cảm biến

        // Serial.print("Response: ");
        // for (int i = 0; i < sizeof(response); i++) {
        //     Serial.print(response[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

    }
    else {
        Serial.println("Failed to read response - - - - - -");
    }
}

void _sensor()
{
    byte NoiseRequest[] = {0x15, 0x03, 0x01, 0xF6, 0x00, 0x01, 0x66, 0xD0};
    byte PressureRequest[] = {0x15, 0x03, 0x01, 0xF9, 0x00, 0x01, 0x56, 0xD3};
    byte pm2p5Request[] = {0x15, 0x03, 0x01, 0xF7, 0x00, 0x01, 0x37, 0x10};
    byte PM10Request[] = {0x15, 0x03, 0x01, 0xF8, 0x00, 0x01, 0x07, 0x13};
    // byte AmbientLight_HighRequest[] = {0x15, 0x03, 0x01, 0xFA, 0x00, 0x01, 0xA6, 0xD3};
    byte AmbientLightRequest[] = {0x15, 0x03, 0x01, 0xFB, 0x00, 0x01, 0xF7, 0x13};
    byte TemperatureRequest[] = {0x15, 0x03, 0x01, 0xF5, 0x00, 0x01, 0x96, 0xD0};
    byte HumidityRequest[] = {0x15, 0x03, 0x01, 0xF4, 0x00, 0x01, 0xC7, 0x10};

    byte response[7];
    
    sendRS485Command(NoiseRequest, sizeof(NoiseRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        noise = (response[3] << 8) | response[4];
        noise /= 10.0;
    } else {
        Serial.println("Failed to read noise");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    sendRS485Command(PressureRequest, sizeof(PressureRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pressure = (response[3] << 8) | response[4];
        pressure /= 10.0;
    } else {
        Serial.println("Failed to read pressure");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    sendRS485Command(pm2p5Request, sizeof(pm2p5Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm2p5 = (response[3] << 8) | response[4];
        pm2p5 /= 10.0;
    } else {
        Serial.println("Failed to read pm2p5");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    sendRS485Command(PM10Request, sizeof(PM10Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm10 = (response[3] << 8) | response[4];
        pm10 /= 10.0;
    } else {
        Serial.println("Failed to read pm10");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    // sendRS485Command(AmbientLight_HighRequest, sizeof(AmbientLight_HighRequest), response, sizeof(response));
    // if (response[1] == 0x03)
    // {
    //     light_high = (response[3] << 8) | response[4];
    //     light_high /= 10.0;
    // } else {
    //     Serial.println("Failed to read light high byte");
    // }

    sendRS485Command(AmbientLightRequest, sizeof(AmbientLightRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        light = (response[3] << 8) | response[4];
        light /= 10.0;
    } else {
        Serial.println("Failed to read light low byte");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    sendRS485Command(TemperatureRequest, sizeof(TemperatureRequest), response, sizeof(response));
    if (response[1])
    {
        temperature = (response[3] << 8) | response[4];
        temperature /= 10.0;
    } else {
        Serial.println("Failed to read temperature");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    sendRS485Command(HumidityRequest, sizeof(HumidityRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        humidity = (response[3] << 8) | response[4];
        humidity /= 10.0;
    } else {
        Serial.println("Failed to read humidity");
    }

    delay(delay_connect);
    memset(response, 0, sizeof(response));	

    Serial.println("noise : "+String(noise));
    Serial.println("pressure: "+String(pressure));
    Serial.println("humidity: "+String(humidity));
    Serial.println("temperature: "+String(temperature));
    // Serial.println("light high: "+String(light_high));
    Serial.println("light: "+String(light));
    Serial.println("pm2p5: "+String(pm2p5));
    Serial.println("pm10: "+String(pm10));

    if (ws.count() > 0)
    {
        DynamicJsonDocument doc(512);
        doc["gauge_noise"] = String(noise, 2);
        doc["gauge_pressure"] = String(pressure, 2);
        doc["gauge_temp"] = String(temperature, 2);
        doc["gauge_humi"] = String(humidity, 2);
        // doc["gauge_light"] = String(light, 2);
        doc["gauge_pm2p5"] = String(pm2p5, 2);
        doc["gauge_pm10"] = String(pm10, 2);

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
        // publishData("telemetry", "light_high", String(light_high, 2));
        publishData("telemetry", "light", String(light, 2));
        publishData("telemetry", "pm2p5", String(pm2p5, 2));
        publishData("telemetry", "pm10", String(pm10, 2));
    }
}

void TaskSensor(void *pvParameters)
{
    RS485Serial.begin(9600, SERIAL_8N1, 9, 8);
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

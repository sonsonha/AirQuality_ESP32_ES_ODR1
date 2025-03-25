#include "tasktest.h"
// Only 1 Modbus RS485 for all sensors
// Sensor: ES-Integrate-ODR1 temp1 + humi1
// Sensor: ES-WS-02 Wind Speed
// Sensor: ES-WS-04 Wind Direction
// Sensor: ES-RainF-01 Rainfall

// Sensor UART communication init
HardwareSerial RS485Serial1(1);


// temp1 and humi1 Sensor
float temp1 = 0.0;
float humi1 = 0.0;

uint8_t temp1Request[] = {0x04, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0x9F};
uint8_t humi1Request[] = {0x04, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x5F};

//////////////////////////////////////////////////////////////////////////////////
// Clears the response buffer before sending a new request
void CleanResponseBuffer() {
    size_t bytesToRead = RS485Serial1.available();
    if (bytesToRead > 0) {
        byte out[bytesToRead];
        RS485Serial1.readBytes(out, bytesToRead);
    }
}

// Sends a command to the specified sensor
void SendCommand(uint8_t *request, size_t size) {
    CleanResponseBuffer();
    RS485Serial1.write(request, size);
}

// Reads and parses data from the sensor
float ReadFromSensor(int responseSize, float divider) {
    byte response[responseSize];
    int available = RS485Serial1.available();
    if (available >= responseSize) {
        RS485Serial1.readBytes(response, responseSize);
        if (response[1] == 0x03) {
            float value = (response[3] << 8) | response[4];
            return value / divider;
        }
    }   
    memset(response, 0, sizeof(response));
    return -1;
}

void Readtemp1() {
    SendCommand(temp1Request, sizeof(temp1Request));
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float temp = ReadFromSensor(7, 10.0);
    if (temp >= 0) {
        temp1 = temp;
        Serial.print("temp1: ");
        Serial.println(temp1);
        tb.sendTelemetryData("temp1: ", temp1);
    } else {
        Serial.println("Failed to read temp1 from ES-Integrate-ODR1-2ThongSo");
    }
}

void Readhumi1() {
    SendCommand(humi1Request, sizeof(humi1Request));
    vTaskDelay(100 / portTICK_PERIOD_MS);
    float hum = ReadFromSensor(7, 10.0);
    if (hum >= 0) {
        humi1 = hum;
        Serial.print("humi1: ");
        Serial.println(humi1);
        tb.sendTelemetryData("humi1", humi1);
    } else {
        Serial.println("Failed to read humi1 from ES-Integrate-ODR1-2ThongSo");
    }
}

void tasktest_iit(void *pvParameters) {
    while (true) {
        Readtemp1();
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
        Readhumi1();
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
    }
}

void tasktest_init() {
    RS485Serial1.begin(9600, SERIAL_8N1, 18, 21); // RX = D5, TX = D6
    xTaskCreate(tasktest_iit, "tasktest_iit", 4096, NULL, 1, NULL);
}

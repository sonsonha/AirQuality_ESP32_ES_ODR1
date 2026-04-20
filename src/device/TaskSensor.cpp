#include "TaskSensor.h"
#include "../common/sensor_data.h"
#include "../common/log.h"

static const char *TAG = "MODBUS";

static HardwareSerial RS485Serial(1);

#define MODBUS_SLAVE_ADDR   0x15
#define MODBUS_FUNC_READ    0x03
#define MODBUS_BAUD         9600
#define MODBUS_RX_PIN       9
#define MODBUS_TX_PIN       8
#define MODBUS_RESPONSE_LEN 7
#define MODBUS_TIMEOUT_MS   200

#define REG_HUMIDITY    0x01F4
#define REG_TEMPERATURE 0x01F5
#define REG_SOUND       0x01F6
#define REG_PM2P5       0x01F7
#define REG_PM10        0x01F8
#define REG_PRESSURE    0x01F9
#define REG_LIGHT       0x01FB

static uint16_t modbusCRC16(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)buf[i];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static bool sendModbusRequest(uint8_t slaveAddr, uint16_t regAddr, uint16_t regCount,
                              uint8_t *response, uint16_t responseLen)
{
    uint8_t frame[8];
    frame[0] = slaveAddr;
    frame[1] = MODBUS_FUNC_READ;
    frame[2] = (regAddr >> 8) & 0xFF;
    frame[3] = regAddr & 0xFF;
    frame[4] = (regCount >> 8) & 0xFF;
    frame[5] = regCount & 0xFF;

    uint16_t crc = modbusCRC16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    while (RS485Serial.available())
        RS485Serial.read();

    RS485Serial.write(frame, sizeof(frame));
    RS485Serial.flush();

    uint32_t start = millis();
    uint16_t idx = 0;
    while (idx < responseLen && (millis() - start) < MODBUS_TIMEOUT_MS)
    {
        if (RS485Serial.available())
        {
            response[idx++] = RS485Serial.read();
        }
    }

    if (idx < responseLen)
    {
        LOG_W(TAG, "Incomplete response: got %u/%u bytes", idx, responseLen);
        return false;
    }

    uint16_t recvCrc = response[responseLen - 2] | (response[responseLen - 1] << 8);
    uint16_t calcCrc = modbusCRC16(response, responseLen - 2);
    if (recvCrc != calcCrc)
    {
        LOG_E(TAG, "CRC mismatch: recv=0x%04X calc=0x%04X", recvCrc, calcCrc);
        return false;
    }

    if (response[1] != MODBUS_FUNC_READ)
    {
        LOG_E(TAG, "Unexpected function code: 0x%02X", response[1]);
        return false;
    }

    return true;
}

static float readModbusRegister(uint8_t slaveAddr, uint16_t regAddr)
{
    uint8_t response[MODBUS_RESPONSE_LEN];
    memset(response, 0, sizeof(response));

    if (!sendModbusRequest(slaveAddr, regAddr, 1, response, MODBUS_RESPONSE_LEN))
    {
        return NAN;
    }

    int16_t raw = (response[3] << 8) | response[4];
    return raw / 10.0f;
}

static void readAllSensors(void)
{
    SensorData_t msg;
    msg.type = SENSOR_TYPE_ENVIRONMENT;
    msg.timestamp_ms = (uint32_t)millis();

    msg.data.env.humidity    = readModbusRegister(MODBUS_SLAVE_ADDR, REG_HUMIDITY);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.temperature = readModbusRegister(MODBUS_SLAVE_ADDR, REG_TEMPERATURE);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.sound       = readModbusRegister(MODBUS_SLAVE_ADDR, REG_SOUND);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.pm2p5       = readModbusRegister(MODBUS_SLAVE_ADDR, REG_PM2P5);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.pm10        = readModbusRegister(MODBUS_SLAVE_ADDR, REG_PM10);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.pressure    = readModbusRegister(MODBUS_SLAVE_ADDR, REG_PRESSURE);
    vTaskDelay(pdMS_TO_TICKS(100));
    msg.data.env.light       = readModbusRegister(MODBUS_SLAVE_ADDR, REG_LIGHT);

    LOG_I(TAG, "T=%.1f H=%.1f PM2.5=%.1f PM10=%.1f P=%.1f L=%.1f S=%.1f",
          msg.data.env.temperature, msg.data.env.humidity,
          msg.data.env.pm2p5, msg.data.env.pm10,
          msg.data.env.pressure, msg.data.env.light, msg.data.env.sound);

    if (xSensorQueue != NULL)
    {
        if (xQueueSend(xSensorQueue, &msg, pdMS_TO_TICKS(100)) != pdPASS)
        {
            LOG_W(TAG, "Sensor queue full, dropping reading");
        }
    }
}

static void TaskSensor(void *pvParameters)
{
    RS485Serial.begin(MODBUS_BAUD, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN);
    LOG_I(TAG, "RS485 initialized on RX=%d TX=%d @ %d baud", MODBUS_RX_PIN, MODBUS_TX_PIN, MODBUS_BAUD);

    for (;;)
    {
        readAllSensors();
        vTaskDelay(pdMS_TO_TICKS(delay_time));
    }
}

void tasksensor_init(void)
{
    xTaskCreate(TaskSensor, "TaskSensor", 4096, NULL, 2, NULL);
}

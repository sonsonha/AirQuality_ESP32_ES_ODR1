#ifndef SENSOR_DATA_H_
#define SENSOR_DATA_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#define SENSOR_QUEUE_LENGTH 16

typedef enum {
    SENSOR_TYPE_ENVIRONMENT,
    SENSOR_TYPE_VIBRATION
} SensorType_t;

typedef struct {
    SensorType_t type;
    union {
        struct {
            float humidity;
            float temperature;
            float pm2p5;
            float pm10;
            float light;
            float pressure;
            float sound;
        } env;
        struct {
            float value;
        } vibration;
    } data;
    uint32_t timestamp_ms;
} SensorData_t;

extern QueueHandle_t xSensorQueue;
extern SemaphoreHandle_t xWsMutex;
extern SemaphoreHandle_t xMqttMutex;

#endif

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include "LittleFS.h"
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "../src/common/defines.h"
#include "../src/common/sensor_data.h"
#include "../src/common/log.h"
#include "../src/common/info.h"

#include "../src/connect/connect_init.h"

#include "../src/task/task_init.h"

#include "../src/device/device_intit.h"

#include "../src/utils/utility_functions.h"

#endif

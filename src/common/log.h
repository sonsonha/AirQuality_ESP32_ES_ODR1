#ifndef LOG_H_
#define LOG_H_

#include "esp_log.h"

#define LOG_I(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
#define LOG_W(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
#define LOG_E(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
#define LOG_D(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)

#endif

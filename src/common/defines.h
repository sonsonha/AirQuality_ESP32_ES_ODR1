#ifndef DEFINES_H_
#define DEFINES_H_

// MQTT broker
#define MQTT_SERVER "app.coreiot.io"
#define MQTT_PORT 1883U

// WiFi AP provisioning
#ifndef SSID_AP
#define SSID_AP "ESP32-SETUP"
#endif

// Task timing (milliseconds)
#define delay_time    10000
#define delay_connect 100

// Battery ADC thresholds (mV after voltage divider)
#define BATTERY_FULL_VOLTAGE  2100
#define BATTERY_EMPTY_VOLTAGE 1500

// GPIO
#define BOOT 0

#ifndef MY_SCL
#define MY_SCL 11
#endif

#ifndef MY_SDA
#define MY_SDA 12
#endif

#define USER_BUTTON_POWER 42
#define POWER_HOLD        46
#define BAT_ADC_PIN       14
#define SEN55_POWER_EN    10

// Filesystem
#define FILESYSTEM LittleFS

// NTP
#define httpPort  80
#define ntpServer "pool.ntp.org"

#endif

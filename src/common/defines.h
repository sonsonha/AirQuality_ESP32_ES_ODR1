// DEFINE MQTT
#define MQTT_SERVER "app.coreiot.io"
#define MQTT_PORT 1883U

// DEFINE WIFI AP
#ifndef SSID_AP
#define SSID_AP "ESP32-SETUP"
#endif

// DEFINE DELAY
#define delay_time 10000
#define delay_gps 15000
#define delay_connect 100
#define delay_30_min 1800000

#define SEN55_POWER_EN 10 /// Chân cảm biến SEN55

// DEFINE PINS
#define BOOT 0
#ifndef MY_SCL
#define MY_SCL 11
#endif

#ifndef MY_SDA
#define MY_SDA 12
#endif

#ifndef TXD_GPS
#define TXD_GPS 18
#endif

#ifndef RXD_GPS
#define RXD_GPS 21
#endif

#define NUM_PIXELS 4
#define Brightness 39 // Set brightness to (0 to 255)

// DEFINE BAUD_RATE
#define BAUD_RATE_1 115200
#define BAUD_RATE_2 9600

// DEFINE NTP
#define httpPort 80

// DEFINE SERVER
#define ntpServer "pool.ntp.org"
#define utcOffsetInSeconds 25200

#include "TaskTime.h"
#include "WiFi.h"
#include "esp_timer.h"
#include <time.h>
#include "../common/sensor_data.h"
#include "../common/log.h"

static const char *TAG = "NTP";

static const long gmtOffset_sec = 7 * 3600;
static const int daylightOffset_sec = 0;

static unsigned long baseEpochTime = 0;
static unsigned long baseMicros = 0;

String current = "";
String date = "";
bool check_different_time = false;

extern AsyncWebSocket ws;

static void syncTimeOnceWithNTP(void)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_W(TAG, "No WiFi, skipping NTP sync");
        return;
    }

    LOG_I(TAG, "Syncing with NTP...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        LOG_E(TAG, "NTP sync failed");
        return;
    }

    time_t now;
    time(&now);
    baseEpochTime = now;
    baseMicros = esp_timer_get_time();

    LOG_I(TAG, "NTP synced: epoch=%lu", baseEpochTime);
}

static unsigned long getCurrentEpochTime(void)
{
    if (baseEpochTime == 0)
        return 0;

    unsigned long elapsedMillis = (esp_timer_get_time() - baseMicros) / 1000;
    return baseEpochTime + (elapsedMillis / 1000);
}

static String getDayOfWeek(unsigned long epochTime)
{
    time_t rawTime = epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return String(days[timeInfo->tm_wday]);
}

static String formatTime(unsigned long epochTime)
{
    time_t rawTime = epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", timeInfo->tm_hour, timeInfo->tm_min);
    return String(buffer);
}

static String getDateString(unsigned long epochTime)
{
    time_t rawTime = epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%02d-%02d-%04d",
             timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
    return String(buffer);
}

static String formatDateTime(unsigned long epochTime)
{
    time_t rawTime = epochTime;
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02d:%02d %02d-%02d-%04d",
             timeInfo->tm_hour, timeInfo->tm_min,
             timeInfo->tm_mday, timeInfo->tm_mon + 1, timeInfo->tm_year + 1900);
    return String(buffer);
}

void TaskTime(void *pvParameters)
{
    LOG_I(TAG, "TaskTime started, waiting for WiFi...");

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    syncTimeOnceWithNTP();

    static String check_time;

    for (;;)
    {
        if (WiFi.status() == WL_CONNECTED && baseEpochTime == 0)
        {
            syncTimeOnceWithNTP();
        }

        unsigned long currentTime = getCurrentEpochTime();
        if (currentTime == 0)
        {
            LOG_W(TAG, "No NTP time yet, waiting...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        String current_time = formatTime(currentTime);
        String dayOfWeek = getDayOfWeek(currentTime);
        date = formatDateTime(currentTime);
        current = dayOfWeek + " " + current_time;
        check_different_time = (check_time != current_time);
        check_time = current_time;

        if (xWsMutex != NULL && xSemaphoreTake(xWsMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (ws.count() > 0)
            {
                String data = "{\"current\":\"" + current + "\"}";
                ws.textAll(data);
            }
            xSemaphoreGive(xWsMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(delay_time));
    }
}

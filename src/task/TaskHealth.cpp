#include "TaskHealth.h"
#include "../connect/TaskMQTT.h"
#include "../common/log.h"
#include "esp_task_wdt.h"

static const char *TAG = "HEALTH";

#define WDT_TIMEOUT_S       30
#define HEALTH_INTERVAL_MS  60000

void watchdog_init(void)
{
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT_S * 1000,
        .idle_core_mask = 0,
        .trigger_panic = true
    };
    esp_err_t err = esp_task_wdt_reconfigure(&wdt_config);
    if (err != ESP_OK)
    {
        LOG_W(TAG, "WDT reconfigure returned 0x%x, attempting init", err);
        esp_task_wdt_init(&wdt_config);
    }
    LOG_I(TAG, "Watchdog configured: %ds timeout", WDT_TIMEOUT_S);
}

static void TaskHealth(void *pvParameters)
{
    esp_task_wdt_add(NULL);
    LOG_I(TAG, "Health task started (interval=%ds)", HEALTH_INTERVAL_MS / 1000);

    for (;;)
    {
        esp_task_wdt_reset();

        uint32_t freeHeap = esp_get_free_heap_size();
        uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
        int32_t rssi = WiFi.RSSI();
        uint32_t uptimeSec = (uint32_t)(millis() / 1000);

        LOG_I(TAG, "heap=%u min_heap=%u rssi=%d uptime=%us",
              freeHeap, minFreeHeap, rssi, uptimeSec);

        if (tb.connected())
        {
            publishAttributeString("freeHeap", String(freeHeap).c_str());
            publishAttributeString("minFreeHeap", String(minFreeHeap).c_str());
            publishAttributeString("rssi", String(rssi).c_str());
            publishAttributeString("uptimeSec", String(uptimeSec).c_str());
        }

        vTaskDelay(pdMS_TO_TICKS(HEALTH_INTERVAL_MS));
    }
}

void health_init(void)
{
    watchdog_init();
    xTaskCreate(TaskHealth, "TaskHealth", 4096, NULL, 1, NULL);
}

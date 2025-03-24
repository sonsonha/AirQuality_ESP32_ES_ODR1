#include "TaskWifi.h"

bool reconnect = false;

void TaskWifi(void *pvParameters)
{
    if (WIFI_SSID.isEmpty())
    {
        vTaskDelete(NULL);
    }

    WiFi.mode(WIFI_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
    }

    String ipAddress = WiFi.localIP().toString();
    Serial.println("Connected to WiFi");
    Serial.println(ipAddress);

#ifdef M5_CORE2
    M5.Display.setCursor(0, 70);
    M5.Display.println("Your webserver :");
    int screenWidth = M5.Lcd.width();
    int textWidth = M5.Display.textWidth(ipAddress);
    M5.Display.setTextColor(YELLOW);
    M5.Display.setCursor((screenWidth - textWidth) / 2, 120);
    M5.Display.println(ipAddress);
#endif

    while (true)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            if (!reconnect)
            {
                reconnect = true;
                WiFi.disconnect();
                WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
                Serial.println("Reconnecting to WiFi...");
            }
        }
        else if (reconnect)
        {
            reconnect = false;
        }
        vTaskDelay(delay_connect / portTICK_PERIOD_MS);
    }
}

void wifi_init()
{
    xTaskCreate(TaskWifi, "TaskWifi", 4096, NULL, 1, NULL);
}
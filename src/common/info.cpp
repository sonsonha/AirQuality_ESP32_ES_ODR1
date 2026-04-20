#include "globals.h"
#include "log.h"

static const char *TAG = "CONFIG";

AsyncWebServer server_1(80);

String NAME_DEVICE;
String WIFI_SSID;
String WIFI_PASS;
String TOKEN;
String EMAIL;

void loadInfoFromFile(void)
{
    File file = LittleFS.open("/info.dat", "r");
    if (!file)
    {
        LOG_W(TAG, "No config file found");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        LOG_E(TAG, "JSON parse failed: %s", error.c_str());
        return;
    }

    NAME_DEVICE = doc["NAME_DEVICE"].as<const char *>();
    WIFI_SSID   = doc["WIFI_SSID"].as<const char *>();
    WIFI_PASS   = doc["WIFI_PASS"].as<const char *>();
    TOKEN       = doc["TOKEN"].as<const char *>();
    EMAIL       = doc["EMAIL"].as<const char *>();

    LOG_I(TAG, "Loaded: device='%s' ssid='%s'", NAME_DEVICE.c_str(), WIFI_SSID.c_str());
}

void deleteInfoFile(void)
{
    if (LittleFS.exists("/info.dat"))
    {
        LittleFS.remove("/info.dat");
        LOG_W(TAG, "Config file deleted");
    }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AIR QUALITY Configuration</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f2f2f2; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; height: 100vh; }
        .container { background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); width: 300px; }
        h2 { text-align: center; color: #333; }
        label { margin-top: 10px; display: block; color: #666; }
        input[type="text"], input[type="password"], input[type="email"], select { width: 100%; padding: 10px; margin-top: 5px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
        input[type="submit"] { background-color: #4CAF50; color: white; border: none; padding: 10px; border-radius: 4px; cursor: pointer; width: 100%; margin-top: 15px; }
        input[type="submit"]:hover { background-color: #45a049; }
    </style>
</head>
<body>
    <div class="container">
        <h2>ESP32 Configuration</h2>
        <form action="/save" method="post">
            <label for="name_device">Name Device:</label>
            <input type="text" name="name_device" id="name_device" required>

            <label for="ssid">WiFi SSID:</label>
            <input type="text" name="ssid" id="ssid" required>

            <label for="pass">WiFi Password:</label>
            <input type="password" name="pass" id="pass">

            <label for="token">MQTT Token:</label>
            <input type="text" name="token" id="token" required>

            <label for="email">Email:</label>
            <input type="email" name="email" id="email" required>

            <input type="submit" value="Save">
        </form>
    </div>
</body>
</html>
)rawliteral";

void startAccessPoint(void)
{
    WiFi.softAP(SSID_AP);
    LOG_I(TAG, "AP started: %s (IP: %s)", SSID_AP, WiFi.softAPIP().toString().c_str());

    server_1.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send_P(200, "text/html", index_html); });

    server_1.on("/save", HTTP_POST, [](AsyncWebServerRequest *request)
                {
        NAME_DEVICE = request->getParam("name_device", true)->value();
        WIFI_SSID = request->getParam("ssid", true)->value();
        WIFI_PASS = request->getParam("pass", true)->value();
        TOKEN = request->getParam("token", true)->value();
        EMAIL = request->getParam("email", true)->value();

        JsonDocument doc;
        doc["NAME_DEVICE"] = NAME_DEVICE;
        doc["WIFI_SSID"] = WIFI_SSID;
        doc["WIFI_PASS"] = WIFI_PASS;
        doc["TOKEN"] = TOKEN;
        doc["EMAIL"] = EMAIL;

        File configFile = LittleFS.open("/info.dat", "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
            LOG_I("CONFIG", "Saved. Restarting...");
            request->send(200, "text/html", "Configuration saved. Restarting...");
            delay(1000);
            ESP.restart();
        } else {
            LOG_E("CONFIG", "Failed to write config file");
            request->send(500, "text/html", "Unable to save configuration.");
        } });

    server_1.begin();

#ifdef M5_CORE2
    unsigned long previousMillis = 0;
    const long interval = 1000;
    int lastStationCount = -1;
    while (true)
    {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            int currentStationCount = WiFi.softAPgetStationNum();
            if (currentStationCount != lastStationCount)
            {
                lastStationCount = currentStationCount;
                lcd.clear(TFT_BLACK);
                lcd.waitDisplay();
                lcd.clear(TFT_WHITE);
                lcd.waitDisplay();

                if (currentStationCount == 0)
                {
                    String apQrcode = "WIFI:T:nopass;S:" + String(SSID_AP) + ";P:;H:false;;";
                    lcd.drawString("Scan me", (lcd.width() - lcd.textWidth("Scan me", &fonts::FreeSansBold9pt7b)) / 2, 15, &fonts::FreeSansBold9pt7b);
                    lcd.qrcode(apQrcode, 35, 35, 130);
                    int textWidth = lcd.textWidth(String(SSID_AP), &fonts::FreeSansBold9pt7b);
                    int xPosition = (lcd.width() - textWidth) / 2;
                    lcd.drawString(String(SSID_AP), xPosition, 175, &fonts::FreeSansBold9pt7b);
                }
                else
                {
                    String ipAddress = "http://" + WiFi.softAPIP().toString();
                    lcd.drawString("Scan me", (lcd.width() - lcd.textWidth("Scan me", &fonts::FreeSansBold9pt7b)) / 2, 15, &fonts::FreeSansBold9pt7b);
                    lcd.qrcode(ipAddress, 35, 35, 130);
                    int textWidth = lcd.textWidth(String(ipAddress), &fonts::FreeSansBold9pt7b);
                    int xPosition = (lcd.width() - textWidth) / 2;
                    lcd.drawString(String(ipAddress), xPosition, 175, &fonts::FreeSansBold9pt7b);
                }
                lcd.waitDisplay();
            }
        }
    }
#endif
}

void TaskResetDevice(void *pvParameters)
{
    unsigned long buttonPressStartTime = 0;
    for (;;)
    {
        if (digitalRead(BOOT) == LOW)
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 5000)
            {
                LOG_W(TAG, "Factory reset triggered");
                deleteInfoFile();
                ESP.restart();
            }
        }
#ifdef M5_CORE2
        else if (M5.BtnB.wasPressed())
        {
            if (buttonPressStartTime == 0)
            {
                buttonPressStartTime = millis();
            }
            else if (millis() - buttonPressStartTime > 5000)
            {
                LOG_W(TAG, "Factory reset triggered (M5 BtnB)");
                deleteInfoFile();
                ESP.restart();
            }
        }
#endif
        else
        {
            buttonPressStartTime = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_connect));
    }
}

static void reset_device(void)
{
    xTaskCreate(TaskResetDevice, "TaskReset", 4096, NULL, 1, NULL);
}

bool check_info(void)
{
#ifdef IS_LEAF
    pinMode(BOOT, INPUT);
#endif
    loadInfoFromFile();
    reset_device();

    if (NAME_DEVICE.isEmpty() || WIFI_SSID.isEmpty() || TOKEN.isEmpty() || EMAIL.isEmpty())
    {
        LOG_W(TAG, "Incomplete config, starting provisioning AP");
        startAccessPoint();
        return false;
    }

    LOG_I(TAG, "Config OK, proceeding to main application");
    return true;
}

#include "WakeupDevice.h"

typedef enum WakeupType_t
{
    E_WAKEUP_TYPE_UNKNOWN = 0,
    E_WAKEUP_TYPE_USER,
    E_WAKEUP_TYPE_USB,
} WakeupType_t;

WakeupType_t wakeupType = E_WAKEUP_TYPE_UNKNOWN;

unsigned long buttonPressStartTime = 0;
bool isDeviceOn = true;

void shutdown()
{
    // Tắt thiết bị
    lcd.sleep();
    lcd.waitDisplay();
    delay(10);
    digitalWrite(POWER_HOLD, LOW);

    lcd.wakeup();
    lcd.waitDisplay();
    log_i("USB powered, continue to operate");
    wakeupType = E_WAKEUP_TYPE_USB;
    digitalWrite(POWER_HOLD, HIGH);
    delay(10);
    gpio_hold_en((gpio_num_t)SEN55_POWER_EN);
    gpio_deep_sleep_hold_en();
    esp_deep_sleep_start();
    isDeviceOn = false;
}

void wakeup()
{
    log_i("Device is waking up");
    isDeviceOn = true;
    digitalWrite(POWER_HOLD, HIGH);
}

WakeupType_t getDeviceWakeupType()
{
    WakeupType_t wakeupType = E_WAKEUP_TYPE_UNKNOWN;
    pinMode(USER_BUTTON_POWER, INPUT);

    if (digitalRead(USER_BUTTON_POWER) == 0)
    {
        if (buttonPressStartTime == 0)
        {
            buttonPressStartTime = millis();
        }
        else if (millis() - buttonPressStartTime >= 5000)
        {
            log_i("Button held for 5 seconds, toggling device state");
            if (isDeviceOn)
            {
                shutdown();
            }
            else
            {
                wakeup();
            }
            buttonPressStartTime = 0;
        }
    }
    else
    {
        buttonPressStartTime = 0;
    }

    return wakeupType;
}

void WakeupDevice()
{
    pinMode(POWER_HOLD, OUTPUT);
    digitalWrite(POWER_HOLD, HIGH);
    wakeupType = getDeviceWakeupType();
}

#include "device_intit.h"
#include "../common/log.h"

static const char *TAG = "DEVICE";

void device_init(void)
{
    Wire.begin(MY_SCL, MY_SDA);
    LOG_I(TAG, "I2C initialized (SCL=%d, SDA=%d)", MY_SCL, MY_SDA);

    tasksensor_init();
    vibration_init();
}

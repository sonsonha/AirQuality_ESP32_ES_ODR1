#include "device_intit.h"

void device_intit()
{
    Wire.begin(MY_SCL, MY_SDA);
    tasksensor_init();
}
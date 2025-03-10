#include "device_intit.h"

void device_intit()
{
    Wire.begin(MY_SCL, MY_SDA);
    GPS_init();
    SEN55_init();
    SCD40_init();
}
#pragma once
#include "globals.h"

class Sensor
{
public:
    void sensor_init();
    Sensor() {}
    ~Sensor() {}
    void ESIntegrateODR01Sensor(
        float noise,
        float pressure,
        float humidity,
        float temperature,
        float light);

public:
    struct
    {
        float noise;
        float atmosphericPressure;
        float ambientHumidity;
        float ambientTemperature;
        float light;
    } senDR01;

private:
    char _errorMessage[256];
};

extern Sensor sensor;

void SENSOR_init();



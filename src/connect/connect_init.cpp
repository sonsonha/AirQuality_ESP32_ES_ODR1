#include "connect_init.h"

void connect_intit()
{
    xTaskCreate(TaskWifi, "TaskWifi", 4096, NULL, 1, NULL);
    xTaskCreate(TaskMQTT, "TaskMQTT", 8192, NULL, 1, NULL);
    xTaskCreate(TaskServer, "TaskServer", 8192, NULL, 1, NULL);
}
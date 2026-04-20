#include "task_init.h"

void task_init(void)
{
    xTaskCreate(TaskTime, "TaskTime", 4096, NULL, 1, NULL);
}

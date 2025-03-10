#ifndef INC_TASKMQTT_H_
#define INC_TASKMQTT_H_

#include "globals.h"

extern ThingsBoard tb;

#ifdef __cplusplus
extern "C"
{
#endif

    void TaskMQTT(void *pvParameters);

#ifdef __cplusplus
}
#endif

void publishData(String mode, String feed, String data);

#endif /* INC_TASKMQTT_H_ */
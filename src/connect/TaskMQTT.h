#ifndef INC_TASKMQTT_H_
#define INC_TASKMQTT_H_

#include "globals.h"

extern ThingsBoard tb;

void publishData(const char *mode, const char *feed, float value);
void publishAttributeString(const char *key, const char *value);
bool mqttConnect(void);
void mqttLoop(void);

#endif

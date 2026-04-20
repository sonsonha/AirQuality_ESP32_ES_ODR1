#ifndef INC_TASKSERVER_H_
#define INC_TASKSERVER_H_

#include "globals.h"

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern bool isServerRunning;

void connectWSV(void);
void stopServer(void);

#endif

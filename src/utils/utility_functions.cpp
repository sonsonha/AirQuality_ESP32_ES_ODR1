#include "utility_functions.h"
#include "../common/log.h"

static const char *TAG = "PARSE";

void parseJson(String message, bool server)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        LOG_W(TAG, "JSON parse error: %s", error.c_str());
        return;
    }

    if (!server)
    {
        if (doc["email"].as<String>() != EMAIL)
        {
            LOG_W(TAG, "Email mismatch, ignoring message");
            return;
        }
    }

    String mode = doc["mode"].as<String>();
    if (mode == "Manual")
    {
        LOG_I(TAG, "Manual mode command received");
    }
}

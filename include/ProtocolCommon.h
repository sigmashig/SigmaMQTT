
#pragma once

typedef struct
{
    String topic;
    int32_t eventId = 0;
    bool isReSubscribe = true;
} ProtocolSubscription;

enum
{
    SIGMAMQTT_CONNECTED = 0xF1000,
    SIGMAMQTT_DISCONNECTED,
    SIGMAMQTT_MESSAGE,
    SIGMAWS_CONNECTED = 0xF2000,
    SIGMAWS_DISCONNECTED,
    SIGMAWS_MESSAGE
} EVENT_IDS;

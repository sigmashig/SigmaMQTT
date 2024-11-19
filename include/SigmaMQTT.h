#ifndef SIGMAMQTT_H
#define SIGMAMQTT_H

#pragma once
#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <SigmaLoger.hpp>
#include <SigmaMQTTPkg.h>


typedef struct{
    String topic;
    int32_t eventId;
    bool isReSubscribe = true;
} SigmaMQTTSubscription;

class SigmaMQTT
{
public:
    static void Init(IPAddress ip, String url = "", uint16_t port = 1883, String user = "", String pwd = "");
    static void ConnectToMqtt();

    inline static TimerHandle_t mqttReconnectTimer;

    static void Subscribe(SigmaMQTTSubscription subscriptionTopic);
    static void Publish(String topic, String payload);
    static void Unsubscribe(String section, String topic);

    static void SetClientId(String id) { strcpy(ClientId, id.c_str()); };

private:
    inline static SigmaLoger *MLogger = new SigmaLoger(512);
    inline static char ClientId[16];
    inline static AsyncMqttClient mqttClient;
    static void onMqttConnect(bool sessionPresent);
    static void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
};

#endif
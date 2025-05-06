#ifndef SIGMAMQTT_H
#define SIGMAMQTT_H

#pragma once
#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <SigmaLoger.h>
#include <SigmaMQTTPkg.h>
#include <map>
#include <esp_event.h>
#include "ProtocolCommon.h"


ESP_EVENT_DECLARE_BASE(SIGMAMQTT_EVENT);

class SigmaMQTT
{
public:
    [[deprecated("Use a single parameter url instead")]] static void Init(IPAddress ip, String url = "", uint16_t port = 1883, String user = "", String pwd = "");
    static void Init(String url, uint16_t port = 1883, String user = "", String pwd = "", String clientId = "");
    static void ConnectToMqtt();

    inline static TimerHandle_t mqttReconnectTimer;

    static void Subscribe(ProtocolSubscription subscriptionTopic, String rootTopic = "");
    static void Subscribe(String topic)
    {
        ProtocolSubscription pkg;
        pkg.topic = topic;
        Subscribe(pkg);
    };
    static void Publish(String topic, String payload);
    static void Unsubscribe(String topic, String rootTopic = "");
    static void Unsubscribe(ProtocolSubscription topic, String rootTopic = "") { Unsubscribe(topic.topic, rootTopic); };

    static void SetClientId(String id) { clientId= id; };
    static void Disconnect();
    static bool IsConnected();

private:
    //inline static SigmaLoger *MLogger = new SigmaLoger(512);
    inline static String clientId;
    inline static AsyncMqttClient mqttClient;
    static void onMqttConnect(bool sessionPresent);
    static void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

    inline static std::map<String, ProtocolSubscription> eventMap;
    inline static std::map<String, String> topicMsg;
};

#endif
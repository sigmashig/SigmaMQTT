#ifndef SIGMAMQTT_H
#define SIGMAMQTT_H

#pragma once
#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <SigmaLoger.h>
#include <SigmaMQTTPkg.h>
#include <map>

typedef struct
{
    String topic;
    int32_t eventId = 0;
    bool isReSubscribe = true;
} SigmaMQTTSubscription;

enum
{
    SIGMAMQTT_CONNECTED = 0xF1000,
    SIGMAMQTT_DISCONNECTED,
    SIGMAMQTT_MESSAGE
} EVENT_IDS;

class SigmaMQTT
{
public:
    [[deprecated("Use a single parameter url instead")]] static void Init(IPAddress ip, String url = "", uint16_t port = 1883, String user = "", String pwd = "");
    static void Init(String url, uint16_t port = 1883, String user = "", String pwd = "", String clientId = "");
    static void ConnectToMqtt();

    inline static TimerHandle_t mqttReconnectTimer;

    static void Subscribe(SigmaMQTTSubscription subscriptionTopic, String rootTopic = "");
    static void Subscribe(String topic)
    {
        SigmaMQTTSubscription pkg;
        pkg.topic = topic;
        Subscribe(pkg);
    };
    static void Publish(String topic, String payload);
    static void Unsubscribe(String topic, String rootTopic = "");
    static void Unsubscribe(SigmaMQTTSubscription topic, String rootTopic = "") { Unsubscribe(topic.topic, rootTopic); };

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

    inline static std::map<String, SigmaMQTTSubscription> eventMap;
    inline static std::map<String, String> topicMsg;
};

#endif
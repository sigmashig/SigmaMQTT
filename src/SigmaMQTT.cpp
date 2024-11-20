#include "SigmaMQTT.h"
#include <esp_event.h>
#include <WiFi.h>

ESP_EVENT_DEFINE_BASE(SIGMAMQTT_EVENT);

void SigmaMQTT::Init(IPAddress ip, String url, uint16_t port, String user, String pwd)
{
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    // mqttClient.onSubscribe(onMqttSubscribe);
    // mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    // mqttClient.onPublish(onMqttPublish);

    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(ConnectToMqtt));

    if (ip[0] != 0)
    {
        mqttClient.setServer(ip, port);
    }
    else
    {
        mqttClient.setServer(url.c_str(), port);
    }

    strncpy(ClientId, (String("Sigma_") + String(random(1000))).c_str(), sizeof(ClientId));
    mqttClient.setClientId(ClientId);
    mqttClient.setCredentials(user.c_str(), pwd.c_str());
    eventMap.clear();
}

void SigmaMQTT::ConnectToMqtt()
{
    //MLogger->Internal("Connecting to MQTT...");
    mqttClient.connect();   
}

void SigmaMQTT::Subscribe(SigmaMQTTSubscription subscriptionTopic)
{
    eventMap[subscriptionTopic.topic] = subscriptionTopic;
    if (mqttClient.connected())
    {
        //MLogger->Append("Send Subscribe for ").Append(subscriptionTopic.topic).Internal();
        mqttClient.subscribe(subscriptionTopic.topic.c_str(), 0);
    }
}

void SigmaMQTT::Publish(String topic, String payload)
{
    mqttClient.publish(topic.c_str(), 0, false, payload.c_str());
}

void SigmaMQTT::Unsubscribe(String topic)
{
    mqttClient.unsubscribe(topic.c_str());
    eventMap.erase(topic);
}

void SigmaMQTT::onMqttConnect(bool sessionPresent)
{
    //MLogger->Internal("Connected to MQTT");
    //MLogger->Internal("Auto Subscribing...");
    for (auto const &x : eventMap)
    {
        if (x.second.isReSubscribe)
        {
            //MLogger->Append("Subscribing to ").Append(x.first).Internal();
            mqttClient.subscribe(x.first.c_str(), 0);
        }
    }
    //MLogger->Internal("Subscribed");
    esp_err_t res = esp_event_post(SIGMAMQTT_EVENT, SIGMAMQTT_CONNECTED, (void*)"", 1, portMAX_DELAY);
}

void SigmaMQTT::onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    bool isReady = true;
    String sPayload = (len == 0 ? "" : String(payload, len));
    //MLogger->Append("{").Append(len).Append(":").Append(index).Append(":").Append(total).Append("}[").Append(topic).Append("]:").Append(sPayload.substring(0, 100)).Internal();

    String sTopic = String(topic);
    for (auto const &x : eventMap)
    {
        if (sTopic==x.first)
        {
            SigmaMQTTPkg pkg(sTopic, sPayload);
            esp_err_t res = esp_event_post(SIGMAMQTT_EVENT, x.second.eventId, pkg.GetMsg(), strlen(pkg.GetMsg()) + 1, portMAX_DELAY);
            return;
        }
    }
}

void SigmaMQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    //MLogger->Internal("Disconnected from MQTT");
    esp_err_t res = esp_event_post(SIGMAMQTT_EVENT, SIGMAMQTT_DISCONNECTED, NULL, 0, portMAX_DELAY);
    if (WiFi.isConnected())
    {
        xTimerStart(mqttReconnectTimer, 0);
    }

}

#include "SigmaMQTT.h"
#include <esp_event.h>
#include <WiFi.h>

ESP_EVENT_DEFINE_BASE(SIGMAMQTT_EVENT);

//[[deprecated("Use a single parameter url instead")]]
void SigmaMQTT::Init(IPAddress ip, String url, uint16_t port, String user, String pwd)
{
    if (ip[0] != 0)
    {
        Init(ip.toString(), port, user, pwd);
    }
    else
    {
        Init(url, port, user, pwd);
    }
}

void SigmaMQTT::Init(String url, uint16_t port, String user, String pwd, String _clientId)
{
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    // mqttClient.onSubscribe(onMqttSubscribe);
    // mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    // mqttClient.onPublish(onMqttPublish);

    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(ConnectToMqtt));
    bool isIp = true;
    for (int i = 0; i < url.length(); i++)
    {
        if (url[i] == '.' || (url[i] >= '0' && url[i] <= '9'))
        {
            continue;
        }
        else
        {
            isIp = false;
            break;
        }
    }

    if (isIp)
    {
        IPAddress ip;
        ip.fromString(url);
        //MLogger->Append("IP: ").Append(ip.toString()).Internal();
        mqttClient.setServer(ip, port);
    }
    else
    {
        // MLogger->Append("URL: ").Append(url).Internal();
        mqttClient.setServer(url.c_str(), port);
    }
    if (_clientId == "") {
        clientId = (String("Sigma_") + String(random(1000))).c_str();
    } else {
        clientId = _clientId;
    }
    // MLogger->Append("ClientId: ").Append(ClientId).Internal();
    mqttClient.setClientId(clientId.c_str());
    mqttClient.setCredentials(user.c_str(), pwd.c_str());
    // MLogger->Internal("MAP CLEAR");
    eventMap.clear();
}

void SigmaMQTT::ConnectToMqtt()
{
    //MLogger->Internal("Connecting to MQTT...");
    mqttClient.connect();
}

void SigmaMQTT::Subscribe(ProtocolSubscription subscriptionTopic, String rootTopic)
{
    if (rootTopic != "")
    {
        subscriptionTopic.topic = rootTopic + subscriptionTopic.topic;
    }
    //    MLogger->Append("Add to map: ").Append(subscriptionTopic.topic).Internal();
    eventMap[subscriptionTopic.topic] = subscriptionTopic;
    if (mqttClient.connected())
    {
        //      MLogger->Append("Send Subscribe for ").Append(subscriptionTopic.topic).Internal();
        mqttClient.subscribe(subscriptionTopic.topic.c_str(), 0);
    }
    // MLogger->Append("Map size(2): ").Append(eventMap.size()).Internal();
}

void SigmaMQTT::Publish(String topic, String payload)
{
    mqttClient.publish(topic.c_str(), 0, false, payload.c_str());
}

void SigmaMQTT::Unsubscribe(String topic, String rootTopic)
{
    if (rootTopic != "")
    {
        topic = rootTopic + topic;
    }
    // MLogger->Append("Unsubscribing from ").Append(topic).Internal();
    mqttClient.unsubscribe(topic.c_str());
    eventMap.erase(topic);
    // MLogger->Append("Map size(3): ").Append(eventMap.size()).Internal();
}
void SigmaMQTT::Disconnect()
{
    if (mqttReconnectTimer != NULL) {
        xTimerStop(mqttReconnectTimer, 0);
    }
    mqttClient.disconnect();
}
bool SigmaMQTT::IsConnected()
{
    return mqttClient.connected();
}
void SigmaMQTT::onMqttConnect(bool sessionPresent)
{
    // MLogger->Internal("Connected to MQTT");
    // MLogger->Internal("Auto Subscribing...");
    // MLogger->Append("Map size(1): ").Append(eventMap.size()).Internal();
    esp_err_t res = esp_event_post(SIGMAMQTT_EVENT, SIGMAMQTT_CONNECTED, (void *)"", 1, portMAX_DELAY);
    for (auto const &x : eventMap)
    {
        // MLogger->Append("Subscribing to ").Append(x.first).Internal();
        if (x.second.isReSubscribe)
        {
            //   MLogger->Append("Subscribing to ").Append(x.first).Internal();
            mqttClient.subscribe(x.first.c_str(), 0);
        }
    }
    // MLogger->Internal("Subscribed");
}

void SigmaMQTT::onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    bool isReady = true;
    String sPayload = (len == 0 ? "" : String(payload, len));
    // MLogger->Append("{").Append(len).Append(":").Append(index).Append(":").Append(total).Append("}[").Append(topic).Append("]:").Append(sPayload.substring(0, 100)).Internal();

    String sTopic = String(topic);

    if (len != total)
    { // fragmented message

        topicMsg[sTopic] += sPayload;
        if (index + len == total)
        {
            sPayload = topicMsg[sTopic];
            topicMsg.erase(sTopic);
            isReady = true;
        }
        else
        {
            isReady = false;
        }
    }
    if (isReady)
    {
        SigmaMQTTPkg pkg(sTopic, sPayload);
        for (auto const &x : eventMap)
        {
            if (sTopic == x.first)
            {
                int32_t e = (x.second.eventId == 0 ? SIGMAMQTT_MESSAGE : x.second.eventId);
                esp_err_t res = esp_event_post(SIGMAMQTT_EVENT, e, pkg.GetMsg(), strlen(pkg.GetMsg()) + 1, portMAX_DELAY);
                return;
            }
        }
        esp_event_post(SIGMAMQTT_EVENT, SIGMAMQTT_MESSAGE, (void *)(pkg.GetMsg()), len, portMAX_DELAY);
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

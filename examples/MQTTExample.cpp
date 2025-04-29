#include <Arduino.h>
#include "SigmaMQTT.h"
#include "WiFi.h"
#include "SigmaLoger.h"
#include "SigmaMQTTPkg.h"


#define MYWIFI_SSID "Sigma"
#define MYWIFI_PASS "kybwynyd"
#define MYMQTT_IP "192.168.0.102"

// SigmaMQTT mqtt;
ESP_EVENT_DECLARE_BASE(SIGMAMQTT_EVENT);

enum EVENT_IDS
{
  MQTT_EVENT_FIRST = 1,
  MQTT_EVENT_SECOND,
  MQTT_EVENT_THIRD,
  MQTT_EVENT_FOURTH,
  MQTT_EVENT_FIFTH

};
TimerHandle_t wifiReconnectTimer;

void TestMqtt()
{
  SigmaMQTT::Publish("test/test1", "Hello world!");
  SigmaMQTT::Publish("test/test999", "Just for publish! ");
  SigmaMQTT::Unsubscribe("test/test1");
  SigmaMQTTSubscription pkg;

  pkg.topic = "test/test1";
  pkg.eventId = MQTT_EVENT_FIRST;
  SigmaMQTT::Subscribe(pkg);
  SigmaMQTT::Publish("test/test1", "Hello world2!");
  pkg.topic = "test/test2";
  pkg.eventId = MQTT_EVENT_THIRD;
  SigmaMQTT::Subscribe(pkg);
}

void mqttEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (strcmp(SIGMAMQTT_EVENT, event_base) == 0)
  {

    if (event_id == SIGMAMQTT_CONNECTED)
    {
      Log->Internal("MQTT connected");
      TestMqtt();
    }
    else if (event_id == MQTT_EVENT_FIRST)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("MQTT_EVENT_FIRST:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload()).Internal();
    }
    else if (event_id == MQTT_EVENT_SECOND)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("MQTT_EVENT_SECOND:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload()).Internal();
    }
    else if (event_id == MQTT_EVENT_THIRD)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("MQTT_EVENT_THIRD:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload()).Internal();
    } // duplicate topic - the new event will overwrite the old one
    else if (event_id == MQTT_EVENT_FOURTH)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("MQTT_EVENT_FOURTH:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload()).Internal();
    }
    else if (event_id == MQTT_EVENT_FIFTH)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("MQTT_EVENT_FIFTH:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload().substring(0,400)).Internal();
    }
    else if (event_id == SIGMAMQTT_DISCONNECTED)
    {
      Log->Internal("MQTT disconnected");
    }
    else if (event_id == SIGMAMQTT_MESSAGE)
    {
      SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);
      Log->Append("SIGMAMQTT_MESSAGE:[").Append(pkg.GetTopic()).Append("]:").Append(pkg.GetPayload()).Internal();
    }

    else
    {
      Log->Internal("Unknown event");
    }
  }
}

void wiFiEvent(WiFiEvent_t event)
{
  Log->Append("WiFi event:" + String(event)).Internal();
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Log->Internal("WiFi connected");
    Log->Append("IP address: ").Append(WiFi.localIP().toString().c_str()).Internal();
    SigmaMQTT::ConnectToMqtt();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Log->Internal("WiFi lost connection");
    xTimerStart(wifiReconnectTimer, 0);
    break;
  }
}

void connectToWifi()
{
  Log->Internal("Connecting to WiFi...");

  WiFi.onEvent(wiFiEvent);

  WiFi.begin(MYWIFI_SSID, MYWIFI_PASS);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("------------------");

  Log = new SigmaLoger(512);
  Log->Append("Starting...").Internal();
  Log->Append("IP: ").Append(MYMQTT_IP).Internal();
  SigmaMQTT::Init(MYMQTT_IP);
  esp_err_t res;
  res = esp_event_loop_create_default();
  if (res != ESP_OK && res != ESP_ERR_INVALID_STATE)
  {
    Log->Printf(F("Failed to create default event loop: %d"), res).Internal();
  }

  delay(100);
  res = esp_event_handler_instance_register(ESP_EVENT_ANY_BASE,
                                            ESP_EVENT_ANY_ID,
                                            mqttEventHandler,
                                            NULL,
                                            NULL);
  if (res != ESP_OK)
  {
    Log->Printf("Failed to register event handler: %d", res).Internal();
  }

  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  connectToWifi();

  SigmaMQTTSubscription topics[] = {
      {"test/test1", MQTT_EVENT_FIRST},
      {"test/test2", MQTT_EVENT_SECOND},
      {"test/test3", MQTT_EVENT_THIRD},
      {"test/test4", MQTT_EVENT_FOURTH} // duplicate topic - the new event will overwrite the old one
  };
  // It is possible to subscribe the topics before the connection is established (when isReSubscribe = true)
  //  the topics will be subscribed after the connection is established
  for (auto topic : topics)
  {
    Log->Append("Subscribing to: ").Append(topic.topic).Internal();
    SigmaMQTT::Subscribe(topic);
  }
  // Topic with long-Long message
  SigmaMQTTSubscription longTopic = {"Config/MBCollector/MB_09/Equipment", MQTT_EVENT_FIFTH};
  SigmaMQTT::Subscribe(longTopic);
}

void loop()
{
  vTaskDelete(NULL);
}

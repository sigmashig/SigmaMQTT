#include <Arduino.h>
#include "SigmaMQTT.h"
#include "WiFi.h"
#include "SigmaLoger.hpp"
#include "SigmaMQTTPkg.h"

SigmaMQTT mqtt;
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

void mqttEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

  if (strcmp(SIGMAMQTT_EVENT, event_base) == 0)
  {
    SigmaMQTTPkg pkg = SigmaMQTTPkg((char *)event_data);

    if (event_id == MQTT_EVENT_FIRST)
    {
      if (pkg.GetTopic() == "test/test1")
      {
        Log->Internal("MQTT_EVENT_FIRST: test/test1");
      }
      else if (pkg.GetTopic() == "test/test3")
      {
        Log->Internal("MQTT_EVENT_FIRST: test/test3");
      }
      else
      {
        Log->Internal("MQTT_EVENT_FIRST: unknown topic");
      }
      Log->Internal("MQTT connected");
    }
    else if (event_id == MQTT_EVENT_SECOND)
    {
      if (pkg.GetTopic() == "test/test2")
      {
        Log->Internal("MQTT_EVENT_SECOND: test/test2");
      }
      else
      {
        Log->Internal("MQTT_EVENT_SECOND: unknown topic");
      }
    }
    else if (event_id == MQTT_EVENT_THIRD)
    {
      if (pkg.GetTopic() == "test/test1")
      {
        Log->Internal("MQTT_EVENT_THIRD: test/test1");
      }
      else
      {
        Log->Internal("MQTT_EVENT_THIRD: unknown topic");
      }
    }
    else
    {
      Log->Internal("Unknown event");
    }
  }
}

void wiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    Log->Internal("WiFi connected");
    Log->Append("IP address: ").Append(WiFi.localIP().toString().c_str());
    mqtt.ConnectToMqtt();
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

  WiFi.begin("Sigma", "kybwynyd");
}

void setup()
{
  Log = new SigmaLoger(512);

  mqtt.Init(IPAddress(192, 168, 0, 98));
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
    Log->Printf(F("Failed to register event handler: %d"), res).Internal();
  }

  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  connectToWifi();

  SigmaMQTTSubscription topics[] = {
      {"test/test1", MQTT_EVENT_FIRST},
      {"test/test2", MQTT_EVENT_SECOND},
      {"test/test3", MQTT_EVENT_FIRST},
      {"test/test1", MQTT_EVENT_THIRD} // duplicate topic - the new event will overwrite the old one
  };
  for (auto topic : topics)
  {
    mqtt.Subscribe(topic);
  }
}

void loop()
{
  vTaskDelete(NULL);
}

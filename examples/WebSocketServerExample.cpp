#include <Arduino.h>
#include "SigmaServerWS.h"
#include "WiFi.h"
#include "SigmaLoger.h"

#define MYWIFI_SSID "Sigma"
#define MYWIFI_PASS "kybwynyd"
#define WEBSOCKET_PORT 8080

ESP_EVENT_DECLARE_BASE(SIGMAWS_EVENT);

enum WS_EVENT_IDS {
  WS_EVENT_FIRST = 1,
  WS_EVENT_SECOND,
  WS_EVENT_THIRD,
  WS_EVENT_FOURTH,
  WS_EVENT_FIFTH
};

TimerHandle_t wifiReconnectTimer;

void TestWebSocket() {
  SigmaWebSocket::Send("/ws/test1", "Hello world!");
  SigmaWebSocket::Send("/ws/test999", "Just for publish!");
  SigmaWebSocket::Unsubscribe("/ws/test1");
  
  ProtocolSubscription pkg;
  pkg.topic = "/ws/test1";
  pkg.eventId = WS_EVENT_FIRST;
  SigmaWebSocket::Subscribe(pkg);
  
  SigmaWebSocket::Send("/ws/test1", "Hello world2!");
  
  pkg.topic = "/ws/test2";
  pkg.eventId = WS_EVENT_THIRD;
  SigmaWebSocket::Subscribe(pkg);
}

void wsEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (strcmp(SIGMAWS_EVENT, event_base) == 0) {
    if (event_id == SIGMAWS_CONNECTED) {
      Log->Internal("WebSocket client connected");
    }
    else if (event_id == SIGMAWS_DISCONNECTED) {
      Log->Internal("WebSocket client disconnected");
    }
    else if (event_id == SIGMAWS_MESSAGE) {
      String message = String((char*)event_data);
      Log->Append("WebSocket message received: ").Append(message).Internal();
    }
    else if (event_id == WS_EVENT_FIRST) {
      String message = String((char*)event_data);
      Log->Append("WS_EVENT_FIRST: ").Append(message).Internal();
    }
    else if (event_id == WS_EVENT_SECOND) {
      String message = String((char*)event_data);
      Log->Append("WS_EVENT_SECOND: ").Append(message).Internal();
    }
    else if (event_id == WS_EVENT_THIRD) {
      String message = String((char*)event_data);
      Log->Append("WS_EVENT_THIRD: ").Append(message).Internal();
    }
    else if (event_id == WS_EVENT_FOURTH) {
      String message = String((char*)event_data);
      Log->Append("WS_EVENT_FOURTH: ").Append(message).Internal();
    }
    else if (event_id == WS_EVENT_FIFTH) {
      String message = String((char*)event_data);
      Log->Append("WS_EVENT_FIFTH: ").Append(message.substring(0, 400)).Internal();
    }
    else {
      Log->Internal("Unknown event");
    }
  }
}

void wiFiEvent(WiFiEvent_t event) {
  Log->Append("WiFi event:" + String(event)).Internal();
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Log->Internal("WiFi connected");
      Log->Append("IP address: ").Append(WiFi.localIP().toString().c_str()).Internal();
      // Start WebSocket server after WiFi is connected
      SigmaWebSocket::StartServer();
      TestWebSocket();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Log->Internal("WiFi lost connection");
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void connectToWifi() {
  Log->Internal("Connecting to WiFi...");
  WiFi.onEvent(wiFiEvent);
  WiFi.begin(MYWIFI_SSID, MYWIFI_PASS);
}

void setup() {
  Serial.begin(115200);
  Serial.println("------------------");

  Log = new SigmaLoger(512);
  Log->Append("Starting WebSocket Example...").Internal();
  Log->Append("WebSocket Port: ").Append(String(WEBSOCKET_PORT)).Internal();
  
  // Initialize WebSocket server
  SigmaWebSocket::Init(WEBSOCKET_PORT);
  
  esp_err_t res;
  res = esp_event_loop_create_default();
  if (res != ESP_OK && res != ESP_ERR_INVALID_STATE) {
    Log->Printf(F("Failed to create default event loop: %d"), res).Internal();
  }

  delay(100);
  res = esp_event_handler_instance_register(ESP_EVENT_ANY_BASE,
                                          ESP_EVENT_ANY_ID,
                                          wsEventHandler,
                                          NULL,
                                          NULL);
  if (res != ESP_OK) {
    Log->Printf("Failed to register event handler: %d", res).Internal();
  }

  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  connectToWifi();

  // Pre-register WebSocket endpoints
  ProtocolSubscription endpoints[] = {
    {"/ws/test1", WS_EVENT_FIRST},
    {"/ws/test2", WS_EVENT_SECOND},
    {"/ws/test3", WS_EVENT_THIRD},
    {"/ws/test4", WS_EVENT_FOURTH}
  };
  
  // It's possible to subscribe to endpoints before the server starts
  for (auto endpoint : endpoints) {
    Log->Append("Registering WebSocket endpoint: ").Append(endpoint.topic).Internal();
    SigmaWebSocket::Subscribe(endpoint);
  }
  
  // Endpoint for handling large messages
  ProtocolSubscription longEndpoint = {"/ws/data", WS_EVENT_FIFTH};
  SigmaWebSocket::Subscribe(longEndpoint);
}

void loop() {
  vTaskDelete(NULL);
} 
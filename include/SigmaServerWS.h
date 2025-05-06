#ifndef SIGMAWEBSOCKET_H
#define SIGMAWEBSOCKET_H
/*
#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SigmaLoger.h>
#include <map>
#include "ProtocolCommon.h"


ESP_EVENT_DECLARE_BASE(SIGMAWS_EVENT);

class SigmaWebSocket {
    
public:
    static void Init(uint16_t port = 80);
    static void StartServer();
    
    static void Subscribe(ProtocolSubscription subscription);
    static void Subscribe(String path) {
        ProtocolSubscription pkg;
        pkg.topic = path;
        Subscribe(pkg);
    }
    
    static void Send(String path, String payload);
    static void Unsubscribe(String path);
    static void Unsubscribe(ProtocolSubscription sub) { Unsubscribe(sub.topic); }
    
    static void Disconnect();
    static bool IsConnected();

private:
    inline static AsyncWebServer* server = nullptr;
    inline static std::map<String, ProtocolSubscription> eventMap;
    inline static std::map<String, AsyncWebSocket*> wsMap;
    
    static void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    static void handleWebSocketMessage(AsyncWebSocket* ws, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len);
};
*/
#endif 
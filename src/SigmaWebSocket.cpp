#include "SigmaServerWS.h"
/*
void SigmaWebSocket::Init(uint16_t port) {
    if (server) {
        delete server;
    }
    server = new AsyncWebServer(port);
}

void SigmaWebSocket::StartServer() {
    if (!server) return;
    
    server->begin();
}

void SigmaWebSocket::Subscribe(ProtocolSubscription subscription) {
    if (!server) return;
    
    String path = subscription.topic;
    if (!path.startsWith("/")) {
        path = "/" + path;
    }
    
    AsyncWebSocket* ws = new AsyncWebSocket(path);
    ws->onEvent(onEvent);
    server->addHandler(ws);
    
    eventMap[path] = subscription;
    wsMap[path] = ws;
}

void SigmaWebSocket::Send(String path, String payload) {
    if (!path.startsWith("/")) {
        path = "/" + path;
    }
    
    auto it = wsMap.find(path);
    if (it != wsMap.end()) {
        it->second->textAll(payload);
    }
}

void SigmaWebSocket::Unsubscribe(String path) {
    if (!path.startsWith("/")) {
        path = "/" + path;
    }
    
    auto it = wsMap.find(path);
    if (it != wsMap.end()) {
        server->removeHandler(it->second);
        delete it->second;
        wsMap.erase(it);
        eventMap.erase(path);
    }
}

void SigmaWebSocket::Disconnect() {
    if (server) {
        for (auto& pair : wsMap) {
            server->removeHandler(pair.second);
            delete pair.second;
        }
        wsMap.clear();
        eventMap.clear();
        server->end();
    }
}

bool SigmaWebSocket::IsConnected() {
    return server != nullptr;
}

void SigmaWebSocket::onEvent(AsyncWebSocket* ws, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            esp_event_post(SIGMAWS_EVENT, SIGMAWS_CONNECTED, nullptr, 0, portMAX_DELAY);
            break;
        case WS_EVT_DISCONNECT:
            esp_event_post(SIGMAWS_EVENT, SIGMAWS_DISCONNECTED, nullptr, 0, portMAX_DELAY);
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(ws, client, arg, data, len);
            break;
        default:
            break;
    }
}

void SigmaWebSocket::handleWebSocketMessage(AsyncWebSocket* ws, AsyncWebSocketClient* client, void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        String message = String((char*)data);
        String path = "";
        for (auto& pair : wsMap) {
            if (pair.second == ws) {
                path = pair.first;
                break;
            }
        }
        
        auto it = eventMap.find(path);
        if (it != eventMap.end()) {
            void* msgData = malloc(message.length() + 1);
            if (msgData) {
                memcpy(msgData, message.c_str(), message.length() + 1);
                esp_event_post(SIGMAWS_EVENT, SIGMAWS_MESSAGE, msgData, message.length() + 1, portMAX_DELAY);
                free(msgData);
            }
        }
    }
} 
*/

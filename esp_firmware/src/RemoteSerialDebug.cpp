#include "RemoteSerialDebug.h"

// 全局变量，保存当前类实例的指针，用于静态回调函数
static RemoteSerialDebug* currentInstance = nullptr;

// 静态回调函数
void onRemoteSerialConnect(void* arg, AsyncClient* client) {
    if (currentInstance) {
        currentInstance->onClientConnect(arg, client);
    }
}

void onRemoteSerialDisconnect(void* arg, AsyncClient* client) {
    if (currentInstance) {
        currentInstance->onClientDisconnect(arg, client);
    }
}

void onRemoteSerialData(void* arg, AsyncClient* client, void* data, size_t len) {
    if (currentInstance) {
        currentInstance->onClientData(arg, client, data, len);
    }
}

void onRemoteSerialError(void* arg, AsyncClient* client, int8_t error) {
    if (currentInstance) {
        currentInstance->onClientError(arg, client, error);
    }
}

// 类方法实现
RemoteSerialDebug::RemoteSerialDebug(Stream* arduino_serial, Stream* debug_serial, int port) 
    : arduinoSerial(arduino_serial), 
      debugSerial(debug_serial),
      debugPort(port),
      transparentMode(false),
      client(nullptr) {
    currentInstance = this;
}

RemoteSerialDebug::~RemoteSerialDebug() {
    if (tcpServer) {
        delete tcpServer;
    }
    if (client && client->connected()) {
        client->close();
    }
}

bool RemoteSerialDebug::begin() {
    if (!arduinoSerial) {
        if (debugSerial) {
            debugSerial->println("[RemoteSerialDebug] Error: Arduino serial not set");
        }
        return false;
    }
    
    tcpServer = new AsyncServer(debugPort);
    if (!tcpServer) {
        if (debugSerial) {
            debugSerial->println("[RemoteSerialDebug] Error: Failed to create TCP server");
        }
        return false;
    }
    
    tcpServer->onClient(&onRemoteSerialConnect, this);
    tcpServer->begin();
    
    if (debugSerial) {
        debugSerial->print("[RemoteSerialDebug] Started on port ");
        debugSerial->println(debugPort);
    }
    
    return true;
}

void RemoteSerialDebug::setTransparentMode(bool enable) {
    transparentMode = enable;
    
    if (debugSerial) {
        debugSerial->print("[RemoteSerialDebug] Transparent mode ");
        debugSerial->println(enable ? "enabled" : "disabled");
    }
    
    // 通知客户端模式已更改
    if (client && client->connected()) {
        String message = String("\r\n[RemoteSerialDebug] Transparent mode ") + 
                         (enable ? "enabled" : "disabled") + 
                         ". All data will " + 
                         (enable ? "now" : "no longer") + 
                         " be directly forwarded to Arduino.\r\n";
        client->write(message.c_str(), message.length());
    }
}

void RemoteSerialDebug::update() {
    // 如果没有处于透明模式或没有客户端连接，不需要处理
    if (!transparentMode || !client || !client->connected()) {
        return;
    }
    
    // 从Arduino串口读取数据并转发到TCP客户端
    while (arduinoSerial->available()) {
        char c = arduinoSerial->read();
        if (client && client->connected()) {
            client->write(&c, 1);
        }
    }
}

// 回调方法实现
void RemoteSerialDebug::onClientConnect(void* arg, AsyncClient* newClient) {
    if (client && client->connected()) {
        // 如果已有客户端连接，拒绝新连接
        newClient->close(true);
        
        if (debugSerial) {
            debugSerial->println("[RemoteSerialDebug] Connection rejected - already connected");
        }
        return;
    }
    
    // 存储新客户端
    client = newClient;
    
    // 设置客户端事件回调
    client->onData(&onRemoteSerialData, this);
    client->onDisconnect(&onRemoteSerialDisconnect, this);
    client->onError(&onRemoteSerialError, this);
    
    // 发送欢迎消息
    String welcomeMsg = String("\r\n[RemoteSerialDebug] Connected to Arduino through ESP32\r\n") +
                        "Type '$transparent' to enable transparent mode\r\n" +
                        "Type '$normal' to return to normal mode\r\n" +
                        "Type '$help' for more commands\r\n\r\n";
    client->write(welcomeMsg.c_str(), welcomeMsg.length());
    
    if (debugSerial) {
        debugSerial->print("[RemoteSerialDebug] Client connected: ");
        debugSerial->println(client->remoteIP().toString());
    }
}

void RemoteSerialDebug::onClientDisconnect(void* arg, AsyncClient* disconnectedClient) {
    if (client == disconnectedClient) {
        client = nullptr;
    }
    
    if (debugSerial) {
        debugSerial->println("[RemoteSerialDebug] Client disconnected");
    }
    
    // 如果客户端断开连接，禁用透明模式
    transparentMode = false;
}

void RemoteSerialDebug::onClientData(void* arg, AsyncClient* sender, void* data, size_t len) {
    uint8_t* d = reinterpret_cast<uint8_t*>(data);
    
    // 检查是否是命令
    if (!transparentMode && len > 1 && d[0] == '$') {
        // 提取命令
        String cmd;
        for (size_t i = 0; i < len; i++) {
            if (d[i] == '\r' || d[i] == '\n') {
                break;
            }
            cmd += (char)d[i];
        }
        
        // 处理命令
        if (cmd == "$transparent") {
            setTransparentMode(true);
            return;
        } 
        else if (cmd == "$normal") {
            setTransparentMode(false);
            return;
        }
        else if (cmd == "$help") {
            String helpMsg = "\r\n[RemoteSerialDebug] Available commands:\r\n" +
                             String("  $transparent - Enable transparent mode (direct forwarding)\r\n") +
                             "  $normal - Disable transparent mode\r\n" +
                             "  $help - Show this help message\r\n\r\n";
            sender->write(helpMsg.c_str(), helpMsg.length());
            return;
        }
        // 如果是未知命令，发送错误消息
        else if (cmd.startsWith("$")) {
            String errorMsg = "\r\n[RemoteSerialDebug] Unknown command: " + cmd + "\r\n" +
                              "Type '$help' for available commands\r\n\r\n";
            sender->write(errorMsg.c_str(), errorMsg.length());
            return;
        }
    }
    
    // 如果处于透明模式，直接将数据转发到Arduino
    if (transparentMode && arduinoSerial) {
        arduinoSerial->write(d, len);
    }
    // 否则，可能是普通文本，回显发送的数据（如终端）
    else {
        String message = "\r\n[RemoteSerialDebug] Not in transparent mode. Type '$transparent' to enable.\r\n";
        sender->write(message.c_str(), message.length());
    }
}

void RemoteSerialDebug::onClientError(void* arg, AsyncClient* client, int8_t error) {
    if (debugSerial) {
        debugSerial->print("[RemoteSerialDebug] Client error: ");
        debugSerial->println(error);
    }
} 
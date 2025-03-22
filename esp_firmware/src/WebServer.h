#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "CommProtocol.h"

/**
 * Web服务器模块
 * 提供REST API和WebSocket通信功能
 */
class WebServerManager {
public:
    // 命令回调函数类型
    using CommandCallback = std::function<void(const char*, const char*)>;
    // 连接状态改变回调函数类型
    using ConnectionCallback = std::function<void(bool)>;

private:
    // AsyncWebServer实例
    AsyncWebServer* _server = nullptr;
    // WebSocket服务器
    AsyncWebSocket* _ws = nullptr;
    
    // WiFi配置
    String _ssid;
    String _password;
    String _hostname;
    
    // WiFi连接状态
    bool _wifiConnected = false;
    // AP模式标志
    bool _isAPMode = false;
    // 上次状态检查时间
    uint32_t _lastCheckTime = 0;
    
    // 命令回调函数
    CommandCallback _commandCallback = nullptr;
    // 连接状态回调函数
    ConnectionCallback _connectionCallback = nullptr;
    
    // 系统状态和数据
    SystemStateCode _currentState = STATE_IDLE;
    String _lastLog;
    String _sensorData;
    
    // WiFi连接超时时间(毫秒)
    const uint32_t WIFI_CONNECT_TIMEOUT = 30000;
    
    // 处理WebSocket消息
    void _handleWebSocketMessage(AsyncWebSocketClient* client, const char* data);
    
    // 发送状态更新到所有WebSocket客户端
    void _sendStatusUpdate();
    
    // 初始化网页服务器
    void _setupWebServer();
    
    // 初始化WiFi连接
    bool _setupWiFi();
    
    // 初始化AP模式
    void _setupAPMode();
    
    // 检查WiFi状态
    void _checkWiFiStatus();

public:
    /**
     * 构造函数
     * @param ssid WiFi SSID
     * @param password WiFi密码
     * @param hostname 主机名
     */
    WebServerManager(const String& ssid = "", const String& password = "", const String& hostname = "b39vs-car");
    
    /**
     * 设置WiFi参数
     * @param ssid WiFi SSID
     * @param password WiFi密码
     */
    void setWiFiCredentials(const String& ssid, const String& password);
    
    /**
     * 设置主机名
     * @param hostname 主机名
     */
    void setHostname(const String& hostname);
    
    /**
     * 初始化Web服务器
     * @param startAP 是否启动AP模式(当WiFi连接失败时)
     * @return 初始化是否成功
     */
    bool begin(bool startAP = true);
    
    /**
     * 设置命令回调函数
     * @param callback 命令回调函数
     */
    void setCommandCallback(CommandCallback callback);
    
    /**
     * 设置连接状态回调函数
     * @param callback 连接状态回调函数
     */
    void setConnectionCallback(ConnectionCallback callback);
    
    /**
     * 更新状态信息
     * @param state 系统状态
     * @param extraParams 额外参数，可为nullptr
     */
    void updateState(SystemStateCode state, const char* extraParams = nullptr);
    
    /**
     * 更新日志信息
     * @param level 日志级别
     * @param message 日志消息
     */
    void updateLog(const char* level, const char* message);
    
    /**
     * 更新传感器数据
     * @param dataType 数据类型
     * @param values 数据值
     */
    void updateData(const char* dataType, const char* values);
    
    /**
     * 获取当前WiFi连接状态
     * @return WiFi连接状态
     */
    bool isWiFiConnected() const { return _wifiConnected; }
    
    /**
     * 检查是否处于AP模式
     * @return AP模式状态
     */
    bool isAPMode() const { return _isAPMode; }
    
    /**
     * 获取本地IP地址
     * @return IP地址字符串
     */
    String getLocalIP() const;
    
    /**
     * 更新函数，在主循环中调用
     */
    void update();
};

extern WebServerManager WebServer;

#endif // WEB_SERVER_H 
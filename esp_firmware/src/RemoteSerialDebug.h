#ifndef REMOTE_SERIAL_DEBUG_H
#define REMOTE_SERIAL_DEBUG_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

/**
 * 远程串口调试类
 * 提供通过TCP连接远程访问Arduino串口的功能
 */
class RemoteSerialDebug {
private:
    AsyncServer* tcpServer;
    AsyncClient* client;
    Stream* arduinoSerial;     // 连接到Arduino的串口
    Stream* debugSerial;       // 调试输出的串口（通常是Serial）
    
    bool transparentMode;      // 是否处于透明传输模式
    int debugPort;             // TCP服务器端口
    
    void onClientConnect(void* arg, AsyncClient* client);
    void onClientDisconnect(void* arg, AsyncClient* client);
    void onClientData(void* arg, AsyncClient* client, void* data, size_t len);
    void onClientError(void* arg, AsyncClient* client, int8_t error);
    
public:
    RemoteSerialDebug(Stream* arduino_serial, Stream* debug_serial = &Serial, int port = 8880);
    ~RemoteSerialDebug();
    
    /**
     * 初始化远程串口调试服务
     * @return 成功返回true，失败返回false
     */
    bool begin();
    
    /**
     * 启用透明传输模式
     * 在该模式下，所有TCP数据直接转发到Arduino串口，Arduino串口数据直接转发到TCP
     * @param enable true启用，false禁用
     */
    void setTransparentMode(bool enable);
    
    /**
     * 检查是否处于透明传输模式
     * @return 是否处于透明传输模式
     */
    bool isTransparentMode() const { return transparentMode; }
    
    /**
     * 获取调试端口
     * @return 调试端口
     */
    int getDebugPort() const { return debugPort; }
    
    /**
     * 是否有客户端连接
     * @return 是否有客户端连接
     */
    bool hasClient() const { return client != nullptr && client->connected(); }
    
    /**
     * 处理Arduino串口数据并转发到TCP
     * 在主循环中调用以处理串口数据
     */
    void update();
};

// 回调函数声明
void onRemoteSerialConnect(void* arg, AsyncClient* client);
void onRemoteSerialDisconnect(void* arg, AsyncClient* client);
void onRemoteSerialData(void* arg, AsyncClient* client, void* data, size_t len);
void onRemoteSerialError(void* arg, AsyncClient* client, int8_t error);

#endif // REMOTE_SERIAL_DEBUG_H 
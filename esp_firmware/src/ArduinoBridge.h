#ifndef ARDUINO_BRIDGE_H
#define ARDUINO_BRIDGE_H

#include <Arduino.h>
#include <functional>
#include "CommProtocol.h"

/**
 * Arduino桥接模块
 * 用于ESP32C3与Arduino之间的串口通信
 */
class ArduinoBridge {
public:
    // 状态回调函数类型
    using StateCallback = std::function<void(SystemStateCode, const char*)>;
    // 日志回调函数类型
    using LogCallback = std::function<void(const char*, const char*)>;
    // 数据回调函数类型
    using DataCallback = std::function<void(const char*, const char*)>;

private:
    Stream* _serial;               // 串口对象
    char _buffer[256];             // 接收缓冲区
    int _bufferIndex = 0;          // 缓冲区索引
    bool _initialized = false;     // 初始化标志
    
    // 回调函数
    StateCallback _stateCallback = nullptr;
    LogCallback _logCallback = nullptr;
    DataCallback _dataCallback = nullptr;
    
    // 解析接收到的消息
    void _parseMessage(const char* message);

public:
    /**
     * 构造函数
     * @param serial 串口对象指针
     */
    ArduinoBridge(Stream* serial);
    
    /**
     * 初始化桥接模块
     * @return 初始化是否成功
     */
    bool begin();
    
    /**
     * 设置状态回调函数
     * @param callback 状态回调函数
     */
    void setStateCallback(StateCallback callback);
    
    /**
     * 设置日志回调函数
     * @param callback 日志回调函数
     */
    void setLogCallback(LogCallback callback);
    
    /**
     * 设置数据回调函数
     * @param callback 数据回调函数
     */
    void setDataCallback(DataCallback callback);
    
    /**
     * 向Arduino发送命令
     * @param cmdType 命令类型
     * @param params 参数字符串，可为nullptr
     * @return 发送是否成功
     */
    bool sendCommand(CommandType cmdType, const char* params = nullptr);
    
    /**
     * 向Arduino发送命令（使用字符串命令名）
     * @param cmdName 命令名称字符串
     * @param params 参数字符串，可为nullptr
     * @return 发送是否成功
     */
    bool sendCommand(const char* cmdName, const char* params = nullptr);
    
    /**
     * 请求Arduino状态
     * @return 请求是否成功发送
     */
    bool requestStatus();
    
    /**
     * 更新函数，在主循环中调用以处理接收到的数据
     */
    void update();
    
    /**
     * 检查桥接是否已初始化
     * @return 初始化状态
     */
    bool isInitialized() const { return _initialized; }
};

#endif // ARDUINO_BRIDGE_H 
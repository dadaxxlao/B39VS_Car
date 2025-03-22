#ifndef BLUETOOTH_SERIAL_H
#define BLUETOOTH_SERIAL_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Config.h"
#include "Logger.h"

// 消息类型定义
enum BtMessageType {
    BT_MSG_COMMAND,  // 命令消息
    BT_MSG_DATA,     // 数据消息
    BT_MSG_LOG,      // 日志消息
    BT_MSG_RESPONSE  // 响应消息
};

class BluetoothSerial {
private:
    SoftwareSerial* btSerial;
    bool initialized;
    
    // 命令解析缓冲区
    static const int BT_BUFFER_SIZE = 128;
    char buffer[BT_BUFFER_SIZE];
    int bufferIndex;
    
    // 蓝牙消息前缀
    static const char* MSG_PREFIX_COMMAND;
    static const char* MSG_PREFIX_DATA;
    static const char* MSG_PREFIX_LOG;
    static const char* MSG_PREFIX_RESPONSE;
    
public:
    BluetoothSerial() : initialized(false), bufferIndex(0) {
        memset(buffer, 0, BT_BUFFER_SIZE);
    }
    
    // 初始化蓝牙串口
    bool begin(int rxPin, int txPin, long baudRate = 9600) {
        if (!ENABLE_BLUETOOTH) {
            return false;
        }
        
        btSerial = new SoftwareSerial(rxPin, txPin);
        btSerial->begin(baudRate);
        
        // 简单测试蓝牙是否就绪
        btSerial->println("AT");
        delay(500);
        
        if (btSerial->available()) {
            // 清空接收缓冲区
            while (btSerial->available()) {
                btSerial->read();
            }
            initialized = true;
            
            // 如果初始化成功，设置Logger的蓝牙流
            setupLogger();
        }
        
        return initialized;
    }
    
    // 设置Logger的蓝牙流
    void setupLogger() {
        if (ENABLE_BLUETOOTH && initialized && btSerial) {
            Logger::setBtStream(btSerial);
        }
    }
    
    // 检查是否初始化
    bool isInitialized() const {
        return ENABLE_BLUETOOTH && initialized;
    }
    
    // 检查是否有可用数据
    bool available() {
        return ENABLE_BLUETOOTH && btSerial && btSerial->available();
    }
    
    // 读取一个字节
    int read() {
        return (ENABLE_BLUETOOTH && btSerial) ? btSerial->read() : -1;
    }
    
    // 向蓝牙发送数据
    size_t write(uint8_t data) {
        return (ENABLE_BLUETOOTH && btSerial) ? btSerial->write(data) : 0;
    }
    
    // 向蓝牙发送字符串
    size_t print(const char* str) {
        return (ENABLE_BLUETOOTH && btSerial) ? btSerial->print(str) : 0;
    }
    
    // 向蓝牙发送字符串并换行
    size_t println(const char* str) {
        return (ENABLE_BLUETOOTH && btSerial) ? btSerial->println(str) : 0;
    }
    
    // 格式化并发送字符串
    size_t printf(const char* format, ...) {
        if (!ENABLE_BLUETOOTH || !btSerial) return 0;
        
        char tempBuffer[128];
        va_list args;
        va_start(args, format);
        vsnprintf(tempBuffer, sizeof(tempBuffer), format, args);
        va_end(args);
        
        return btSerial->print(tempBuffer);
    }
    
    // 读取一行数据（直到换行符）
    String readLine() {
        if (!ENABLE_BLUETOOTH || !btSerial) return "";
        
        return btSerial->readStringUntil('\n');
    }
    
    // 发送命令响应
    void sendResponse(const char* command, bool success) {
        if (!ENABLE_BLUETOOTH || !btSerial) return;
        
        btSerial->print(MSG_PREFIX_RESPONSE);
        btSerial->print(command);
        btSerial->print(",");
        btSerial->println(success ? "OK" : "FAIL");
    }
    
    // 发送传感器数据
    void sendSensorData(const uint16_t* sensorValues, int position) {
        if (!ENABLE_BLUETOOTH || !btSerial) return;
        
        btSerial->print(MSG_PREFIX_DATA);
        btSerial->print("LINE,");
        btSerial->print(position);
        
        for (int i = 0; i < 8; i++) {
            btSerial->print(",");
            btSerial->print(sensorValues[i]);
        }
        
        btSerial->println();
    }
    
    // 发送日志消息
    void sendLog(int level, const char* message) {
        if (!ENABLE_BLUETOOTH || !btSerial) return;
        
        const char* levelStr;
        switch (level) {
            case LOG_LEVEL_ERROR:
                levelStr = "ERROR";
                break;
            case LOG_LEVEL_WARNING:
                levelStr = "WARN";
                break;
            case LOG_LEVEL_INFO:
                levelStr = "INFO";
                break;
            case LOG_LEVEL_DEBUG:
                levelStr = "DEBUG";
                break;
            default:
                levelStr = "UNKNOWN";
                break;
        }
        
        btSerial->print(MSG_PREFIX_LOG);
        btSerial->print(levelStr);
        btSerial->print(",");
        btSerial->println(message);
    }
    
    // 处理接收到的数据
    bool processReceivedData() {
        if (!ENABLE_BLUETOOTH || !btSerial || !btSerial->available()) {
            return false;
        }
        
        // 读取数据到缓冲区
        while (btSerial->available() && bufferIndex < BT_BUFFER_SIZE - 1) {
            char c = btSerial->read();
            
            // 检测到行结束
            if (c == '\n' || c == '\r') {
                if (bufferIndex > 0) {
                    buffer[bufferIndex] = '\0';
                    bufferIndex = 0;
                    return true;
                }
            } else {
                buffer[bufferIndex++] = c;
            }
        }
        
        return false;
    }
    
    // 获取最后接收到的命令
    const char* getLastCommand() {
        return buffer;
    }
    
    // 清空接收缓冲区
    void clearBuffer() {
        if (!ENABLE_BLUETOOTH || !btSerial) return;
        
        while (btSerial->available()) {
            btSerial->read();
        }
        bufferIndex = 0;
    }
    
    // 解析蓝牙消息类型
    BtMessageType parseMessageType(const char* message) {
        if (strncmp(message, MSG_PREFIX_COMMAND, strlen(MSG_PREFIX_COMMAND)) == 0) {
            return BT_MSG_COMMAND;
        } else if (strncmp(message, MSG_PREFIX_DATA, strlen(MSG_PREFIX_DATA)) == 0) {
            return BT_MSG_DATA;
        } else if (strncmp(message, MSG_PREFIX_LOG, strlen(MSG_PREFIX_LOG)) == 0) {
            return BT_MSG_LOG;
        } else if (strncmp(message, MSG_PREFIX_RESPONSE, strlen(MSG_PREFIX_RESPONSE)) == 0) {
            return BT_MSG_RESPONSE;
        } else {
            // 默认当作命令处理
            return BT_MSG_COMMAND;
        }
    }
};

// 定义静态成员
const char* BluetoothSerial::MSG_PREFIX_COMMAND = "$CMD:";
const char* BluetoothSerial::MSG_PREFIX_DATA = "$DATA:";
const char* BluetoothSerial::MSG_PREFIX_LOG = "$LOG:";
const char* BluetoothSerial::MSG_PREFIX_RESPONSE = "$RESP:";

// 全局蓝牙实例
extern BluetoothSerial BtSerial;

#endif // BLUETOOTH_SERIAL_H 
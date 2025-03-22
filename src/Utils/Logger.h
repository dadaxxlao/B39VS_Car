#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "Config.h"

// 日志级别定义
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// 通信类型枚举
enum CommunicationType {
    COMM_SERIAL, // 普通串口
    COMM_BT,     // 蓝牙
    COMM_ESP,    // ESP
    COMM_COUNT   // 通信类型数量
};

// 当前日志级别
static int currentLogLevel = LOG_LEVEL_INFO;

class Logger {
private:
    // 各种通信流
    static Stream* commStreams[COMM_COUNT];
    static bool commEnabled[COMM_COUNT];

public:
    static void init() {
        // 确保Serial已初始化
        if (!Serial) {
            Serial.begin(115200);
            delay(100);
        }
        
        // 初始化通信流和状态
        commStreams[COMM_SERIAL] = &Serial;
        commStreams[COMM_BT] = nullptr;
        commStreams[COMM_ESP] = nullptr;
        
        commEnabled[COMM_SERIAL] = true;
        commEnabled[COMM_BT] = ENABLE_BLUETOOTH;
        commEnabled[COMM_ESP] = ENABLE_ESP;
        
        Serial.println(F("Logger initialized"));
    }
    
    // 设置通信流
    static void setStream(CommunicationType type, Stream* stream) {
        if (type >= 0 && type < COMM_COUNT) {
            commStreams[type] = stream;
        }
    }
    
    // 设置蓝牙串口流 (兼容旧代码)
    static void setBtStream(Stream* stream) {
        setStream(COMM_BT, stream);
    }
    
    // 启用或禁用某种通信方式
    static void enableComm(CommunicationType type, bool enable) {
        if (type >= 0 && type < COMM_COUNT) {
            commEnabled[type] = enable;
        }
    }
    
    // 设置日志级别
    static void setLogLevel(int level) {
        currentLogLevel = level;
    }
    
    static void update() {
        // 定期刷新日志，可以在此添加日志缓冲处理
    }
    
    static void error(const char* format, ...) {
        if (currentLogLevel >= LOG_LEVEL_ERROR) {
            // 准备格式化字符串
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            // 输出到串口
            if (commEnabled[COMM_SERIAL] && commStreams[COMM_SERIAL]) {
                commStreams[COMM_SERIAL]->print(F("[ERROR] "));
                commStreams[COMM_SERIAL]->println(buffer);
            }
            
            // 输出到蓝牙
            if (commEnabled[COMM_BT] && commStreams[COMM_BT]) {
                commStreams[COMM_BT]->print(F("$LOG:ERROR,"));
                commStreams[COMM_BT]->println(buffer);
            }
            
            // 输出到ESP
            if (commEnabled[COMM_ESP] && commStreams[COMM_ESP]) {
                commStreams[COMM_ESP]->print(F("$LOG:ERROR,"));
                commStreams[COMM_ESP]->println(buffer);
            }
        }
    }
    
    static void warning(const char* format, ...) {
        if (currentLogLevel >= LOG_LEVEL_WARNING) {
            // 准备格式化字符串
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            // 输出到串口
            if (commEnabled[COMM_SERIAL] && commStreams[COMM_SERIAL]) {
                commStreams[COMM_SERIAL]->print(F("[WARN] "));
                commStreams[COMM_SERIAL]->println(buffer);
            }
            
            // 输出到蓝牙
            if (commEnabled[COMM_BT] && commStreams[COMM_BT]) {
                commStreams[COMM_BT]->print(F("$LOG:WARN,"));
                commStreams[COMM_BT]->println(buffer);
            }
            
            // 输出到ESP
            if (commEnabled[COMM_ESP] && commStreams[COMM_ESP]) {
                commStreams[COMM_ESP]->print(F("$LOG:WARN,"));
                commStreams[COMM_ESP]->println(buffer);
            }
        }
    }
    
    static void info(const char* format, ...) {
        if (currentLogLevel >= LOG_LEVEL_INFO) {
            // 准备格式化字符串
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            // 输出到串口
            if (commEnabled[COMM_SERIAL] && commStreams[COMM_SERIAL]) {
                commStreams[COMM_SERIAL]->print(F("[INFO] "));
                commStreams[COMM_SERIAL]->println(buffer);
            }
            
            // 输出到蓝牙
            if (commEnabled[COMM_BT] && commStreams[COMM_BT]) {
                commStreams[COMM_BT]->print(F("$LOG:INFO,"));
                commStreams[COMM_BT]->println(buffer);
            }
            
            // 输出到ESP
            if (commEnabled[COMM_ESP] && commStreams[COMM_ESP]) {
                commStreams[COMM_ESP]->print(F("$LOG:INFO,"));
                commStreams[COMM_ESP]->println(buffer);
            }
        }
    }
    
    static void debug(const char* format, ...) {
        if (currentLogLevel >= LOG_LEVEL_DEBUG) {
            // 准备格式化字符串
            char buffer[128];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            // 输出到串口
            if (commEnabled[COMM_SERIAL] && commStreams[COMM_SERIAL]) {
                commStreams[COMM_SERIAL]->print(F("[DEBUG] "));
                commStreams[COMM_SERIAL]->println(buffer);
            }
            
            // 输出到蓝牙
            if (commEnabled[COMM_BT] && commStreams[COMM_BT]) {
                commStreams[COMM_BT]->print(F("$LOG:DEBUG,"));
                commStreams[COMM_BT]->println(buffer);
            }
            
            // 输出到ESP
            if (commEnabled[COMM_ESP] && commStreams[COMM_ESP]) {
                commStreams[COMM_ESP]->print(F("$LOG:DEBUG,"));
                commStreams[COMM_ESP]->println(buffer);
            }
        }
    }
};

// 注意：静态成员变量的定义已移到Logger.cpp文件中
// Stream* Logger::btStream = nullptr;

#endif // LOGGER_H 
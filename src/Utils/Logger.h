#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "Config.h"

// 日志级别定义
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// 当前日志级别
static int currentLogLevel = LOG_LEVEL_INFO;

class Logger {
private:
    static Stream* btStream;  // 蓝牙串口流
    static bool useBluetooth; // 是否使用蓝牙输出

public:
    static void init() {
        // 确保Serial已初始化
        if (!Serial) {
            Serial.begin(115200);
            delay(100);
        }
        useBluetooth = false;
        btStream = nullptr;
        Serial.println(F("Logger initialized"));
    }
    
    // 设置蓝牙串口流
    static void setBtStream(Stream* stream) {
        btStream = stream;
        useBluetooth = (stream != nullptr);
    }
    
    // 启用或禁用蓝牙输出
    static void enableBluetooth(bool enable) {
        useBluetooth = enable && (btStream != nullptr);
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
            Serial.print(F("[ERROR] "));
            Serial.println(buffer);
            
            // 输出到蓝牙
            if (useBluetooth && btStream) {
                btStream->print(F("$LOG:ERROR,"));
                btStream->println(buffer);
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
            Serial.print(F("[WARN] "));
            Serial.println(buffer);
            
            // 输出到蓝牙
            if (useBluetooth && btStream) {
                btStream->print(F("$LOG:WARN,"));
                btStream->println(buffer);
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
            Serial.print(F("[INFO] "));
            Serial.println(buffer);
            
            // 输出到蓝牙
            if (useBluetooth && btStream) {
                btStream->print(F("$LOG:INFO,"));
                btStream->println(buffer);
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
            Serial.print(F("[DEBUG] "));
            Serial.println(buffer);
            
            // 输出到蓝牙
            if (useBluetooth && btStream) {
                btStream->print(F("$LOG:DEBUG,"));
                btStream->println(buffer);
            }
        }
    }
};

// 初始化静态成员
Stream* Logger::btStream = nullptr;
bool Logger::useBluetooth = false;

#endif // LOGGER_H 
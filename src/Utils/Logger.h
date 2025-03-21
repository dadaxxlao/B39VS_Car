#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// 日志级别定义
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// 设置当前日志级别
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

class Logger {
public:
    static void init() {
        // 确保Serial已初始化
        if (!Serial) {
            Serial.begin(115200);
            delay(100);
        }
        Serial.println(F("Logger initialized"));
    }
    
    static void update() {
        // 定期刷新日志，可以在此添加日志缓冲处理
    }
    
    static void error(const char* format, ...) {
        if (CURRENT_LOG_LEVEL >= LOG_LEVEL_ERROR) {
            Serial.print(F("[ERROR] "));
            va_list args;
            va_start(args, format);
            char buffer[128];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.println(buffer);
            va_end(args);
        }
    }
    
    static void warning(const char* format, ...) {
        if (CURRENT_LOG_LEVEL >= LOG_LEVEL_WARNING) {
            Serial.print(F("[WARN] "));
            va_list args;
            va_start(args, format);
            char buffer[128];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.println(buffer);
            va_end(args);
        }
    }
    
    static void info(const char* format, ...) {
        if (CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO) {
            Serial.print(F("[INFO] "));
            va_list args;
            va_start(args, format);
            char buffer[128];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.println(buffer);
            va_end(args);
        }
    }
    
    static void debug(const char* format, ...) {
        if (CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG) {
            Serial.print(F("[DEBUG] "));
            va_list args;
            va_start(args, format);
            char buffer[128];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.println(buffer);
            va_end(args);
        }
    }
};

#endif // LOGGER_H 
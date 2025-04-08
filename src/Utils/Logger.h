#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "Config.h"

// 日志级别定义
#define LOG_LEVEL_ERROR   1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO    3
#define LOG_LEVEL_DEBUG   4

// 日志级别名称（保存在Flash中）
#define LOG_LEVEL_ERROR_STR   F("ERROR")
#define LOG_LEVEL_WARNING_STR F("WARN")
#define LOG_LEVEL_INFO_STR    F("INFO")
#define LOG_LEVEL_DEBUG_STR   F("DEBUG")


// 通信类型枚举
enum CommunicationType {
    COMM_SERIAL, // 普通串口
    COMM_BT,     // 蓝牙
    COMM_ESP32,  // ESP32通信
    COMM_COUNT   // 通信类型数量
};

// 日志配置结构体
struct LoggerConfig {
    int logLevel;          // 日志级别
    bool useTimestamp;     // 是否使用时间戳
    bool usePrefix;        // 是否使用前缀（如$LOG:）
    char tag[16];          // 日志标签 [TAG]
};

// 通信通道配置
struct CommConfig {
    bool enabled;          // 是否启用
    Stream* stream;        // 通信流
    LoggerConfig config;   // 日志配置
};

class Logger {
private:
    // 共享消息缓冲区
    static char messageBuffer[256];
    static char timeBuffer[32];
    
    // 各个通信通道的配置
    static CommConfig commConfigs[COMM_COUNT];
    
    // 系统启动时间
    static unsigned long startTime;
    
    // --- 标签日志级别存储 ---
    // 标签日志级别结构体
    struct TagLogLevel {
        char name[16]; // 标签名称
        int level;     // 日志级别
    };
    
    // 标签日志级别数组
    static const int MAX_TAGS = 10; // 最多支持10个特定标签级别
    static TagLogLevel tagLogLevels[MAX_TAGS];
    static int tagLogLevelCount; // 当前配置的标签数量
    
    // 获取指定标签的日志级别
    static int getLogLevelForTag(const char* tag);
    
    // 内部日志处理函数，增加tag参数
    static void logInternal(int level, const __FlashStringHelper* levelStr, const char* tag, const char* format, va_list args);
    
    // 格式化时间戳
    static void formatTimestamp(char* buffer, size_t size);

public:
    static void init() {
        // 记录启动时间
        startTime = millis();
        
        // 确保Serial已初始化
        if (!Serial) {
            Serial.begin(115200);
            delay(100);
        }
        
        // 初始化通信配置
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            commConfigs[i].enabled = false;
            commConfigs[i].stream = nullptr;
            commConfigs[i].config.logLevel = LOG_LEVEL_INFO;
            commConfigs[i].config.useTimestamp = (i == COMM_SERIAL); // 默认只在Serial上显示时间戳
            commConfigs[i].config.usePrefix = false; // Disable prefix for all non-Serial channels by default
            commConfigs[i].config.tag[0] = '\0';                     // 默认无标签
        }
        
        // 设置默认配置
        commConfigs[COMM_SERIAL].enabled = true;
        commConfigs[COMM_SERIAL].stream = &Serial;
        
        commConfigs[COMM_BT].enabled = ENABLE_BLUETOOTH;
        
        // 初始化标签日志级别数组
        tagLogLevelCount = 0;
        
        Serial.println(F("Logger initialized"));
    }
    
    // 设置通信流
    static void setStream(CommunicationType type, Stream* stream) {
        if (type >= 0 && type < COMM_COUNT) {
            commConfigs[type].stream = stream;
        }
    }
    
    // 设置蓝牙串口流 (兼容旧代码)
    static void setBtStream(Stream* stream) {
        setStream(COMM_BT, stream);
    }
    
    // 启用或禁用某种通信方式
    static void enableComm(CommunicationType type, bool enable) {
        if (type >= 0 && type < COMM_COUNT) {
            commConfigs[type].enabled = enable;
        }
    }
    
    // 配置通信通道
    static void configureChannel(CommunicationType type, const LoggerConfig& config) {
        if (type >= 0 && type < COMM_COUNT) {
            commConfigs[type].config = config;
        }
    }
    
    // 为特定通道设置日志级别
    static void setLogLevel(CommunicationType type, int level) {
        if (type >= 0 && type < COMM_COUNT) {
            commConfigs[type].config.logLevel = level;
        }
    }
    
    // 设置全局日志级别（所有通道）
    static void setGlobalLogLevel(int level) {
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            commConfigs[i].config.logLevel = level;
        }
    }
    
    // 设置日志标签
    static void setLogTag(CommunicationType type, const char* tag) {
        if (type >= 0 && type < COMM_COUNT) {
            strncpy(commConfigs[type].config.tag, tag, sizeof(commConfigs[type].config.tag) - 1);
            commConfigs[type].config.tag[sizeof(commConfigs[type].config.tag) - 1] = '\0';
        }
    }
    
    // 设置全局日志标签
    static void setGlobalLogTag(const char* tag) {
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            setLogTag((CommunicationType)i, tag);
        }
    }
    
    // --- 标签日志级别管理 ---
    // 为特定标签设置日志级别
    static void setLogLevelForTag(const char* tag, int level);
    
    // 重置特定标签的日志级别
    static void resetLogLevelForTag(const char* tag);
    
    // 重置所有标签的日志级别
    static void resetAllTagLogLevels();
    
    static void update() {
        // 此函数保留用于未来的缓冲处理
    }
    
    // --- 日志记录方法 ---
    // 错误级别日志
    static void error(const char* format, ...);
    static void error(const char* tag, const char* format, ...);
    
    // 警告级别日志
    static void warning(const char* format, ...);
    static void warning(const char* tag, const char* format, ...);
    
    // 信息级别日志
    static void info(const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    
    // 调试级别日志
    static void debug(const char* format, ...);
    static void debug(const char* tag, const char* format, ...);
};

#endif // LOGGER_H 
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
    COMM_ESP,    // ESP
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
    
    // 内部日志处理函数
    static void logInternal(int level, const __FlashStringHelper* levelStr, const char* format, va_list args);
    
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
            commConfigs[i].config.usePrefix = (i != COMM_SERIAL);    // 默认在非Serial上使用前缀
            commConfigs[i].config.tag[0] = '\0';                     // 默认无标签
        }
        
        // 设置默认配置
        commConfigs[COMM_SERIAL].enabled = true;
        commConfigs[COMM_SERIAL].stream = &Serial;
        
        commConfigs[COMM_BT].enabled = ENABLE_BLUETOOTH;
        commConfigs[COMM_ESP].enabled = ENABLE_ESP;
        
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
    
    static void update() {
        // 此函数保留用于未来的缓冲处理
    }
    
    // 错误级别日志
    static void error(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_ERROR, LOG_LEVEL_ERROR_STR, format, args);
        va_end(args);
    }
    
    // 警告级别日志
    static void warning(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_WARNING, LOG_LEVEL_WARNING_STR, format, args);
        va_end(args);
    }
    
    // 信息级别日志
    static void info(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_INFO, LOG_LEVEL_INFO_STR, format, args);
        va_end(args);
    }
    
    // 调试级别日志
    static void debug(const char* format, ...) {
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_STR, format, args);
        va_end(args);
    }
    
    // 带标签的错误日志
    static void errorWithTag(const char* tag, const char* format, ...) {
        char tempTag[16];
        strncpy(tempTag, tag, sizeof(tempTag) - 1);
        tempTag[sizeof(tempTag) - 1] = '\0';
        
        // 临时保存原始标签
        char originalTags[COMM_COUNT][16];
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            strncpy(originalTags[i], commConfigs[i].config.tag, sizeof(originalTags[i]));
            setLogTag((CommunicationType)i, tag);
        }
        
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_ERROR, LOG_LEVEL_ERROR_STR, format, args);
        va_end(args);
        
        // 恢复原始标签
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            setLogTag((CommunicationType)i, originalTags[i]);
        }
    }
    
    // 带标签的警告日志
    static void warningWithTag(const char* tag, const char* format, ...) {
        char tempTag[16];
        strncpy(tempTag, tag, sizeof(tempTag) - 1);
        tempTag[sizeof(tempTag) - 1] = '\0';
        
        // 临时保存原始标签
        char originalTags[COMM_COUNT][16];
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            strncpy(originalTags[i], commConfigs[i].config.tag, sizeof(originalTags[i]));
            setLogTag((CommunicationType)i, tag);
        }
        
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_WARNING, LOG_LEVEL_WARNING_STR, format, args);
        va_end(args);
        
        // 恢复原始标签
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            setLogTag((CommunicationType)i, originalTags[i]);
        }
    }
    
    // 带标签的信息日志
    static void infoWithTag(const char* tag, const char* format, ...) {
        char tempTag[16];
        strncpy(tempTag, tag, sizeof(tempTag) - 1);
        tempTag[sizeof(tempTag) - 1] = '\0';
        
        // 临时保存原始标签
        char originalTags[COMM_COUNT][16];
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            strncpy(originalTags[i], commConfigs[i].config.tag, sizeof(originalTags[i]));
            setLogTag((CommunicationType)i, tag);
        }
        
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_INFO, LOG_LEVEL_INFO_STR, format, args);
        va_end(args);
        
        // 恢复原始标签
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            setLogTag((CommunicationType)i, originalTags[i]);
        }
    }
    
    // 带标签的调试日志
    static void debugWithTag(const char* tag, const char* format, ...) {
        char tempTag[16];
        strncpy(tempTag, tag, sizeof(tempTag) - 1);
        tempTag[sizeof(tempTag) - 1] = '\0';
        
        // 临时保存原始标签
        char originalTags[COMM_COUNT][16];
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            strncpy(originalTags[i], commConfigs[i].config.tag, sizeof(originalTags[i]));
            setLogTag((CommunicationType)i, tag);
        }
        
        va_list args;
        va_start(args, format);
        logInternal(LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_STR, format, args);
        va_end(args);
        
        // 恢复原始标签
        for (uint8_t i = 0; i < COMM_COUNT; i++) {
            setLogTag((CommunicationType)i, originalTags[i]);
        }
    }
};

#endif // LOGGER_H 
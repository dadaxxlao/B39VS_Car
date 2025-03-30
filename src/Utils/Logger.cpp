#include "Logger.h"

// 定义静态成员变量
char Logger::messageBuffer[256] = {0};
char Logger::timeBuffer[32] = {0};
CommConfig Logger::commConfigs[COMM_COUNT] = {};
unsigned long Logger::startTime = 0;

// 格式化时间戳
void Logger::formatTimestamp(char* buffer, size_t size) {
    unsigned long runtime = millis() - startTime;
    snprintf(buffer, size, "[%lu.%03lu]", runtime/1000, runtime%1000);
}

// 内部日志处理函数
void Logger::logInternal(int level, const __FlashStringHelper* levelStr, const char* format, va_list args) {
    // 格式化消息
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    // 处理每个通信通道
    for (uint8_t i = 0; i < COMM_COUNT; i++) {
        // 检查通道是否启用、流是否存在、日志级别是否满足
        if (!commConfigs[i].enabled || !commConfigs[i].stream || commConfigs[i].config.logLevel < level) {
            continue;
        }
        
        Stream* stream = commConfigs[i].stream;
        
        // 如果使用前缀（非Serial格式）
        if (commConfigs[i].config.usePrefix) {
            stream->print(F("$LOG:"));
            stream->print(levelStr);
            
            // 如果有标签，添加标签
            if (commConfigs[i].config.tag[0] != '\0') {
                stream->print(F(","));
                stream->print(commConfigs[i].config.tag);
            }
            
            stream->print(F(","));
            stream->println(messageBuffer);
        }
        // 否则使用标准格式（通常用于Serial）
        else {
            // 添加时间戳（如果配置了）
            if (commConfigs[i].config.useTimestamp) {
                formatTimestamp(timeBuffer, sizeof(timeBuffer));
                stream->print(timeBuffer);
                stream->print(' ');
            }
            
            // 添加日志级别
            stream->print('[');
            stream->print(levelStr);
            stream->print(']');
            
            // 如果有标签，添加标签
            if (commConfigs[i].config.tag[0] != '\0') {
                stream->print('[');
                stream->print(commConfigs[i].config.tag);
                stream->print(']');
            }
            
            stream->print(' ');
            stream->println(messageBuffer);
        }
    }
} 
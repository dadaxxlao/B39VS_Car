#include "Logger.h"
#include <string.h> // 用于strcmp, strncpy

// 定义静态成员变量
char Logger::messageBuffer[256] = {0};
char Logger::timeBuffer[32] = {0};
CommConfig Logger::commConfigs[COMM_COUNT] = {};
unsigned long Logger::startTime = 0;
Logger::TagLogLevel Logger::tagLogLevels[Logger::MAX_TAGS] = {};
int Logger::tagLogLevelCount = 0;

// 格式化时间戳
void Logger::formatTimestamp(char* buffer, size_t size) {
    unsigned long runtime = millis() - startTime;
    snprintf(buffer, size, "[%lu.%03lu]", runtime/1000, runtime%1000);
}

// 获取指定标签的日志级别
int Logger::getLogLevelForTag(const char* tag) {
    if (!tag || tag[0] == '\0') return LOG_LEVEL_DEBUG + 1; // 无标签，默认最高级别
    
    for (int i = 0; i < tagLogLevelCount; ++i) {
        if (strcmp(tagLogLevels[i].name, tag) == 0) {
            return tagLogLevels[i].level;
        }
    }
    // 未找到标签，默认最高级别
    return LOG_LEVEL_DEBUG + 1;
}

// 为特定标签设置日志级别
void Logger::setLogLevelForTag(const char* tag, int level) {
    if (!tag || tag[0] == '\0') return; // 忽略空标签
    
    // 检查标签是否已存在
    for (int i = 0; i < tagLogLevelCount; ++i) {
        if (strcmp(tagLogLevels[i].name, tag) == 0) {
            tagLogLevels[i].level = level;
            return;
        }
    }
    
    // 标签不存在，如果有空间则添加
    if (tagLogLevelCount < MAX_TAGS) {
        strncpy(tagLogLevels[tagLogLevelCount].name, tag, sizeof(tagLogLevels[0].name) - 1);
        tagLogLevels[tagLogLevelCount].name[sizeof(tagLogLevels[0].name) - 1] = '\0'; // 确保以空字符结尾
        tagLogLevels[tagLogLevelCount].level = level;
        tagLogLevelCount++;
    }
}

// 重置特定标签的日志级别
void Logger::resetLogLevelForTag(const char* tag) {
    if (!tag) return;
    for (int i = 0; i < tagLogLevelCount; ++i) {
        if (strcmp(tagLogLevels[i].name, tag) == 0) {
            // 通过将最后一个元素移至此位置来删除此条目
            if (i < tagLogLevelCount - 1) {
                tagLogLevels[i] = tagLogLevels[tagLogLevelCount - 1];
            }
            tagLogLevelCount--;
            return;
        }
    }
}

// 重置所有标签的日志级别
void Logger::resetAllTagLogLevels() {
    tagLogLevelCount = 0;
}

// 内部日志处理函数
void Logger::logInternal(int level, const __FlashStringHelper* levelStr, const char* tag, const char* format, va_list args) {
    // 检查是否有任何通道需要此日志级别（优化）
    bool needed = false;
    for (uint8_t i = 0; i < COMM_COUNT; i++) {
        if (commConfigs[i].enabled && commConfigs[i].stream && level <= commConfigs[i].config.logLevel) {
            // 进一步检查标签级别
            int tagLevel = getLogLevelForTag(tag);
            if (level <= tagLevel) {
                needed = true;
                break;
            }
        }
    }
    if (!needed) {
        return; // 根据级别/标签过滤，没有通道需要此消息
    }
    
    // 格式化消息（仅在需要时）
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);
    
    // 处理每个通信通道
    for (uint8_t i = 0; i < COMM_COUNT; i++) {
        // 检查通道是否启用、流是否存在、通道日志级别是否满足
        if (!commConfigs[i].enabled || !commConfigs[i].stream || level > commConfigs[i].config.logLevel) {
            continue;
        }
        
        // 检查标签特定的日志级别
        int tagLevel = getLogLevelForTag(tag);
        if (level > tagLevel) {
            continue; // 消息级别低于此标签所需级别
        }
        
        Stream* stream = commConfigs[i].stream;
        const LoggerConfig& config = commConfigs[i].config; // 使用引用以便更清晰地访问
        
        // 确定要打印的标签：如果提供了特定标签，则使用它，否则使用通道的默认标签
        const char* effectiveTag = (tag && tag[0] != '\0') ? tag : config.tag;
        
        // 如果使用前缀（非Serial格式）
        if (config.usePrefix) {
            stream->print(F("$LOG:"));
            stream->print(levelStr);
            
            // 如果有有效标签，添加它
            if (effectiveTag[0] != '\0') {
                stream->print(F(",["));
                stream->print(effectiveTag);
                stream->print(F("]"));
            }
            
            stream->print(F(","));
            stream->println(messageBuffer);
        }
        // 否则使用标准格式（通常用于Serial）
        else {
            // 添加时间戳（如果配置了）
            if (config.useTimestamp) {
                formatTimestamp(timeBuffer, sizeof(timeBuffer));
                stream->print(timeBuffer);
                stream->print(' ');
            }
            
            // 添加日志级别
            stream->print('[');
            stream->print(levelStr);
            stream->print(']');
            
            // 如果有有效标签，添加它
            if (effectiveTag[0] != '\0') {
                stream->print('[');
                stream->print(effectiveTag);
                stream->print(']');
            }
            
            stream->print(' ');
            stream->println(messageBuffer);
        }
    }
}

// --- 错误级别日志 ---
void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_ERROR, LOG_LEVEL_ERROR_STR, nullptr, format, args);
    va_end(args);
}

void Logger::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_ERROR, LOG_LEVEL_ERROR_STR, tag, format, args);
    va_end(args);
}

// --- 警告级别日志 ---
void Logger::warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_WARNING, LOG_LEVEL_WARNING_STR, nullptr, format, args);
    va_end(args);
}

void Logger::warning(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_WARNING, LOG_LEVEL_WARNING_STR, tag, format, args);
    va_end(args);
}

// --- 信息级别日志 ---
void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_INFO, LOG_LEVEL_INFO_STR, nullptr, format, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_INFO, LOG_LEVEL_INFO_STR, tag, format, args);
    va_end(args);
}

// --- 调试级别日志 ---
void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_STR, nullptr, format, args);
    va_end(args);
}

void Logger::debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_DEBUG, LOG_LEVEL_DEBUG_STR, tag, format, args);
    va_end(args);
} 
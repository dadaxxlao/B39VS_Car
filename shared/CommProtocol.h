#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

/**
 * 通信协议定义
 * Arduino Mega 与 ESP32C3 之间的通信协议
 */

// 消息类型前缀
#define PREFIX_CMD    "$CMD:"    // 命令消息前缀
#define PREFIX_STATE  "$STATE:"  // 状态消息前缀
#define PREFIX_LOG    "$LOG:"    // 日志消息前缀
#define PREFIX_DATA   "$DATA:"   // 数据消息前缀
#define MSG_TERMINATOR '#'       // 消息结束标记

// 命令类型枚举
enum CommandType {
    CMD_START = 0,       // 开始任务
    CMD_STOP = 1,        // 停止任务
    CMD_RESET = 2,       // 重置系统
    CMD_CONFIG = 3,      // 配置系统
    CMD_GET_STATUS = 4,  // 获取状态
    CMD_CUSTOM_MOTION = 5, // 自定义运动
    CMD_UNKNOWN = 99     // 未知命令
};

// 系统状态枚举
enum SystemStateCode {
    STATE_IDLE = 0,           // 空闲
    STATE_RUNNING = 1,        // 正在运行
    STATE_ERROR = 2,          // 错误
    STATE_OBJECT_FIND = 3,    // 寻找物体
    STATE_OBJECT_GRAB = 4,    // 抓取物体
    STATE_OBJECT_PLACE = 5,   // 放置物体
    STATE_RETURN_BASE = 6,    // 返回基地
    STATE_UNKNOWN = 99        // 未知状态
};

// 日志级别
enum LogLevel {
    LOG_ERROR = 0,     // 错误
    LOG_WARNING = 1,   // 警告
    LOG_INFO = 2,      // 信息
    LOG_DEBUG = 3      // 调试
};

// 数据类型
enum DataType {
    DATA_SENSORS = 0,    // 传感器数据
    DATA_POSITION = 1,   // 位置数据
    DATA_BATTERY = 2,    // 电池数据
    DATA_CUSTOM = 99     // 自定义数据
};

// 字符串转换辅助函数
#ifdef ESP32_FIRMWARE
// 仅在ESP32固件中实现这些函数
inline const char* commandTypeToString(CommandType cmd) {
    switch (cmd) {
        case CMD_START: return "START";
        case CMD_STOP: return "STOP";
        case CMD_RESET: return "RESET";
        case CMD_CONFIG: return "CONFIG";
        case CMD_GET_STATUS: return "GET_STATUS";
        case CMD_CUSTOM_MOTION: return "CUSTOM_MOTION";
        default: return "UNKNOWN";
    }
}

inline const char* stateCodeToString(SystemStateCode state) {
    switch (state) {
        case STATE_IDLE: return "IDLE";
        case STATE_RUNNING: return "RUNNING";
        case STATE_ERROR: return "ERROR";
        case STATE_OBJECT_FIND: return "FINDING";
        case STATE_OBJECT_GRAB: return "GRABBING";
        case STATE_OBJECT_PLACE: return "PLACING";
        case STATE_RETURN_BASE: return "RETURNING";
        default: return "UNKNOWN";
    }
}

inline const char* logLevelToString(LogLevel level) {
    switch (level) {
        case LOG_ERROR: return "ERROR";
        case LOG_WARNING: return "WARNING";
        case LOG_INFO: return "INFO";
        case LOG_DEBUG: return "DEBUG";
        default: return "UNKNOWN";
    }
}

inline const char* dataTypeToString(DataType type) {
    switch (type) {
        case DATA_SENSORS: return "SENSORS";
        case DATA_POSITION: return "POSITION";
        case DATA_BATTERY: return "BATTERY";
        case DATA_CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}
#endif // ESP32_FIRMWARE

#endif // COMM_PROTOCOL_H 
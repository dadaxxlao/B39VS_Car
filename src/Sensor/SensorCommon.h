#ifndef SENSOR_COMMON_H
#define SENSOR_COMMON_H

#include <Arduino.h>

// 传感器类型枚举
enum class SensorType {
    ULTRASONIC,
    INFRARED_ARRAY,
    COLOR
};

// 传感器状态枚举
enum class SensorStatus {
    OK,                // 传感器工作正常
    NOT_INITIALIZED,   // 传感器未初始化
    ERROR_COMM,        // 通信错误 (I2C NACK等)
    ERROR_TIMEOUT,     // 超时 (pulseIn等)
    ERROR_CONFIG,      // 配置错误
    UNKNOWN            // 未知状态
};

// 常量定义
const int INFRARED_NO_LINE = 0;  // 未检测到线时的返回值
const unsigned long ULTRASONIC_PULSE_TIMEOUT = 30000; // 30ms超声波脉冲超时

#endif // SENSOR_COMMON_H 
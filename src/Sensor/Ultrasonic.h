#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>
#include "../Utils/Config.h"
#include "SensorCommon.h"

class UltrasonicSensor {
private:
    uint8_t trigPin;  // 触发引脚
    uint8_t echoPin;  // 回声引脚
    bool initialized; // 初始化状态
    unsigned long lastPulseDuration; // 上次测量的脉冲时长
    
    // 距离计算函数
    float calculateDistance(unsigned long duration);
    
public:
    UltrasonicSensor();
    
    // 初始化超声波传感器 - 不再需要参数
    bool init();
    
    // 获取初始化状态
    bool isInitialized() const;
    
    // 检查传感器健康状态
    SensorStatus checkHealth();
    
    // 测量脉冲时长
    unsigned long measurePulseDuration();
    
    // 从脉冲时长计算距离
    float getDistanceCmFromDuration(unsigned long duration);
    
    // 获取距离（厘米）- 保持向后兼容
    float getDistance();
    
    // 判断是否有障碍物在指定距离内
    bool isObstacleDetected(float threshold);
    
    // 调试打印
    void debugPrint();
};

#endif // ULTRASONIC_H 
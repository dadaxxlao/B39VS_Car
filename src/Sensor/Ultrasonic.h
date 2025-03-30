#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

class UltrasonicSensor {
private:
    uint8_t trigPin;  // 触发引脚
    uint8_t echoPin;  // 回声引脚
    float lastValidDistance; // 上次有效距离
    
    // 距离计算函数
    float calculateDistance(unsigned long duration);
    
public:
    UltrasonicSensor();
    
    // 初始化超声波传感器
    void init(uint8_t trigPin, uint8_t echoPin);
    
    // 获取距离（厘米，已过滤异常值）
    float getDistance();
    
    // 判断是否有障碍物在指定距离内
    bool isObstacleInRange(float threshold);
    
#ifdef DEBUG_ULTRASONIC
    // 调试方法
    void printDebugInfo();
    float getRawDistance(); // 获取未过滤的原始距离
#endif
};

#endif // ULTRASONIC_H 
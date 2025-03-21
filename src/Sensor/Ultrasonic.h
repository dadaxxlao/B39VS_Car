#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

class UltrasonicSensor {
private:
    uint8_t trigPin;  // 触发引脚
    uint8_t echoPin;  // 回声引脚
    
    // 距离计算函数
    float calculateDistance(unsigned long duration);
    
public:
    UltrasonicSensor();
    
    // 初始化超声波传感器
    void init(uint8_t trigPin, uint8_t echoPin);
    
    // 获取距离（厘米）
    float getDistance();
    
    // 判断是否有障碍物在指定距离内
    bool isObstacleDetected(float threshold);
};

#endif // ULTRASONIC_H 
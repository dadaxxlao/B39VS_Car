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
    
    /**
     * @brief 获取稳定过滤后的距离读数（厘米）。
     *
     * 该函数进行多次测量，去除最大和最小值，然后计算剩余值的平均值，
     * 以减少瞬时噪声或异常读数的影响。
     *
     * @param samples 要进行的测量次数 (默认 10)。
     * @param delayMs 每次测量之间的延迟时间（毫秒）(默认 10)。
     * @return float 稳定后的平均距离（厘米）。如果有效读数少于3个，返回 -1.0。
     */
    float getStableDistanceCm(int samples = 10, int delayMs = 2);

    // 调试打印
    void debugPrint();
};

#endif // ULTRASONIC_H 
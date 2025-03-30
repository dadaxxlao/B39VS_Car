#include "Ultrasonic.h"
#include "../Utils/Logger.h"

UltrasonicSensor::UltrasonicSensor() : trigPin(0), echoPin(0), lastValidDistance(0) {
}

void UltrasonicSensor::init(uint8_t trig, uint8_t echo) {
    trigPin = trig;
    echoPin = echo;
    
    // 设置引脚模式
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    // 确保触发引脚初始状态为低电平
    digitalWrite(trigPin, LOW);
    
    Logger::info("超声波传感器初始化完成");
}

float UltrasonicSensor::getDistance() {
    // 确保触发引脚为低电平
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // 发送10微秒的高电平脉冲
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // 读取回波时间（微秒）
    unsigned long duration = pulseIn(echoPin, HIGH, 30000); // 30ms超时
    
    // 计算距离（厘米）
    float distance = calculateDistance(duration);
    
    // 异常值过滤
    if (distance <= 0 || distance > 400) {
        // 返回上次有效值
#ifdef DEBUG_ULTRASONIC
        Logger::warning("超声波测量异常值: %.2f，使用上次值: %.2f", distance, lastValidDistance);
#endif
        return lastValidDistance;
    }
    
    // 更新上次有效值
    lastValidDistance = distance;
    
#ifdef DEBUG_ULTRASONIC
    Logger::debug("超声波距离: %.2f cm", distance);
#endif
    
    return distance;
}

float UltrasonicSensor::calculateDistance(unsigned long duration) {
    // 声速约为340米/秒或34000厘米/秒
    // 距离 = 时间 * 速度 / 2（来回距离）
    // 转换为厘米：duration(微秒) * 0.034 / 2
    
    if (duration == 0) {
        // 超时情况
        return -1;
    }
    
    return duration * 0.034 / 2;
}

bool UltrasonicSensor::isObstacleInRange(float threshold) {
    float distance = getDistance();
    
    // 检查是否有有效的距离测量
    if (distance <= 0) {
        return false;
    }
    
    // 检查是否在阈值范围内有障碍物
    return distance <= threshold;
}

#ifdef DEBUG_ULTRASONIC
// 调试方法
void UltrasonicSensor::printDebugInfo() {
    float rawDistance = getRawDistance();
    float filteredDistance = getDistance();
    
    Logger::debug("=== 超声波传感器调试信息 ===");
    Logger::debug("原始距离: %.2f cm", rawDistance);
    Logger::debug("过滤后距离: %.2f cm", filteredDistance);
    Logger::debug("距离有效: %s", (rawDistance > 0 && rawDistance <= 400) ? "是" : "否");
    Logger::debug("有障碍物(10cm内): %s", isObstacleInRange(10) ? "是" : "否");
    Logger::debug("============================");
}

float UltrasonicSensor::getRawDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    unsigned long duration = pulseIn(echoPin, HIGH, 30000);
    return calculateDistance(duration);
}
#endif 
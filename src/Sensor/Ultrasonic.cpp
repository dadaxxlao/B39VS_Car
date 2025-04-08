#include "Ultrasonic.h"
#include "../Utils/Logger.h"

UltrasonicSensor::UltrasonicSensor() : trigPin(0), echoPin(0), initialized(false), lastPulseDuration(0) {
}

bool UltrasonicSensor::init() {
    // 从Config.h读取引脚配置
    trigPin = ULTRASONIC_TRIG_PIN;
    echoPin = ULTRASONIC_ECHO_PIN;
    
    // 检查引脚配置是否有效
    if (trigPin == 0 || echoPin == 0) {
        Logger::error("Ultrasonic", "无效的超声波引脚配置!");
        initialized = false;
        return false;
    }
    
    // 设置引脚模式
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    // 确保触发引脚初始状态为低电平
    digitalWrite(trigPin, LOW);
    
    initialized = true;
    Logger::info("Ultrasonic", "超声波传感器初始化完成 (Trig: %d, Echo: %d)", trigPin, echoPin);
    return true;
}

bool UltrasonicSensor::isInitialized() const {
    return initialized;
}

SensorStatus UltrasonicSensor::checkHealth() {
    if (!initialized) return SensorStatus::NOT_INITIALIZED;
    
    // 简化版本：只检查是否初始化
    // 可以扩展为实际测试一次脉冲
    return SensorStatus::OK;
}

unsigned long UltrasonicSensor::measurePulseDuration() {
    if (!initialized) {
        Logger::warning("Ultrasonic", "尝试在未初始化的状态下进行测量");
        return 0;
    }
    
    // 确保触发引脚为低电平
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    
    // 发送10微秒的高电平脉冲
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // 读取回波时间（微秒）
    lastPulseDuration = pulseIn(echoPin, HIGH, ULTRASONIC_PULSE_TIMEOUT);
    
    if (lastPulseDuration == 0) {
    //    Logger::warning("Ultrasonic", "超声波脉冲检测超时");
    }
    
    Logger::debug("Ultrasonic", "原始脉冲时长: %lu us", lastPulseDuration);
    return lastPulseDuration;
}

float UltrasonicSensor::getDistanceCmFromDuration(unsigned long duration) {
    return calculateDistance(duration);
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

float UltrasonicSensor::getDistance() {
    // 向后兼容的方法，内部使用新的measurePulseDuration方法
    unsigned long duration = measurePulseDuration();
    return calculateDistance(duration);
}

bool UltrasonicSensor::isObstacleDetected(float threshold) {
    float distance = getDistance();
    
    // 检查是否有有效的距离测量
    if (distance <= 0) {
        return false;
    }
    
    // 检查是否在阈值范围内有障碍物
    return distance <= threshold;
}

void UltrasonicSensor::debugPrint() {
    if (!initialized) {
        Logger::debug("Ultrasonic", "传感器未初始化");
        return;
    }
    
    float distance = calculateDistance(lastPulseDuration);
    Logger::debug("Ultrasonic", "状态: %s, Trig: %d, Echo: %d, 最近脉冲: %lu us, 计算距离: %.2f cm",
                 initialized ? "已初始化" : "未初始化", trigPin, echoPin, 
                 lastPulseDuration, distance);
} 
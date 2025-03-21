#include "SensorManager.h"
#include "../Utils/Logger.h"

SensorManager::SensorManager() : lastDistance(0), lastColor(COLOR_UNKNOWN) {
}

void SensorManager::initAllSensors() {
    // 初始化红外线传感器
    if (!infraredSensor.begin(INFRARED_ARRAY_ADDR)) {
        Logger::error("红外线传感器初始化失败");
    }
    
    // 初始化超声波传感器
    ultrasonicSensor.init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
    
    // 初始化颜色传感器
    if (!colorSensor.begin(COLOR_SENSOR_ADDR)) {
        Logger::error("颜色传感器初始化失败");
    }
    
    Logger::info("传感器初始化完成");
}

float SensorManager::getUltrasonicDistance() {
    float distance = ultrasonicSensor.getDistance();
    
    // 简单的异常值过滤
    if (distance <= 0 || distance > 400) {
        // 测量值异常，返回上次有效值
        Logger::warning("超声波测量异常值: %.2f，使用上次值: %.2f", distance, lastDistance);
        return lastDistance;
    }
    
    // 更新上次有效值
    lastDistance = distance;
    return distance;
}

int SensorManager::getLinePosition() {
    return infraredSensor.getLinePosition();
}

const uint16_t* SensorManager::getInfraredSensorValues() {
    return infraredSensor.getAllSensorValues();
}

bool SensorManager::isLineDetected() {
    return infraredSensor.isLineDetected();
}

ColorCode SensorManager::getColor() {
    ColorCode color = colorSensor.readColor();
    
    // 颜色检测可能需要多次采样确认
    if (color != lastColor) {
        // 颜色变化，可能需要确认
        // 这里简化处理，直接更新
        lastColor = color;
    }
    
    // 在此添加一些处理逻辑，确保颜色编码匹配新的定义
    // 例如，如果colorSensor.readColor()返回的不是Config.h定义的枚举值，需要进行转换
    
    return color;
}

void SensorManager::getColorSensorValues(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c) {
    colorSensor.getRGB(r, g, b, c);
}

void SensorManager::debugColorSensor() {
    colorSensor.debugPrint();
}

void SensorManager::update() {
    // 定期更新所有传感器数据
    // 这可以在主循环中调用，以确保数据的实时性
    
    // 更新颜色传感器数据
    colorSensor.update();
    
    // 更新红外线传感器数据
    infraredSensor.update();
} 
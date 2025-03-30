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

void SensorManager::update() {
    // 更新所有传感器数据
    colorSensor.update();
    infraredSensor.update();
    // 超声波按需更新，不在这里调用
}

float SensorManager::getDistance() {
    return ultrasonicSensor.getDistance();
}

bool SensorManager::isObstacleDetected(float threshold) {
    return ultrasonicSensor.isObstacleInRange(threshold);
}

int SensorManager::getLinePosition() {
    return infraredSensor.getLinePosition();
}

bool SensorManager::isLineDetected() {
    return infraredSensor.isLineDetected();
}

ColorCode SensorManager::getColor() {
    ColorCode color = colorSensor.readColor();
    
    // 颜色检测可能需要多次采样确认
    if (color != lastColor) {
        // 颜色变化，可能需要确认
        // 这里保留简单实现，只更新lastColor
        lastColor = color;
    }
    
    return color;
}

#ifdef DEBUG_MODE
// 调试方法实现
void SensorManager::debugAllSensors() {
    Logger::debug("====== 所有传感器状态 ======");
    Logger::debug("距离: %.2f cm", getDistance());
    Logger::debug("线位置: %d", getLinePosition());
    Logger::debug("检测到线: %s", isLineDetected() ? "是" : "否");
    
    ColorCode color = getColor();
    const char* colorName = "未知";
    switch (color) {
        case COLOR_RED: colorName = "红色"; break;
        case COLOR_BLUE: colorName = "蓝色"; break;
        case COLOR_YELLOW: colorName = "黄色"; break;
        case COLOR_WHITE: colorName = "白色"; break;
        case COLOR_BLACK: colorName = "黑色"; break;
        default: colorName = "未知"; break;
    }
    Logger::debug("颜色: %s", colorName);
    Logger::debug("============================");
}

void SensorManager::debugInfrared() {
#ifdef DEBUG_INFRARED
    infraredSensor.printDebugInfo();
#else
    Logger::warning("红外传感器调试模式未启用");
#endif
}

void SensorManager::debugUltrasonic() {
#ifdef DEBUG_ULTRASONIC
    ultrasonicSensor.printDebugInfo();
#else
    Logger::warning("超声波传感器调试模式未启用");
#endif
}

void SensorManager::debugColorSensor() {
#ifdef DEBUG_COLOR_SENSOR
    colorSensor.printDebugInfo();
#else
    Logger::warning("颜色传感器调试模式未启用");
#endif
}

const uint16_t* SensorManager::getInfraredSensorValues() {
    return infraredSensor.getAllSensorValues();
}

void SensorManager::getColorSensorValues(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c) {
    colorSensor.getRGB(r, g, b, c);
}

void SensorManager::setColorLED(bool on) {
    colorSensor.setLED(on);
}

void SensorManager::calibrateColor(ColorCode color) {
#ifdef DEBUG_COLOR_SENSOR
    colorSensor.calibrateColor(color);
#else
    Logger::warning("颜色传感器调试模式未启用，无法校准");
#endif
}
#endif 
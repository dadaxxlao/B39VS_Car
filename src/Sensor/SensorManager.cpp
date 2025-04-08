#include "SensorManager.h"
#include "../Utils/Logger.h"

SensorManager::SensorManager() 
    : allSensorsInitialized(false), 
      lastValidDistance(0), 
      lastValidLinePosition(0), 
      lastValidColor(COLOR_UNKNOWN) {
    // 初始化缓存数组
    for (int i = 0; i < 8; i++) {
        lastInfraredValues[i] = 0;
    }
}

bool SensorManager::initAllSensors() {
    bool allSuccess = true;
    
    // 初始化红外线传感器
    if (!infraredSensor.begin()) {
        Logger::error("SensorMgr", "红外线传感器初始化失败");
        allSuccess = false;
    }
    
    // 初始化超声波传感器
    if (!ultrasonicSensor.init()) {
        Logger::error("SensorMgr", "超声波传感器初始化失败");
        allSuccess = false;
    }
    
    // 初始化颜色传感器
    if (!colorSensor.begin()) {
        Logger::error("SensorMgr", "颜色传感器初始化失败");
        allSuccess = false;
    }
    

    allSensorsInitialized = allSuccess;
    
    if (allSuccess) {
        Logger::info("SensorMgr", "所有传感器初始化成功");
    } else {
        Logger::warning("SensorMgr", "部分传感器初始化失败");
    }
    
    
    return allSuccess;
}

bool SensorManager::areAllSensorsInitialized() const {
    return allSensorsInitialized;
}

bool SensorManager::isSensorInitialized(SensorType type) const {
    switch (type) {
        case SensorType::ULTRASONIC:
            return ultrasonicSensor.isInitialized();
        case SensorType::INFRARED_ARRAY:
            return infraredSensor.isInitialized();
        case SensorType::COLOR:
            return colorSensor.isInitialized();
        default:
            return false;
    }
}

SensorStatus SensorManager::getSensorHealth(SensorType type) {
    switch (type) {
        case SensorType::ULTRASONIC:
            return ultrasonicSensor.checkHealth();
        case SensorType::INFRARED_ARRAY:
            return infraredSensor.checkHealth();
        case SensorType::COLOR:
            return colorSensor.checkHealth();
        default:
            return SensorStatus::UNKNOWN;
    }
}

bool SensorManager::checkAllSensorsHealth() {
    bool allHealthy = true;
    
    // 检查超声波传感器
    SensorStatus ultrasonicStatus = ultrasonicSensor.checkHealth();
    if (ultrasonicStatus != SensorStatus::OK) {
        Logger::warning("SensorMgr", "超声波传感器状态异常: %d", (int)ultrasonicStatus);
        allHealthy = false;
    }
    
    // 检查红外传感器
    SensorStatus infraredStatus = infraredSensor.checkHealth();
    if (infraredStatus != SensorStatus::OK) {
        Logger::warning("SensorMgr", "红外传感器状态异常: %d", (int)infraredStatus);
        allHealthy = false;
    }
    
    // 检查颜色传感器
    SensorStatus colorStatus = getSensorHealth(SensorType::COLOR);
    if (colorStatus != SensorStatus::OK) {
        Logger::warning("SensorMgr", "颜色传感器状态异常: %d", (int)colorStatus);
        allHealthy = false;
    }
    return allHealthy;
}

void SensorManager::updateAll() {
    // 更新各个传感器数据
    updateInfrared();
    updateColor();
    
    // 主动更新超声波传感器
    // 超声波传感器在状态机中很重要，应当主动更新
    float distance;
    getDistanceCm(distance); // 这会调用measurePulseDuration()更新数据
}

void SensorManager::updateInfrared() {
    infraredSensor.update();
    
    // 缓存传感器值
    infraredSensor.getAllSensorValues(lastInfraredValues);
    
    // 获取并缓存线位置
    int position;
    if (infraredSensor.getLinePosition(position)) {
        lastValidLinePosition = position;
    }
}

void SensorManager::updateColor() {
    // 颜色传感器不需要主动调用update，由各方法自动更新
    // 保留此方法以兼容现有代码
}

bool SensorManager::getDistanceCm(float& distance) {
    if (!ultrasonicSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的超声波传感器获取距离");
        return false;
    }
    
    // 测量新的距离
    unsigned long duration = ultrasonicSensor.measurePulseDuration();
    float newDistance = ultrasonicSensor.getDistanceCmFromDuration(duration);
    
    // 简单的异常值过滤
    if (newDistance <= 0 || newDistance > 1000) {
        // 测量值异常，返回上次有效值
        // Logger::warning("SensorMgr", "超声波测量异常值: %f，使用上次值: %f", 
        //                 newDistance, lastValidDistance);
        
        // 如果有上次有效值，则使用它
        if (lastValidDistance > 0) {
            distance = lastValidDistance;
            return true;
        }
        
        return false; // 无有效值
    }
    
    // 更新上次有效值
    lastValidDistance = newDistance;
    distance = newDistance;
    
    return true;
}

float SensorManager::getUltrasonicDistance() {
    // 向后兼容方法
    float distance = 0;
    if (getDistanceCm(distance)) {
        return distance;
    }
    return lastValidDistance; // 返回最后有效值或0
}

bool SensorManager::isObstacleDetected(float threshold) {
    if (!ultrasonicSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的超声波传感器检测障碍物");
        return false;
    }
    
    float distance;
    if (getDistanceCm(distance)) {
        return distance <= threshold;
    }
    return false; // 如果测量失败，假设没有障碍物
}

bool SensorManager::getLinePosition(int& position) {
    // 首先检查传感器是否初始化
    if (!infraredSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的红外传感器获取线位置");
        return false;
    }
    
    // 使用红外传感器的新方法
    return infraredSensor.getLinePosition(position);
}

int SensorManager::getLinePosition() {
    // 向后兼容方法
    int position;
    if (getLinePosition(position)) {
        return position;
    }
    return INFRARED_NO_LINE; // 未检测到线的特殊值
}

bool SensorManager::getInfraredSensorValues(uint16_t values[8]) {
    if (!infraredSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的红外传感器获取传感器值");
        return false;
    }
    
    // 使用新方法直接填充数组
    infraredSensor.getAllSensorValues(values);
    return true;
}

const uint16_t* SensorManager::getInfraredSensorValues() {
    // 向后兼容方法
    return infraredSensor.getAllSensorValues();
}

bool SensorManager::isLineDetected() {
    if (!infraredSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的红外传感器检测线");
        return false;
    }
    return infraredSensor.isLineDetected();
}

ColorCode SensorManager::getColor() {
    // 检查传感器是否初始化
    if (!colorSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的颜色传感器获取颜色");
        return COLOR_UNKNOWN;
    }
    
    // 读取颜色
    ColorCode current_color = colorSensor.getColor();
    
    // 更新缓存的颜色值
    lastValidColor = current_color;
    
    return lastValidColor;
}

bool SensorManager::getColorSensorRGB(uint8_t& r, uint8_t& g, uint8_t& b) {
    // 检查传感器是否初始化
    if (!colorSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的颜色传感器获取RGB值");
        return false;
    }
    
    // 调用颜色传感器的RGB方法
    return colorSensor.getColorRGB(r, g, b);
}

bool SensorManager::getColorSensorHSL(uint8_t& h, uint8_t& s, uint8_t& l) {
    // 检查传感器是否初始化
    if (!colorSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的颜色传感器获取HSL值");
        return false;
    }
    
    // 调用颜色传感器的HSL方法
    return colorSensor.getColorHSL(h, s, l);
}

void SensorManager::printSensorDebugInfo(SensorType type) {
    bool isInit = isSensorInitialized(type);
    if (!isInit) {
        Logger::warning("SensorMgr", "尝试打印未初始化的传感器(%d)调试信息", (int)type);
    }
    
    switch (type) {
        case SensorType::ULTRASONIC:
            ultrasonicSensor.debugPrint();
            break;
        case SensorType::INFRARED_ARRAY:
            infraredSensor.debugPrint();
            break;
        case SensorType::COLOR:
            colorSensor.debugPrint();
            break;
        default:
            Logger::warning("SensorMgr", "未知的传感器类型: %d", (int)type);
            break;
    }
}

void SensorManager::printAllDebugInfo() {
    Logger::debug("SensorMgr", "====== 所有传感器调试信息 ======");
    Logger::debug("SensorMgr", "初始化状态: %s", 
                 allSensorsInitialized ? "全部初始化" : "部分未初始化");
    
    Logger::debug("SensorMgr", "--- 超声波传感器 ---");
    ultrasonicSensor.debugPrint();
    
    Logger::debug("SensorMgr", "--- 红外传感器 ---");
    infraredSensor.debugPrint();
    
    Logger::debug("SensorMgr", "--- 颜色传感器 ---");
    colorSensor.debugPrint();
    
    Logger::debug("SensorMgr", "=============================");
}

void SensorManager::debugColorSensor() {
    // 向后兼容方法
    if (!colorSensor.isInitialized()) {
        Logger::warning("SensorMgr", "尝试从未初始化的颜色传感器打印调试信息");
    }
    colorSensor.debugPrint();
}


 
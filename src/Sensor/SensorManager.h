#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../Utils/Config.h"
#include "Infrared.h"
#include "Ultrasonic.h"
#include "ColorSensor.h"
#include "SensorCommon.h"

class SensorManager {
private:
    InfraredArray infraredSensor;
    UltrasonicSensor ultrasonicSensor;
    ColorSensor colorSensor;
    
    // 内部状态标志
    bool allSensorsInitialized;
    
    // 缓存的数据
    float lastValidDistance;   // 上次测量的有效距离值（用于过滤异常值）
    int lastValidLinePosition; // 上次检测到的有效线位置
    ColorCode lastValidColor;  // 上次检测到的有效颜色
    uint16_t lastInfraredValues[8]; // 缓存的红外传感器值
    
    // 内部更新函数
    void updateInfrared();
    void updateColor();
    
public:
    SensorManager();
    
    // === 生命周期与状态管理 ===
    
    // 初始化所有传感器
    bool initAllSensors();
    
    // 检查所有传感器是否已初始化
    bool areAllSensorsInitialized() const;
    
    // 检查特定传感器是否已初始化
    bool isSensorInitialized(SensorType type) const;
    
    // 获取特定传感器的健康状态
    SensorStatus getSensorHealth(SensorType type);
    
    // 检查所有传感器的健康状况
    bool checkAllSensorsHealth();
    
    // 更新所有传感器数据（在主循环中调用）
    void updateAll();
    
    // === 传感器数据访问 ===
    
    // --- 超声波传感器 ---
    
    // 获取超声波测量的距离（厘米）
    // 返回通过引用参数，函数返回值表示是否获取成功
    bool getDistanceCm(float& distance);
    
    // 获取超声波测量的距离（厘米）- 向后兼容方法
    // DEPRECATED: 请使用 getDistanceCm(float& distance) 替代
    float getUltrasonicDistance();
    
    // 判断是否有障碍物在指定距离内
    bool isObstacleDetected(float threshold);
    
    // --- 红外传感器 ---
    
    // 获取红外线传感器检测的线位置 - 新方法，通过引用返回
    bool getLinePosition(int& position);
    
    // 获取红外线传感器检测的线位置 - 向后兼容方法
    // DEPRECATED: 请使用 getLinePosition(int& position) 替代
    int getLinePosition();
    
    // 获取红外传感器原始数据到提供的数组
    bool getInfraredSensorValues(uint16_t values[8]);
    
    // 获取红外传感器原始数据 - 向后兼容方法
    // DEPRECATED: 请使用 getInfraredSensorValues(uint16_t values[8]) 替代
    const uint16_t* getInfraredSensorValues();
    
    // 判断是否检测到线
    bool isLineDetected();
    
    // --- 颜色传感器 ---
    
    // 获取颜色传感器检测的颜色
    ColorCode getColor();
    
    // 获取颜色传感器原始值到提供的引用参数
    bool getColorSensorValues(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c);
    
    // 获取颜色传感器原始值 - 向后兼容方法
    // DEPRECATED: 请使用 getColorSensorValues(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c) 替代
    void getColorSensorValues(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
    
    // === 调试功能 ===
    
    // 打印指定传感器的调试信息
    void printSensorDebugInfo(SensorType type);
    
    // 打印所有传感器的调试信息
    void printAllDebugInfo();
    
    // 打印颜色传感器调试信息 - 向后兼容方法
    // DEPRECATED: 请使用 printSensorDebugInfo(SensorType::COLOR) 替代
    void debugColorSensor();
    
    // === 老接口（不推荐，但保留兼容性） ===
    
    // 获取红外传感器实例的引用 - 不推荐使用，保留兼容性
    // DEPRECATED: 请使用特定的数据访问方法代替，如 getLinePosition(int&) 或 getInfraredSensorValues(uint16_t[])
    InfraredArray& getInfraredArray() { return infraredSensor; }
};

#endif // SENSOR_MANAGER_H 
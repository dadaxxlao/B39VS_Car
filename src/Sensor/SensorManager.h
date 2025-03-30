#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../Utils/Config.h"
#include "../Utils/Logger.h"
#include "Infrared.h"
#include "Ultrasonic.h"
#include "ColorSensor.h"

class SensorManager {
private:
    InfraredArray infraredSensor;
    UltrasonicSensor ultrasonicSensor;
    ColorSensor colorSensor;
    
    // 上次测量的值（用于过滤异常值）
    float lastDistance;
    ColorCode lastColor;
    
public:
    SensorManager();
    
    // ===== 核心API - 简洁清晰 =====
    
    // 初始化与更新
    void initAllSensors();  // 初始化所有传感器
    void update();          // 更新所有传感器数据(主循环调用)
    
    // 传感器数据访问
    // 超声波传感器
    float getDistance();    // 获取超声波距离(cm)
    bool isObstacleDetected(float threshold = GRAB_DISTANCE); // 是否检测到障碍物
    
    // 红外传感器
    int getLinePosition();  // 获取线位置(-100到100)
    bool isLineDetected();  // 是否检测到线
    
    // 颜色传感器
    ColorCode getColor();   // 获取检测到的颜色
    
#ifdef DEBUG_MODE
    // ===== 调试API - 提供更多细节 =====
    
    // 调试信息打印
    void debugAllSensors(); // 打印所有传感器状态
    
    // 传感器原始数据访问
    const uint16_t* getInfraredSensorValues(); // 获取红外原始数据
    void getColorSensorValues(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c); // 获取颜色原始值
    
    // 传感器控制
    void setColorLED(bool on); // 控制颜色传感器LED
    
    // 单一传感器调试
    void debugInfrared();    // 红外传感器调试
    void debugUltrasonic();  // 超声波传感器调试
    void debugColorSensor(); // 颜色传感器调试
    
    // 传感器校准
    void calibrateColor(ColorCode color); // 校准特定颜色
#endif
};

#endif // SENSOR_MANAGER_H 
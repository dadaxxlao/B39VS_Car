#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "../Utils/Config.h"
#include "Infrared.h"
#include "Ultrasonic.h"
#include "ColorSensor.h"

class SensorManager {
private:
    InfraredArray infraredSensor;
    UltrasonicSensor ultrasonicSensor;
    ColorSensor colorSensor;
    
    // 上次测量的距离值（用于过滤异常值）
    float lastDistance;
    // 上次检测到的颜色
    ColorCode lastColor;
    
public:
    SensorManager();
    
    // 初始化所有传感器
    void initAllSensors();
    
    // 获取超声波测量的距离（厘米）
    float getUltrasonicDistance();
    
    // 获取红外线传感器检测的线位置（-100到100，0为中心）
    int getLinePosition();
    
    // 获取红外传感器原始数据
    const uint16_t* getInfraredSensorValues();
    
    // 获取红外传感器实例的引用
    InfraredArray& getInfraredArray() { return infraredSensor; }
    
    // 判断是否检测到线
    bool isLineDetected();
    
    // 获取颜色传感器检测的颜色
    ColorCode getColor();
    
    // 获取颜色传感器原始值
    void getColorSensorValues(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
    
    // 打印颜色传感器调试信息
    void debugColorSensor();
    
    // 更新传感器数据（在主循环中调用）
    void update();
};

#endif // SENSOR_MANAGER_H 
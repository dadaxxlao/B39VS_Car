#ifndef INFRARED_H
#define INFRARED_H

#include <Arduino.h>
#include <Wire.h>
#include "../Utils/Config.h"

class InfraredArray {
private:
    uint8_t i2cAddress;
    uint16_t sensorValues[8]; // 8路红外传感器值
    bool isConnected;
    
    // 读取传感器原始数据
    void update();
    
public:
    InfraredArray();
    
    // 核心API - 保持简洁
    bool begin(uint8_t address);   // 初始化传感器
    void update();                 // 更新传感器数据
    int getLinePosition();         // 获取线位置 (-100到100)
    bool isLineDetected();         // 是否检测到线
    const uint16_t* getAllSensorValues() const; // 获取原始数据数组
    
<<<<<<< HEAD
#ifdef DEBUG_INFRARED
    // 调试API
    void printDebugInfo();         // 打印调试信息
    uint16_t getSensorValue(uint8_t index); // 获取单个传感器值(仅调试用)
#endif
=======
    // 获取巡线位置（-100到100，0表示线在中心）
    int getLinePosition();
    
    // 获取指定传感器的值
    uint16_t getSensorValue(uint8_t index);
    
    // 获取所有传感器的值数组
    const uint16_t* getAllSensorValues();
    
    // 判断是否检测到线
    bool isLineDetected();
>>>>>>> main
};

#endif // INFRARED_H 
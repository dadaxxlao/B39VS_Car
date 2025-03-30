#ifndef INFRARED_H
#define INFRARED_H

#include <Arduino.h>
#include <Wire.h>
#include "../Utils/Config.h"
#include "SensorCommon.h"

// 红外传感器寄存器地址常量
const uint8_t IR_READ_REGISTER = 0x30;  // 读取传感器状态的寄存器地址

class InfraredArray {
private:
    uint8_t i2cAddress;
    uint16_t sensorValues[8]; // 8路红外传感器值
    bool isConnected;         // 连接状态
    bool initialized;         // 初始化状态
    
    // 读取传感器原始数据
    bool readSensorValues();
    
public:
    InfraredArray();
    
    // 初始化红外传感器 - 不再需要参数
    bool begin();
    
    // 获取初始化状态
    bool isInitialized() const;
    
    // 检查传感器健康状态
    SensorStatus checkHealth();
    
    // 更新传感器数据
    void update();
    
    // 获取巡线位置（-100到100，0表示线在中心）
    // 返回INFRARED_NO_LINE表示未检测到线
    int getLinePosition();
    
    // 获取线位置并通过引用参数返回
    // 返回true表示检测到线，false表示未检测到
    bool getLinePosition(int& position);
    
    // 获取指定传感器的值
    uint16_t getSensorValue(uint8_t index);
    
    // 获取所有传感器的值数组
    const uint16_t* getAllSensorValues() const;
    
    // 填充传感器值到提供的数组
    void getAllSensorValues(uint16_t values[8]) const;
    
    // 判断是否检测到线
    bool isLineDetected();
    
    // 调试打印
    void debugPrint();
};

#endif // INFRARED_H 
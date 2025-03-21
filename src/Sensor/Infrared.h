#ifndef INFRARED_H
#define INFRARED_H

#include <Arduino.h>
#include <Wire.h>

class InfraredArray {
private:
    uint8_t i2cAddress;
    uint16_t sensorValues[8]; // 8路红外传感器值
    bool isConnected;
    
    // 读取传感器原始数据
    bool readSensorValues();
    
public:
    InfraredArray();
    
    // 初始化红外传感器
    bool begin(uint8_t address);
    
    // 更新传感器数据
    void update();
    
    // 获取巡线位置（-100到100，0表示线在中心）
    int getLinePosition();
    
    // 获取指定传感器的值
    uint16_t getSensorValue(uint8_t index);
    
    // 获取所有传感器的值数组
    const uint16_t* getAllSensorValues() const;
    
    // 判断是否检测到线
    bool isLineDetected();
};

#endif // INFRARED_H 
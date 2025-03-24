#include "Infrared.h"
#include "../Utils/Logger.h"

InfraredArray::InfraredArray() : i2cAddress(0), isConnected(false) {
    // 初始化传感器数值
    for (int i = 0; i < 8; i++) {
        sensorValues[i] = 0;
    }
}

bool InfraredArray::begin(uint8_t address) {
    i2cAddress = address;
    Wire.begin(); // 确保I2C总线已初始化
    
    // 简单测试I2C连接
    Wire.beginTransmission(i2cAddress);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        isConnected = true;
        Logger::info("红外线传感器连接成功");
        
        // 初次更新传感器数据
        update();
        return true;
    } else {
        isConnected = false;
        Logger::error("红外线传感器连接失败，错误码: %d", error);
        return false;
    }
}

void InfraredArray::update() {
    if (!isConnected) {
        return;
    }
    
    // 根据您提供的示例，发送读取命令并处理数据
    byte data = 0;
    
    Wire.beginTransmission(i2cAddress);
    Wire.write(0x30);  // 寄存器地址，从示例代码获取
    Wire.endTransmission();
    
    delay(10); // 给设备足够时间处理请求
    
    Wire.requestFrom(int(i2cAddress), int(1)); // 请求1个字节的数据
    if (Wire.available()) {
        data = Wire.read();
    }
    
    // 解析数据到各个传感器值
    sensorValues[0] = (data >> 7) & 0x01;
    sensorValues[1] = (data >> 6) & 0x01;
    sensorValues[2] = (data >> 5) & 0x01;
    sensorValues[3] = (data >> 4) & 0x01;
    sensorValues[4] = (data >> 3) & 0x01;
    sensorValues[5] = (data >> 2) & 0x01;
    sensorValues[6] = (data >> 1) & 0x01;
    sensorValues[7] = (data >> 0) & 0x01;
    
    Logger::debug("红外传感器值: %d,%d,%d,%d,%d,%d,%d,%d", 
                 sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3],
                 sensorValues[4], sensorValues[5], sensorValues[6], sensorValues[7]);
}

int InfraredArray::getLinePosition() {
    if (!isConnected) {
        return 0; // 如果未连接，返回中心位置
    }
    
    // 计算线的位置
    // 使用加权平均法计算线位置
    // 传感器排列假设为从左到右 0-7
    
    int sum = 0;
    int weightedSum = 0;
    
    for (int i = 0; i < 8; i++) {
        // 对于数字量传感器，检测到黑线时值为0（而不是1）
        // 未检测到黑线时值为1
        if (sensorValues[i] == 0) {
            sum += 1;
            // 将传感器位置映射到-100到100的范围
            // 0号传感器在最左边(-100)，7号传感器在最右边(+100)
            int position = map(i, 0, 7, -100, 100);
            weightedSum += position;
        }
    }
    
    // 如果没有检测到线，返回上一次的位置
    if (sum == 0) {
        return 0; // 默认中心位置
    }
    
    // 计算加权平均值
    return weightedSum / sum;
}

uint16_t InfraredArray::getSensorValue(uint8_t index) {
    if (index < 8) {
        return sensorValues[index];
    }
    return 0;
}

const uint16_t* InfraredArray::getAllSensorValues() const {
    return sensorValues;
}

bool InfraredArray::isLineDetected() {
    // 检查是否有任何传感器检测到线
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] == 0) {
            return true;
        }
    }
    return false;
} 
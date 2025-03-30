#include "Infrared.h"
#include "../Utils/Logger.h"

InfraredArray::InfraredArray() : i2cAddress(0), isConnected(false), initialized(false) {
    // 初始化传感器数值
    for (int i = 0; i < 8; i++) {
        sensorValues[i] = 0;
    }
}

bool InfraredArray::begin() {
    // 从Config.h读取地址
    i2cAddress = INFRARED_ARRAY_ADDR;
    
    // 检查地址是否有效
    if (i2cAddress == 0) {
        Logger::error("Infrared", "无效的红外传感器地址配置!");
        isConnected = false;
        initialized = false;
        return false;
    }
    
    Wire.begin(); // 确保I2C总线已初始化
    
    // 简单测试I2C连接
    Wire.beginTransmission(i2cAddress);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
        isConnected = true;
        initialized = true;
        
        // 记录详细的初始化信息
        Logger::info("Infrared", "红外线传感器连接成功 (地址: 0x%02X)", i2cAddress);
        
        // 初次更新传感器数据
        update();
        return true;
    } else {
        isConnected = false;
        initialized = false;
        
        // 提供更详细的错误信息
        const char* errorMsg = "未知错误";
        switch (error) {
            case 1: errorMsg = "数据过长"; break;
            case 2: errorMsg = "地址未应答(NACK)"; break;
            case 3: errorMsg = "数据未应答(NACK)"; break;
            case 4: errorMsg = "其他I2C错误"; break;
        }
        
        Logger::error("Infrared", "红外线传感器连接失败 (地址: 0x%02X) - 错误 %d: %s", 
                     i2cAddress, error, errorMsg);
        return false;
    }
}

bool InfraredArray::isInitialized() const {
    return initialized;
}

SensorStatus InfraredArray::checkHealth() {
    if (!initialized) return SensorStatus::NOT_INITIALIZED;
    if (!isConnected) return SensorStatus::ERROR_COMM;
    
    // 简单通信测试
    Wire.beginTransmission(i2cAddress);
    byte error = Wire.endTransmission();
    
    if (error != 0) {
        isConnected = false;
        return SensorStatus::ERROR_COMM;
    }
    
    return SensorStatus::OK;
}

void InfraredArray::update() {
    if (!isConnected) {
        return;
    }
    
    // 根据示例代码，发送读取命令并处理数据
    byte data = 0;
    
    Wire.beginTransmission(i2cAddress);
    Wire.write(IR_READ_REGISTER);  // 使用常量替代硬编码的0x30寄存器地址
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
    
    Logger::debug("Infrared", "红外传感器值: %d,%d,%d,%d,%d,%d,%d,%d", 
                 sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3],
                 sensorValues[4], sensorValues[5], sensorValues[6], sensorValues[7]);
}

int InfraredArray::getLinePosition() {
    if (!isConnected) {
        return INFRARED_NO_LINE; // 如果未连接，返回未检测到线的特殊值
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
    
    // 如果没有检测到线，返回特殊值
    if (sum == 0) {
        return INFRARED_NO_LINE;
    }
    
    // 计算加权平均值
    return weightedSum / sum;
}

bool InfraredArray::getLinePosition(int& position) {
    int pos = getLinePosition();
    
    if (pos == INFRARED_NO_LINE) {
        return false; // 未检测到线
    }
    
    position = pos;
    return true; // 检测到线
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

void InfraredArray::getAllSensorValues(uint16_t values[8]) const {
    for (int i = 0; i < 8; i++) {
        values[i] = sensorValues[i];
    }
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

void InfraredArray::debugPrint() {
    if (!initialized) {
        Logger::debug("Infrared", "传感器未初始化");
        return;
    }
    
    if (!isConnected) {
        Logger::debug("Infrared", "传感器已初始化但未连接 (地址: 0x%02X)", i2cAddress);
        return;
    }
    
    Logger::debug("Infrared", "状态: 已连接 (地址: 0x%02X)", i2cAddress);
    Logger::debug("Infrared", "传感器值: %d,%d,%d,%d,%d,%d,%d,%d", 
                 sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3],
                 sensorValues[4], sensorValues[5], sensorValues[6], sensorValues[7]);
    
    int linePos = getLinePosition();
    if (linePos == INFRARED_NO_LINE) {
        Logger::debug("Infrared", "线位置: 未检测到线");
    } else {
        Logger::debug("Infrared", "线位置: %d (-100左, 0中, +100右)", linePos);
    }
} 
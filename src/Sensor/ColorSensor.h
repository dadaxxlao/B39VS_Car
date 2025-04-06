#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "../Utils/Config.h"
#include "SensorCommon.h"

// Ganwei Color Sensor I2C Commands
const uint8_t CMD_READ_RGB = 0xD0;
const uint8_t CMD_READ_HSL = 0xD1;
const uint8_t CMD_PING = 0xAA;
const uint8_t CMD_READ_ERROR = 0xDE;
const uint8_t CMD_RESET = 0xC0;
const uint8_t CMD_READ_VERSION = 0xC1;

// Expected PING response
const uint8_t PING_RESPONSE = 0x66;

// 感为 'Ganwei' GW) I2C 颜色传感器接口
class ColorSensor {
private:
    uint8_t i2cAddress;
    bool initialized;
    bool isConnected;
    uint8_t lastCommandSent;
    uint8_t lastR, lastG, lastB;
    uint8_t lastH, lastS, lastL;
    unsigned long lastReadMillis;
    
    // 用于颜色识别的阈值
    struct ColorThreshold {
        // HSL阈值
        uint8_t minH, maxH;  // 色相范围
        uint8_t minS, maxS;  // 饱和度范围
        uint8_t minL, maxL;  // 亮度范围
    };
    
    // 为每种颜色定义阈值
    ColorThreshold colorThresholds[COLOR_COUNT];
    
    // 初始化颜色阈值
    void initColorThresholds();
    
    // 根据HSL值识别颜色
    ColorCode identifyColorHSL(uint8_t h, uint8_t s, uint8_t l);
    
    // 读取传感器数据的辅助函数
    bool readSensorData(uint8_t command, uint8_t* dataBuffer, uint8_t numBytes);
    
    // 发送命令的辅助函数
    bool sendCommand(uint8_t command);

public:
    ColorSensor();
    
    // 初始化传感器
    bool begin();
    
    // 获取初始化状态
    bool isInitialized() const;
    
    // 检查传感器健康状态
    SensorStatus checkHealth();
    
    // 获取识别的颜色
    ColorCode getColor();
    
    // 获取RGB值
    bool getColorRGB(uint8_t& r, uint8_t& g, uint8_t& b);
    
    // 获取HSL值
    bool getColorHSL(uint8_t& h, uint8_t& s, uint8_t& l);
    
    // 获取错误状态
    bool getErrorStatus(uint8_t& errorByte);
    
    // Ping传感器
    bool pingSensor();
    
    // 重置传感器
    bool resetSensor();
    
    // 获取固件版本
    bool getFirmwareVersion(uint8_t& versionByte);
    
    // 调试打印
    void debugPrint();
};

#endif // COLOR_SENSOR_H 
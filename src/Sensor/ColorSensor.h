#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "../Utils/Config.h"

// TCS34725颜色传感器接口
class ColorSensor {
private:
    Adafruit_TCS34725 tcs;
    bool initialized;
    uint16_t r, g, b, c;
    float colorTemp;
    float lux;
    
    // 用于颜色识别的阈值
    struct ColorThreshold {
        uint16_t minR, maxR;
        uint16_t minG, maxG;
        uint16_t minB, maxB;
        uint16_t minC, maxC; // 清晰度/强度
    };
    
    // 为每种颜色定义阈值
    ColorThreshold colorThresholds[COLOR_COUNT];
    
    // 初始化颜色阈值
    void initColorThresholds();
    
    // 根据RGB值识别颜色
    ColorCode identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c);

public:
    ColorSensor();
    
    // 初始化传感器
    bool begin(uint8_t addr = 0x29);
    
    // 更新传感器数据
    void update();
    
    // 读取当前颜色
    ColorCode readColor();
    
    // 获取RGB原始值
    void getRGB(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
    
    // 调试打印
    void debugPrint();
    
    // 控制传感器LED
    void lock();   // 关闭LED
    void unlock(); // 打开LED
    
    // 颜色校准函数
    void calibrateColor(ColorCode color);
    
    // 打印原始RGB值和比例值
    void printRawValues();
    
    // 计算RGB比例
    void calculateNormalizedRGB(uint16_t r, uint16_t g, uint16_t b, uint16_t c, 
                                float* normR, float* normG, float* normB);
};

#endif // COLOR_SENSOR_H 
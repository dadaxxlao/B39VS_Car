#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// TCS34725颜色传感器接口
class ColorSensor {
private:
    Adafruit_TCS34725 tcs;
    bool initialized;
    uint16_t r, g, b, c;  // 原始RGB值
    float colorTemp;
    float lux;
    
    // 颜色阈值结构体
    struct ColorThreshold {
        // RGB阈值
        uint16_t minR, maxR;
        uint16_t minG, maxG;
        uint16_t minB, maxB;
        uint16_t minC, maxC;
        
        // HSV阈值
        float minH, maxH;
        float minS, maxS;
        float minV, maxV;
    };
    
    // 颜色阈值数组
    ColorThreshold colorThresholds[COLOR_COUNT];
    
    // 初始化颜色阈值
    void initColorThresholds();
    
    // RGB转HSV
    void rgbToHSV(uint16_t r, uint16_t g, uint16_t b, float* h, float* s, float* v);
    
    // 计算归一化RGB
    void calculateNormalizedRGB(uint16_t r, uint16_t g, uint16_t b, uint16_t c,
                                float* normR, float* normG, float* normB);
                                
    // 统一的颜色识别算法
    ColorCode identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
    
public:
    ColorSensor();
    
    // 核心API
    bool begin(uint8_t addr = 0x29);
    void update();
    ColorCode readColor();
    void getRGB(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
    
    // 控制LED
    void setLED(bool on);
    
#ifdef DEBUG_COLOR_SENSOR
    // 调试API
    void printDebugInfo();
    void printRGBValues();
    void printHSVValues();
    void calibrateColor(ColorCode color); // 颜色校准
#endif
};

#endif // COLOR_SENSOR_H 
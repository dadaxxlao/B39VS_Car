#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include "../Utils/Config.h"
#include "SensorCommon.h"

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
        // RGB阈值（兼容旧算法）
        uint16_t minR, maxR;
        uint16_t minG, maxG;
        uint16_t minB, maxB;
        uint16_t minC, maxC; // 清晰度/强度
        
        // HSV阈值（新算法）
        float minH, maxH;  // 色相范围
        float minS, maxS;  // 饱和度范围
        float minV, maxV;  // 明度范围
    };
    
    // 为每种颜色定义阈值
    ColorThreshold colorThresholds[COLOR_COUNT];
    
    // 初始化颜色阈值
    void initColorThresholds();
    
    // RGB转HSV颜色空间
    void rgbToHSV(uint16_t r, uint16_t g, uint16_t b, float* h, float* s, float* v);
    
    // 根据RGB值识别颜色
    ColorCode identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c);

public:
    ColorSensor();
    
    // 初始化传感器
    bool begin(uint8_t addr = 0x29);
    
    // 获取初始化状态
    bool isInitialized() const;
    
    // 检查传感器健康状态
    SensorStatus checkHealth();
    
    // 更新传感器数据
    void update();
    
    // 读取当前颜色
    ColorCode readColor();
    
    // 获取RGB原始值
    void getRGB(uint16_t* r, uint16_t* g, uint16_t* b, uint16_t* c);
    
    // 获取RGB原始值(通过引用)
    bool getRGB(uint16_t& r, uint16_t& g, uint16_t& b, uint16_t& c);
    
    // 获取HSV颜色值
    bool getHSV(float& h, float& s, float& v);
    
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
                                
    // 颜色识别算法（测试用）
    ColorCode identifyColorHSV(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
    ColorCode identifyColorRGB(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
    
    // EEPROM相关方法 (待实现)
    bool saveCalibration();
    bool loadCalibration();
};

#endif // COLOR_SENSOR_H 
#ifndef RGB_CONTROLLER_H
#define RGB_CONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"

class RGBController {
private:
    Adafruit_NeoPixel* strip;
    uint8_t pin;
    uint8_t numPixels;
    bool initialized;
    
    // Gamma校正表
    uint8_t gamma8[256];
    
    // 当前动画状态
    bool animationActive;
    uint8_t animationPattern;
    uint32_t lastAnimationUpdate;
    uint8_t animationStep;
    
    // 储存当前颜色
    uint8_t currentRed;
    uint8_t currentGreen;
    uint8_t currentBlue;
    
    // HSV转RGB辅助函数
    void hsv2rgb(uint16_t hue, uint8_t sat, uint8_t val, uint8_t* r, uint8_t* g, uint8_t* b);

public:
    RGBController();
    
    // 初始化 LED 灯环
    bool init(uint8_t pin = RGB_LED_PIN, uint8_t numPixels = RGB_LED_COUNT);
    
    // 设置所有LED为指定颜色
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    
    // 设置指定LED的颜色
    void setPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b);
    
    // 根据颜色编码设置颜色
    void setColorCode(ColorCode color);
    
    // 关闭所有LED
    void off();
    
    // 显示彩虹效果
    void rainbow(uint8_t wait = 10);
    
    // 显示特定模式
    void showPattern(uint8_t pattern, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t wait = 50);
    
    // 更新动画状态
    void update();
};

#endif // RGB_CONTROLLER_H 
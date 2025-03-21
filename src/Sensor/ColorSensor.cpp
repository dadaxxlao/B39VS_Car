#include "ColorSensor.h"
#include "../Utils/Logger.h"

ColorSensor::ColorSensor() : initialized(false) {
    // 在构造函数中初始化颜色阈值
    initColorThresholds();
}

bool ColorSensor::begin(uint8_t addr) {
    // 尝试初始化TCS34725传感器
    initialized = tcs.begin(addr);
    
    if (!initialized) {
        Logger::error("无法连接到TCS34725颜色传感器");
        return false;
    }
    
    // 设置适当的增益和积分时间
    tcs.setGain(TCS34725_GAIN_4X);
    tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_154MS);
    
    Logger::info("TCS34725颜色传感器初始化成功");
    return true;
}

void ColorSensor::initColorThresholds() {
    // 初始化颜色阈值
    // 注意：这些值需要根据实际测试进行校准
    
    // 红色阈值
    colorThresholds[COLOR_RED].minR = 150;
    colorThresholds[COLOR_RED].maxR = 255;
    colorThresholds[COLOR_RED].minG = 0;
    colorThresholds[COLOR_RED].maxG = 100;
    colorThresholds[COLOR_RED].minB = 0;
    colorThresholds[COLOR_RED].maxB = 100;
    colorThresholds[COLOR_RED].minC = 50;
    colorThresholds[COLOR_RED].maxC = 255;
    
    // 黄色阈值
    colorThresholds[COLOR_YELLOW].minR = 0;
    colorThresholds[COLOR_YELLOW].maxR = 100;
    colorThresholds[COLOR_YELLOW].minG = 150;
    colorThresholds[COLOR_YELLOW].maxG = 255;
    colorThresholds[COLOR_YELLOW].minB = 0;
    colorThresholds[COLOR_YELLOW].maxB = 100;
    colorThresholds[COLOR_YELLOW].minC = 50;
    colorThresholds[COLOR_YELLOW].maxC = 255;
    
    // 蓝色阈值
    colorThresholds[COLOR_BLUE].minR = 0;
    colorThresholds[COLOR_BLUE].maxR = 100;
    colorThresholds[COLOR_BLUE].minG = 0;
    colorThresholds[COLOR_BLUE].maxG = 100;
    colorThresholds[COLOR_BLUE].minB = 150;
    colorThresholds[COLOR_BLUE].maxB = 255;
    colorThresholds[COLOR_BLUE].minC = 50;
    colorThresholds[COLOR_BLUE].maxC = 255;
    
    // 黑色阈值
    colorThresholds[COLOR_BLACK].minR = 0;
    colorThresholds[COLOR_BLACK].maxR = 50;
    colorThresholds[COLOR_BLACK].minG = 0;
    colorThresholds[COLOR_BLACK].maxG = 50;
    colorThresholds[COLOR_BLACK].minB = 0;
    colorThresholds[COLOR_BLACK].maxB = 50;
    colorThresholds[COLOR_BLACK].minC = 0;
    colorThresholds[COLOR_BLACK].maxC = 50;
    
    // 白色阈值
    colorThresholds[COLOR_WHITE].minR = 150;
    colorThresholds[COLOR_WHITE].maxR = 255;
    colorThresholds[COLOR_WHITE].minG = 150;
    colorThresholds[COLOR_WHITE].maxG = 255;
    colorThresholds[COLOR_WHITE].minB = 150;
    colorThresholds[COLOR_WHITE].maxB = 255;
    colorThresholds[COLOR_WHITE].minC = 150;
    colorThresholds[COLOR_WHITE].maxC = 255;
    
    // 未知颜色不需要阈值
}

void ColorSensor::update() {
    if (!initialized) {
        return;
    }
    
    // 读取RGB和清晰度值
    tcs.getRawData(&r, &g, &b, &c);
    
    // 计算色温和亮度
    colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
    lux = tcs.calculateLux(r, g, b);
}

ColorCode ColorSensor::readColor() {
    if (!initialized) {
        return COLOR_UNKNOWN;
    }
    
    // 先更新传感器数据
    update();
    
    // 根据RGB值识别颜色
    return identifyColor(r, g, b, c);
}

ColorCode ColorSensor::identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // 标准化RGB值到0-255范围
    uint8_t normR = map(r, 0, 1024, 0, 255);
    uint8_t normG = map(g, 0, 1024, 0, 255);
    uint8_t normB = map(b, 0, 1024, 0, 255);
    uint8_t normC = map(c, 0, 1024, 0, 255);
    
    // 检查每种颜色的阈值匹配
    for (int i = 0; i < COLOR_COUNT - 1; i++) { // 不包括COLOR_UNKNOWN
        if (normR >= colorThresholds[i].minR && normR <= colorThresholds[i].maxR &&
            normG >= colorThresholds[i].minG && normG <= colorThresholds[i].maxG &&
            normB >= colorThresholds[i].minB && normB <= colorThresholds[i].maxB &&
            normC >= colorThresholds[i].minC && normC <= colorThresholds[i].maxC) {
            return static_cast<ColorCode>(i);
        }
    }
    
    // 如果没有匹配的颜色，返回未知
    return COLOR_UNKNOWN;
}

void ColorSensor::getRGB(uint16_t* red, uint16_t* green, uint16_t* blue, uint16_t* clear) {
    if (!initialized) {
        *red = 0;
        *green = 0;
        *blue = 0;
        *clear = 0;
        return;
    }
    
    *red = r;
    *green = g;
    *blue = b;
    *clear = c;
}

void ColorSensor::debugPrint() {
    if (!initialized) {
        Logger::debug("颜色传感器未初始化");
        return;
    }
    
    // 确保有最新数据
    update();
    
    // 打印RGB和清晰度值
    Logger::debug("颜色传感器数据: R=%d, G=%d, B=%d, C=%d", r, g, b, c);
    Logger::debug("色温: %.2f K, 亮度: %.2f lux", colorTemp, lux);
    
    // 打印检测到的颜色
    ColorCode color = identifyColor(r, g, b, c);
    const char* colorName;
    
    switch (color) {
        case COLOR_RED:    colorName = "红色"; break;
        case COLOR_YELLOW: colorName = "黄色"; break;
        case COLOR_BLUE:   colorName = "蓝色"; break;
        case COLOR_BLACK:  colorName = "黑色"; break;
        case COLOR_WHITE:  colorName = "白色"; break;
        default:           colorName = "未知"; break;
    }
    
    Logger::debug("检测到颜色: %s", colorName);
} 
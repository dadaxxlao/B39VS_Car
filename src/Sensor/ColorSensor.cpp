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
    // 增加增益，适合低光条件，积分时间增加提高精度
    tcs.setGain(TCS34725_GAIN_16X);
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

void ColorSensor::calculateNormalizedRGB(uint16_t r, uint16_t g, uint16_t b, uint16_t c,
                                         float* normR, float* normG, float* normB) {
    // 检查总亮度是否足够
    uint32_t sum = c;
    if (sum < 10) {
        *normR = 0;
        *normG = 0;
        *normB = 0;
        return;
    }
    
    // 计算RGB比例
    *normR = r; *normR /= sum; *normR *= 255;
    *normG = g; *normG /= sum; *normG *= 255;
    *normB = b; *normB /= sum; *normB *= 255;
}

ColorCode ColorSensor::identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // 计算RGB比例
    float normR, normG, normB;
    calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
    
    // 检查光线是否足够
    if (c < 10) return COLOR_UNKNOWN; // 光线太暗，无法可靠判断
    
    // 基于RGB比例判断颜色
    // 红色: R比例高，G和B比例低
    if (normR > 120 && normG < 80 && normB < 80) return COLOR_RED;
    
    // 蓝色: B比例高，R和G比例低
    if (normB > 120 && normR < 80 && normG < 80) return COLOR_BLUE;
    
    // 黄色: R和G比例高，B比例低
    if (normR > 100 && normG > 100 && normB < 80) return COLOR_YELLOW;
    
    // 白色: 所有比例都高
    if (normR > 90 && normG > 90 && normB > 90) return COLOR_WHITE;
    
    // 黑色: 所有比例都低，或者整体亮度低
    if ((normR < 50 && normG < 50 && normB < 50) || c < 30) return COLOR_BLACK;
    
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
    
    // 计算并打印RGB比例
    float normR, normG, normB;
    calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
    Logger::debug("RGB比例: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
    
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

void ColorSensor::lock() {
    if (initialized) {
        // 关闭TCS34725的LED
        // 注意：这取决于Adafruit_TCS34725库是否支持
        // 需要检查库是否有以下函数或类似功能
        tcs.disable();
    }
}

void ColorSensor::unlock() {
    if (initialized) {
        // 打开TCS34725的LED
        tcs.enable();
    }
}

void ColorSensor::calibrateColor(ColorCode color) {
    if (!initialized) {
        Logger::error("颜色传感器未初始化，无法校准");
        return;
    }
    
    uint32_t sumR = 0, sumG = 0, sumB = 0, sumC = 0;
    const int samples = 10;
    
    Logger::info("开始校准颜色，请保持传感器位置不变...");
    
    // 采集多个样本并平均
    for (int i = 0; i < samples; i++) {
        update();
        sumR += r; sumG += g; sumB += b; sumC += c;
        Logger::debug("采样 %d: R=%d, G=%d, B=%d, C=%d", i+1, r, g, b, c);
        delay(100); // 采样间隔
    }
    
    // 计算平均值
    uint16_t avgR = sumR / samples;
    uint16_t avgG = sumG / samples;
    uint16_t avgB = sumB / samples;
    uint16_t avgC = sumC / samples;
    
    // 计算RGB比例
    float normR, normG, normB;
    calculateNormalizedRGB(avgR, avgG, avgB, avgC, &normR, &normG, &normB);
    
    // 打印校准值
    const char* colorName;
    switch (color) {
        case COLOR_RED:    colorName = "红色"; break;
        case COLOR_BLUE:   colorName = "蓝色"; break;
        case COLOR_YELLOW: colorName = "黄色"; break;
        case COLOR_WHITE:  colorName = "白色"; break;
        case COLOR_BLACK:  colorName = "黑色"; break;
        default:           colorName = "未知"; break;
    }
    
    Logger::info("颜色校准 - %s:", colorName);
    Logger::info("原始值: R=%d, G=%d, B=%d, C=%d", avgR, avgG, avgB, avgC);
    Logger::info("比例值: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
    
    // 提供建议的阈值设置
    Logger::info("建议的阈值设置:");
    Logger::info("colorThresholds[%d].minR = %.0f;", color, normR * 0.8);
    Logger::info("colorThresholds[%d].maxR = %.0f;", color, normR * 1.2);
    Logger::info("colorThresholds[%d].minG = %.0f;", color, normG * 0.8);
    Logger::info("colorThresholds[%d].maxG = %.0f;", color, normG * 1.2);
    Logger::info("colorThresholds[%d].minB = %.0f;", color, normB * 0.8);
    Logger::info("colorThresholds[%d].maxB = %.0f;", color, normB * 1.2);
}

void ColorSensor::printRawValues() {
    if (!initialized) {
        Logger::error("颜色传感器未初始化");
        return;
    }
    
    update();
    
    // 打印原始RGB和清晰度值
    Logger::debug("原始值: R=%d, G=%d, B=%d, C=%d", r, g, b, c);
    
    // 计算RGB比例
    float normR, normG, normB;
    calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
    
    // 打印比例值
    Logger::debug("比例值: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
} 
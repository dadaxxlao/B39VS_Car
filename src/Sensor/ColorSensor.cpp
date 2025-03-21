#include "ColorSensor.h"
#include "../Utils/Logger.h"
#include <math.h>

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
    // HSV阈值
    colorThresholds[COLOR_RED].minH = 350.0f;
    colorThresholds[COLOR_RED].maxH = 10.0f;  // 跨0度处理
    colorThresholds[COLOR_RED].minS = 0.7f;
    colorThresholds[COLOR_RED].maxS = 1.0f;
    colorThresholds[COLOR_RED].minV = 0.3f;
    colorThresholds[COLOR_RED].maxV = 1.0f;
    
    // 黄色阈值
    colorThresholds[COLOR_YELLOW].minR = 0;
    colorThresholds[COLOR_YELLOW].maxR = 100;
    colorThresholds[COLOR_YELLOW].minG = 150;
    colorThresholds[COLOR_YELLOW].maxG = 255;
    colorThresholds[COLOR_YELLOW].minB = 0;
    colorThresholds[COLOR_YELLOW].maxB = 100;
    colorThresholds[COLOR_YELLOW].minC = 50;
    colorThresholds[COLOR_YELLOW].maxC = 255;
    // HSV阈值
    colorThresholds[COLOR_YELLOW].minH = 50.0f;
    colorThresholds[COLOR_YELLOW].maxH = 70.0f;
    colorThresholds[COLOR_YELLOW].minS = 0.6f;
    colorThresholds[COLOR_YELLOW].maxS = 1.0f;
    colorThresholds[COLOR_YELLOW].minV = 0.4f;
    colorThresholds[COLOR_YELLOW].maxV = 1.0f;
    
    // 蓝色阈值
    colorThresholds[COLOR_BLUE].minR = 0;
    colorThresholds[COLOR_BLUE].maxR = 100;
    colorThresholds[COLOR_BLUE].minG = 0;
    colorThresholds[COLOR_BLUE].maxG = 100;
    colorThresholds[COLOR_BLUE].minB = 150;
    colorThresholds[COLOR_BLUE].maxB = 255;
    colorThresholds[COLOR_BLUE].minC = 50;
    colorThresholds[COLOR_BLUE].maxC = 255;
    // HSV阈值
    colorThresholds[COLOR_BLUE].minH = 210.0f;
    colorThresholds[COLOR_BLUE].maxH = 250.0f;
    colorThresholds[COLOR_BLUE].minS = 0.5f;
    colorThresholds[COLOR_BLUE].maxS = 1.0f;
    colorThresholds[COLOR_BLUE].minV = 0.3f;
    colorThresholds[COLOR_BLUE].maxV = 0.9f;
    
    // 黑色阈值
    colorThresholds[COLOR_BLACK].minR = 0;
    colorThresholds[COLOR_BLACK].maxR = 50;
    colorThresholds[COLOR_BLACK].minG = 0;
    colorThresholds[COLOR_BLACK].maxG = 50;
    colorThresholds[COLOR_BLACK].minB = 0;
    colorThresholds[COLOR_BLACK].maxB = 50;
    colorThresholds[COLOR_BLACK].minC = 0;
    colorThresholds[COLOR_BLACK].maxC = 50;
    // HSV阈值
    colorThresholds[COLOR_BLACK].minH = 0.0f;
    colorThresholds[COLOR_BLACK].maxH = 360.0f;
    colorThresholds[COLOR_BLACK].minS = 0.0f;
    colorThresholds[COLOR_BLACK].maxS = 1.0f;
    colorThresholds[COLOR_BLACK].minV = 0.0f;
    colorThresholds[COLOR_BLACK].maxV = 0.15f;
    
    // 白色阈值
    colorThresholds[COLOR_WHITE].minR = 150;
    colorThresholds[COLOR_WHITE].maxR = 255;
    colorThresholds[COLOR_WHITE].minG = 150;
    colorThresholds[COLOR_WHITE].maxG = 255;
    colorThresholds[COLOR_WHITE].minB = 150;
    colorThresholds[COLOR_WHITE].maxB = 255;
    colorThresholds[COLOR_WHITE].minC = 150;
    colorThresholds[COLOR_WHITE].maxC = 255;
    // HSV阈值
    colorThresholds[COLOR_WHITE].minH = 0.0f;
    colorThresholds[COLOR_WHITE].maxH = 360.0f;
    colorThresholds[COLOR_WHITE].minS = 0.0f;
    colorThresholds[COLOR_WHITE].maxS = 0.1f;
    colorThresholds[COLOR_WHITE].minV = 0.9f;
    colorThresholds[COLOR_WHITE].maxV = 1.0f;
    
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

// RGB转HSV颜色空间
void ColorSensor::rgbToHSV(uint16_t r, uint16_t g, uint16_t b, float* h, float* s, float* v) {
    // 归一化RGB值 (0-1)
    float normR, normG, normB;
    calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
    
    float rf = normR / 255.0f;
    float gf = normG / 255.0f;
    float bf = normB / 255.0f;
    
    float max = fmaxf(rf, fmaxf(gf, bf));
    float min = fminf(rf, fminf(gf, bf));
    float delta = max - min;
    
    // 计算HSV值
    *v = max;  // 明度值
    
    if (delta < 0.001f) {
        *h = 0;  // 无色相
        *s = 0;  // 无饱和度
        return;
    }
    
    *s = delta / max;  // 饱和度
    
    // 计算色相 (0-360°)
    if (rf >= max) {
        // 红色区域 (在黄色和品红色之间)
        *h = fmodf(((gf - bf) / delta) * 60.0f + 360.0f, 360.0f);
    } else if (gf >= max) {
        // 绿色区域 (在青色和黄色之间)
        *h = ((bf - rf) / delta + 2.0f) * 60.0f;
    } else {
        // 蓝色区域 (在品红色和青色之间)
        *h = ((rf - gf) / delta + 4.0f) * 60.0f;
    }
}

// 使用RGB算法识别颜色（兼容旧版）
ColorCode ColorSensor::identifyColorRGB(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
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

// 使用HSV算法识别颜色
ColorCode ColorSensor::identifyColorHSV(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    
    // 优先处理极端情况
    if (c < 30) return COLOR_BLACK;  // 低强度直接判黑
    if (v < 0.15f) return COLOR_BLACK;  // 低明度直接判黑
    if (s < 0.1f && v > 0.9f) return COLOR_WHITE;  // 低饱和高亮判白
    
    // 判断红色（可能跨越360度）
    if (colorThresholds[COLOR_RED].minH > colorThresholds[COLOR_RED].maxH) {
        // 处理跨0度的情况（如350-10度）
        if ((h >= colorThresholds[COLOR_RED].minH || h <= colorThresholds[COLOR_RED].maxH) &&
            s >= colorThresholds[COLOR_RED].minS && s <= colorThresholds[COLOR_RED].maxS &&
            v >= colorThresholds[COLOR_RED].minV && v <= colorThresholds[COLOR_RED].maxV) {
            return COLOR_RED;
        }
    }
    
    // 遍历其他颜色
    for (int i = 1; i < COLOR_COUNT; i++) {
        // 跳过红色（已处理）和未知颜色
        if (i == COLOR_RED || i == COLOR_UNKNOWN) continue;
        
        if (h >= colorThresholds[i].minH && h <= colorThresholds[i].maxH &&
            s >= colorThresholds[i].minS && s <= colorThresholds[i].maxS &&
            v >= colorThresholds[i].minV && v <= colorThresholds[i].maxV) {
            return static_cast<ColorCode>(i);
        }
    }
    
    return COLOR_UNKNOWN;
}

// 根据RGB值识别颜色（集成HSV和RGB两种方法）
ColorCode ColorSensor::identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // 使用HSV方法进行识别（主要方法）
    ColorCode colorHSV = identifyColorHSV(r, g, b, c);
    
    // 如果HSV方法能够确定颜色，直接返回
    if (colorHSV != COLOR_UNKNOWN) {
        return colorHSV;
    }
    
    // 如果HSV方法无法确定，尝试使用RGB方法作为备选
    return identifyColorRGB(r, g, b, c);
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
    
    // 计算并打印HSV值
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    Logger::debug("HSV值: H=%.1f°, S=%.2f, V=%.2f", h, s, v);
    
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
    float sumH = 0, sumS = 0, sumV = 0;
    const int samples = 10;
    
    Logger::info("开始校准颜色，请保持传感器位置不变...");
    
    // 采集多个样本并平均
    for (int i = 0; i < samples; i++) {
        update();
        sumR += r; sumG += g; sumB += b; sumC += c;
        
        // 计算HSV值并累加
        float h, s, v;
        rgbToHSV(r, g, b, &h, &s, &v);
        sumH += h; sumS += s; sumV += v;
        
        Logger::debug("采样 %d: R=%d, G=%d, B=%d, C=%d, H=%.1f, S=%.2f, V=%.2f", 
                     i+1, r, g, b, c, h, s, v);
        delay(100); // 采样间隔
    }
    
    // 计算平均值
    uint16_t avgR = sumR / samples;
    uint16_t avgG = sumG / samples;
    uint16_t avgB = sumB / samples;
    uint16_t avgC = sumC / samples;
    
    float avgH = sumH / samples;
    float avgS = sumS / samples;
    float avgV = sumV / samples;
    
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
    Logger::info("RGB比例值: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
    Logger::info("HSV值: H=%.1f°, S=%.2f, V=%.2f", avgH, avgS, avgV);
    
    // 提供建议的阈值设置（RGB）
    Logger::info("建议的RGB阈值设置:");
    Logger::info("colorThresholds[%d].minR = %.0f;", color, normR * 0.8);
    Logger::info("colorThresholds[%d].maxR = %.0f;", color, normR * 1.2);
    Logger::info("colorThresholds[%d].minG = %.0f;", color, normG * 0.8);
    Logger::info("colorThresholds[%d].maxG = %.0f;", color, normG * 1.2);
    Logger::info("colorThresholds[%d].minB = %.0f;", color, normB * 0.8);
    Logger::info("colorThresholds[%d].maxB = %.0f;", color, normB * 1.2);
    
    // 提供建议的HSV阈值设置
    Logger::info("建议的HSV阈值设置:");
    
    // 红色特殊处理（可能跨越0度）
    if (color == COLOR_RED) {
        // 如果靠近0度，使用跨越设置
        if (avgH < 20 || avgH > 340) {
            if (avgH < 20) {
                Logger::info("colorThresholds[%d].minH = %.1f;", color, 360 - (20 - avgH));
                Logger::info("colorThresholds[%d].maxH = %.1f;", color, avgH + 20);
            } else {
                Logger::info("colorThresholds[%d].minH = %.1f;", color, avgH - 20);
                Logger::info("colorThresholds[%d].maxH = %.1f;", color, (avgH + 20) - 360);
            }
        } else {
            // 普通范围
            Logger::info("colorThresholds[%d].minH = %.1f;", color, fmax(0, avgH - 20));
            Logger::info("colorThresholds[%d].maxH = %.1f;", color, fmin(360, avgH + 20));
        }
    } else {
        // 其他颜色正常处理
        Logger::info("colorThresholds[%d].minH = %.1f;", color, fmax(0, avgH - 20));
        Logger::info("colorThresholds[%d].maxH = %.1f;", color, fmin(360, avgH + 20));
    }
    
    Logger::info("colorThresholds[%d].minS = %.2f;", color, fmax(0, avgS * 0.8));
    Logger::info("colorThresholds[%d].maxS = %.2f;", color, fmin(1.0, avgS * 1.2));
    Logger::info("colorThresholds[%d].minV = %.2f;", color, fmax(0, avgV * 0.8));
    Logger::info("colorThresholds[%d].maxV = %.2f;", color, fmin(1.0, avgV * 1.2));
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
    Logger::debug("RGB比例值: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
    
    // 计算并打印HSV值
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    Logger::debug("HSV值: H=%.1f°, S=%.2f, V=%.2f", h, s, v);
} 
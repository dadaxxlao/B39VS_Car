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

// 统一的颜色识别算法，合并原来的RGB和HSV方法
ColorCode ColorSensor::identifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
    // 决定使用HSV算法作为主要实现
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    
    // 首先检查亮度是否太低 (黑色)
    if (v < colorThresholds[COLOR_BLACK].maxV) {
        return COLOR_BLACK;
    }
    
    // 然后检查饱和度是否太低 (白色)
    if (s < colorThresholds[COLOR_WHITE].maxS) {
        return COLOR_WHITE;
    }
    
    // 根据色相判断其他颜色
    for (int i = 1; i < COLOR_COUNT; i++) {
        if (i == COLOR_BLACK || i == COLOR_WHITE) {
            continue; // 已经处理了黑白色
        }
        
        // 检查是否在HSV阈值范围内
        bool inRange = false;
        
        // 处理红色跨越0度的特殊情况
        if (i == COLOR_RED && colorThresholds[i].minH > colorThresholds[i].maxH) {
            inRange = (h >= colorThresholds[i].minH || h <= colorThresholds[i].maxH);
        } else {
            inRange = (h >= colorThresholds[i].minH && h <= colorThresholds[i].maxH);
        }
        
        if (inRange &&
            s >= colorThresholds[i].minS && s <= colorThresholds[i].maxS &&
            v >= colorThresholds[i].minV && v <= colorThresholds[i].maxV) {
            return static_cast<ColorCode>(i);
        }
    }
    
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

void ColorSensor::setLED(bool on) {
    if (!initialized) return;
    
    if (on) {
        tcs.setInterrupt(false); // LED开
    } else {
        tcs.setInterrupt(true);  // LED关
    }
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

#ifdef DEBUG_COLOR_SENSOR
// 调试方法
void ColorSensor::printDebugInfo() {
    if (!initialized) {
        Logger::warning("颜色传感器未初始化");
        return;
    }
    
    ColorCode color = readColor();
    const char* colorName = "未知";
    
    switch (color) {
        case COLOR_RED: colorName = "红色"; break;
        case COLOR_BLUE: colorName = "蓝色"; break;
        case COLOR_YELLOW: colorName = "黄色"; break;
        case COLOR_WHITE: colorName = "白色"; break;
        case COLOR_BLACK: colorName = "黑色"; break;
        default: colorName = "未知"; break;
    }
    
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    
    Logger::debug("=== 颜色传感器调试信息 ===");
    Logger::debug("R: %d, G: %d, B: %d, C: %d", r, g, b, c);
    Logger::debug("H: %.1f, S: %.2f, V: %.2f", h, s, v);
    Logger::debug("检测到颜色: %s", colorName);
    Logger::debug("==========================");
}

void ColorSensor::printRGBValues() {
    if (!initialized) {
        Logger::warning("颜色传感器未初始化");
        return;
    }
    
    Logger::debug("RGB原始值: R=%d, G=%d, B=%d, C=%d", r, g, b, c);
    
    float normR, normG, normB;
    calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
    Logger::debug("RGB归一化值: R=%.2f, G=%.2f, B=%.2f", normR, normG, normB);
}

void ColorSensor::printHSVValues() {
    if (!initialized) {
        Logger::warning("颜色传感器未初始化");
        return;
    }
    
    float h, s, v;
    rgbToHSV(r, g, b, &h, &s, &v);
    Logger::debug("HSV值: H=%.1f, S=%.2f, V=%.2f", h, s, v);
}

// 颜色校准功能保留在调试部分
void ColorSensor::calibrateColor(ColorCode color) {
    if (color <= COLOR_UNKNOWN || color >= COLOR_COUNT) {
        Logger::error("无效的颜色代码: %d", color);
        return;
    }
    
    Logger::info("开始校准颜色: %d", color);
    Logger::info("请将传感器放在目标颜色上...");
    
    // 持续读取20次获得平均值
    uint32_t sumR = 0, sumG = 0, sumB = 0, sumC = 0;
    float sumH = 0, sumS = 0, sumV = 0;
    
    for (int i = 0; i < 20; i++) {
        update();
        
        sumR += r;
        sumG += g;
        sumB += b;
        sumC += c;
        
        float h, s, v;
        rgbToHSV(r, g, b, &h, &s, &v);
        sumH += h;
        sumS += s;
        sumV += v;
        
        delay(50);
    }
    
    uint16_t avgR = sumR / 20;
    uint16_t avgG = sumG / 20;
    uint16_t avgB = sumB / 20;
    uint16_t avgC = sumC / 20;
    
    float avgH = sumH / 20.0f;
    float avgS = sumS / 20.0f;
    float avgV = sumV / 20.0f;
    
    // 打印校准结果
    Logger::info("校准结果:");
    Logger::info("RGB: (%d, %d, %d, %d)", avgR, avgG, avgB, avgC);
    Logger::info("HSV: (%.1f, %.2f, %.2f)", avgH, avgS, avgV);
    
    // 调整阈值范围
    float margin = 0.1; // 10%余量
    
    // 更新RGB阈值
    colorThresholds[color].minR = avgR * (1 - margin);
    colorThresholds[color].maxR = avgR * (1 + margin);
    colorThresholds[color].minG = avgG * (1 - margin);
    colorThresholds[color].maxG = avgG * (1 + margin);
    colorThresholds[color].minB = avgB * (1 - margin);
    colorThresholds[color].maxB = avgB * (1 + margin);
    colorThresholds[color].minC = avgC * (1 - margin);
    colorThresholds[color].maxC = avgC * (1 + margin);
    
    // 更新HSV阈值
    colorThresholds[color].minH = fmax(0, avgH - 10);
    colorThresholds[color].maxH = fmin(360, avgH + 10);
    colorThresholds[color].minS = fmax(0, avgS - 0.1);
    colorThresholds[color].maxS = fmin(1.0, avgS + 0.1);
    colorThresholds[color].minV = fmax(0, avgV - 0.1);
    colorThresholds[color].maxV = fmin(1.0, avgV + 0.1);
    
    Logger::info("校准完成!");
}
#endif 
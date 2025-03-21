#include "RGBController.h"
#include "Logger.h"

RGBController::RGBController() : strip(nullptr), pin(RGB_LED_PIN), numPixels(RGB_LED_COUNT), 
                                 initialized(false), animationActive(false), 
                                 currentRed(0), currentGreen(0), currentBlue(0) {
    // 初始化Gamma校正表
    for (int i = 0; i < 256; i++) {
        float x = i;
        x /= 255;
        x = pow(x, 2.5);
        x *= 255;
        gamma8[i] = x;
    }
}

bool RGBController::init(uint8_t ledPin, uint8_t ledCount) {
    if (!initialized) {
        pin = ledPin;
        numPixels = ledCount;
        
        strip = new Adafruit_NeoPixel(numPixels, pin, NEO_GRB + NEO_KHZ800);
        strip->begin();
        strip->clear();
        strip->show();
        initialized = true;
        Logger::info("RGB LED初始化完成，引脚:%d, LED数量:%d", pin, numPixels);
        return true;
    }
    return false;
}

void RGBController::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (!initialized) {
        init();
    }
    
    // 保存当前颜色值
    currentRed = r;
    currentGreen = g;
    currentBlue = b;
    
    for (int i = 0; i < numPixels; i++) {
        strip->setPixelColor(i, strip->Color(
            gamma8[r], 
            gamma8[g], 
            gamma8[b]
        ));
    }
    strip->show();
}

void RGBController::setPixelColor(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
    if (!initialized) {
        init();
    }
    
    if (pixel < numPixels) {
        strip->setPixelColor(pixel, strip->Color(
            gamma8[r], 
            gamma8[g], 
            gamma8[b]
        ));
        strip->show();
    }
}

void RGBController::setColorCode(ColorCode color) {
    switch (color) {
        case COLOR_RED:
            setColor(255, 0, 0);
            break;
        case COLOR_YELLOW:
            setColor(255, 255, 0);
            break;
        case COLOR_BLUE:
            setColor(0, 0, 255);
            break;
        case COLOR_BLACK:
            setColor(0, 0, 0);
            break;
        case COLOR_WHITE:
            setColor(255, 255, 255);
            break;
        default:
            // 紫色表示未知颜色
            setColor(128, 0, 128);
            break;
    }
}

void RGBController::off() {
    if (initialized) {
        strip->clear();
        strip->show();
        
        // 重置当前颜色
        currentRed = 0;
        currentGreen = 0;
        currentBlue = 0;
        
        // 停止任何活动的动画
        animationActive = false;
    }
}

void RGBController::rainbow(uint8_t wait) {
    if (!initialized) {
        init();
    }
    
    for (long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
        for (int i = 0; i < strip->numPixels(); i++) {
            int pixelHue = firstPixelHue + (i * 65536L / strip->numPixels());
            
            // 替换不存在的gamma32和ColorHSV方法
            // 使用HSV到RGB的转换，手动实现彩虹效果
            uint8_t r, g, b;
            hsv2rgb(pixelHue, 255, 150, &r, &g, &b);
            
            // 应用gamma校正
            r = gamma8[r];
            g = gamma8[g];
            b = gamma8[b];
            
            strip->setPixelColor(i, strip->Color(r, g, b));
        }
        strip->show();
        delay(wait);
    }
}

// HSV转RGB
void RGBController::hsv2rgb(uint16_t hue, uint8_t sat, uint8_t val, uint8_t* r, uint8_t* g, uint8_t* b) {
    uint8_t sector = hue / (65536 / 6);
    uint16_t offset = hue % (65536 / 6);
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * offset) >> 8))) >> 8;
    uint8_t t = (val * (255 - ((sat * (65535 - offset)) >> 8))) >> 8;

    switch (sector) {
        case 0:
            *r = val; *g = t; *b = p;
            break;
        case 1:
            *r = q; *g = val; *b = p;
            break;
        case 2:
            *r = p; *g = val; *b = t;
            break;
        case 3:
            *r = p; *g = q; *b = val;
            break;
        case 4:
            *r = t; *g = p; *b = val;
            break;
        default:
            *r = val; *g = p; *b = q;
            break;
    }
}

void RGBController::showPattern(uint8_t pattern, uint8_t r, uint8_t g, uint8_t b, uint8_t wait) {
    if (!initialized) {
        init();
    }
    
    // 保存动画状态
    animationActive = true;
    animationPattern = pattern;
    animationStep = 0;
    lastAnimationUpdate = millis();
    
    // 保存颜色设置
    currentRed = r;
    currentGreen = g;
    currentBlue = b;
    
    switch (pattern) {
        case 0: // 闪烁模式
            for (int j = 0; j < 5; j++) {
                setColor(r, g, b);
                delay(wait);
                off();
                delay(wait);
            }
            break;
            
        case 1: // 旋转模式
            for (int j = 0; j < 3; j++) {
                for (int i = 0; i < numPixels; i++) {
                    strip->clear();
                    setPixelColor(i, r, g, b);
                    delay(wait);
                }
            }
            break;
            
        case 2: // 彩虹模式
            rainbow(wait);
            break;
            
        default:
            break;
    }
    
    // 动画完成
    animationActive = false;
}

void RGBController::update() {
    if (!initialized || !animationActive) {
        return;
    }
    
    // 此函数可用于实现连续的动画更新
    // 而不是像showPattern那样阻塞主循环
    
    uint32_t currentTime = millis();
    uint32_t elapsed = currentTime - lastAnimationUpdate;
    
    // 这里可以根据动画类型和步骤更新灯光状态
    // 例如实现非阻塞的闪烁、旋转等效果
} 
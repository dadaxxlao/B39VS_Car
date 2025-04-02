#include "ColorSensor.h"
#include "../Utils/Logger.h"

ColorSensor::ColorSensor() : 
    i2cAddress(0), 
    initialized(false), 
    isConnected(false),
    lastCommandSent(0),
    lastR(0), lastG(0), lastB(0),
    lastH(0), lastS(0), lastL(0),
    lastReadMillis(0) {
    // 在构造函数中初始化颜色阈值
    initColorThresholds();
}

void ColorSensor::initColorThresholds() {
    // 初始化颜色阈值
    // 注意：这些值需要根据实际测试进行校准
    // 感为传感器使用0-240范围的HSL值
    
    // 红色阈值 (H接近0或接近240)
    colorThresholds[COLOR_RED].minH = 220;
    colorThresholds[COLOR_RED].maxH = 5; // 跨0判断
    colorThresholds[COLOR_RED].minS = 100;
    colorThresholds[COLOR_RED].maxS = 240;
    colorThresholds[COLOR_RED].minL = 60;
    colorThresholds[COLOR_RED].maxL = 180;
    
    // 黄色阈值 (H约60)
    colorThresholds[COLOR_YELLOW].minH = 15;
    colorThresholds[COLOR_YELLOW].maxH = 30;
    colorThresholds[COLOR_YELLOW].minS = 100;
    colorThresholds[COLOR_YELLOW].maxS = 240;
    colorThresholds[COLOR_YELLOW].minL = 60;
    colorThresholds[COLOR_YELLOW].maxL = 180;
    
    // 蓝色阈值 (H约160)
    colorThresholds[COLOR_BLUE].minH = 140;
    colorThresholds[COLOR_BLUE].maxH = 180;
    colorThresholds[COLOR_BLUE].minS = 100;
    colorThresholds[COLOR_BLUE].maxS = 240;
    colorThresholds[COLOR_BLUE].minL = 60;
    colorThresholds[COLOR_BLUE].maxL = 180;
    
    // 黑色阈值 (低亮度L)
    colorThresholds[COLOR_BLACK].minH = 0;
    colorThresholds[COLOR_BLACK].maxH = 240;
    colorThresholds[COLOR_BLACK].minS = 0;
    colorThresholds[COLOR_BLACK].maxS = 240;
    colorThresholds[COLOR_BLACK].minL = 0;
    colorThresholds[COLOR_BLACK].maxL = 40;
    
    // 白色阈值 (高亮度L，低饱和度S)
    colorThresholds[COLOR_WHITE].minH = 0;
    colorThresholds[COLOR_WHITE].maxH = 240;
    colorThresholds[COLOR_WHITE].minS = 0;
    colorThresholds[COLOR_WHITE].maxS = 30;
    colorThresholds[COLOR_WHITE].minL = 180;
    colorThresholds[COLOR_WHITE].maxL = 240;
}

bool ColorSensor::begin() {
    // 使用配置的地址初始化传感器
    i2cAddress = GANWEI_COLOR_SENSOR_ADDR;
    
    // 初始化I2C通讯
    // Wire.begin() 是安全的，可以多次调用
    Wire.begin();
    
    // 尝试与传感器通信
    isConnected = pingSensor();
    initialized = isConnected;
    
    if (initialized) {
        Logger::info("Color", "感为颜色传感器初始化成功");
    } else {
        Logger::error("Color", "无法连接到感为颜色传感器");
    }
    
    return initialized;
}

bool ColorSensor::pingSensor() {
    // 发送ping命令验证传感器连接
    Wire.beginTransmission(i2cAddress);
    Wire.write(CMD_PING);
    uint8_t error = Wire.endTransmission(false); // 不发送停止位
    
    if (error != 0) {
        // I2C通讯错误
        Logger::error("Color", "I2C通讯错误: %d", error);
        isConnected = false;
        return false;
    }
    
    // 请求1字节的响应
    if (Wire.requestFrom(i2cAddress, (uint8_t)1) != 1) {
        Logger::error("Color", "未收到响应");
        isConnected = false;
        return false;
    }
    
    // 读取响应字节
    uint8_t response = Wire.read();
    
    // 验证响应是否正确
    if (response != PING_RESPONSE) {
        Logger::error("Color", "响应不匹配: 0x%02X (预期: 0x%02X)", response, PING_RESPONSE);
        isConnected = false;
        return false;
    }
    
    isConnected = true;
    return true;
}

SensorStatus ColorSensor::checkHealth() {
    if (!initialized) {
        return SensorStatus::NOT_INITIALIZED;
    }
    
    // 通过ping测试传感器是否响应
    if (pingSensor()) {
        return SensorStatus::OK;
    } else {
        return SensorStatus::ERROR_COMM;
    }
}

bool ColorSensor::isInitialized() const {
    return initialized;
}

bool ColorSensor::readSensorData(uint8_t command, uint8_t* dataBuffer, uint8_t numBytes) {
    if (!isConnected) {
        return false;
    }
    
    // 对于RGB和HSL读取，仅在命令变化时发送命令
    if (command != lastCommandSent || (command != CMD_READ_RGB && command != CMD_READ_HSL)) {
        Wire.beginTransmission(i2cAddress);
        Wire.write(command);
        uint8_t error = Wire.endTransmission(false); // 不发送停止位
        
        if (error != 0) {
            Logger::error("Color", "发送命令错误: %d", error);
            return false;
        }
        
        // 更新最后发送的命令（仅对RGB和HSL读取）
        if (command == CMD_READ_RGB || command == CMD_READ_HSL) {
            lastCommandSent = command;
        }
    }
    
    // 请求指定数量的字节
    if (Wire.requestFrom(i2cAddress, numBytes) != numBytes) {
        Logger::error("Color", "读取数据失败，请求 %d 字节", numBytes);
        return false;
    }
    
    // 读取数据到缓冲区
    for (uint8_t i = 0; i < numBytes; i++) {
        if (Wire.available()) {
            dataBuffer[i] = Wire.read();
        } else {
            Logger::error("Color", "数据读取不完整");
            return false;
        }
    }
    
    // 更新最后读取时间
    lastReadMillis = millis();
    return true;
}

bool ColorSensor::sendCommand(uint8_t command) {
    if (!isConnected) {
        return false;
    }
    
    Wire.beginTransmission(i2cAddress);
    Wire.write(command);
    uint8_t error = Wire.endTransmission(true); // 发送停止位
    
    if (error != 0) {
        Logger::error("Color", "发送命令错误: %d", error);
        return false;
    }
    
    // 对于重置命令，清除最后命令状态
    if (command == CMD_RESET) {
        lastCommandSent = 0;
    }
    
    return true;
}

bool ColorSensor::getColorRGB(uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t rgbData[3];
    
    // 读取RGB数据
    if (!readSensorData(CMD_READ_RGB, rgbData, 3)) {
        return false;
    }
    
    // 更新缓存值
    lastR = rgbData[0];
    lastG = rgbData[1];
    lastB = rgbData[2];
    
    // 设置输出参数
    r = lastR;
    g = lastG;
    b = lastB;
    
    return true;
}

bool ColorSensor::getColorHSL(uint8_t& h, uint8_t& s, uint8_t& l) {
    uint8_t hslData[3];
    
    // 读取HSL数据
    if (!readSensorData(CMD_READ_HSL, hslData, 3)) {
        return false;
    }
    
    // 更新缓存值
    lastH = hslData[0];
    lastS = hslData[1];
    lastL = hslData[2];
    
    // 设置输出参数
    h = lastH;
    s = lastS;
    l = lastL;
    
    return true;
}

bool ColorSensor::getErrorStatus(uint8_t& errorByte) {
    return readSensorData(CMD_READ_ERROR, &errorByte, 1);
}

bool ColorSensor::getFirmwareVersion(uint8_t& versionByte) {
    return readSensorData(CMD_READ_VERSION, &versionByte, 1);
}

bool ColorSensor::resetSensor() {
    bool result = sendCommand(CMD_RESET);
    
    // 重置后等待传感器恢复
    delay(50);
    
    // 重置命令状态
    lastCommandSent = 0;
    
    return result;
}

ColorCode ColorSensor::identifyColorHSL(uint8_t h, uint8_t s, uint8_t l) {
    // 优先处理极端情况
    // 黑色判断：亮度低
    if (l <= colorThresholds[COLOR_BLACK].maxL) {
        return COLOR_BLACK;
    }
    
    // 白色判断：亮度高，饱和度低
    if (l >= colorThresholds[COLOR_WHITE].minL && s <= colorThresholds[COLOR_WHITE].maxS) {
        return COLOR_WHITE;
    }
    
    // 处理红色（可能跨越0度）
    if (colorThresholds[COLOR_RED].minH > colorThresholds[COLOR_RED].maxH) {
        // 跨零度处理
        if ((h >= colorThresholds[COLOR_RED].minH || h <= colorThresholds[COLOR_RED].maxH) &&
            s >= colorThresholds[COLOR_RED].minS && s <= colorThresholds[COLOR_RED].maxS &&
            l >= colorThresholds[COLOR_RED].minL && l <= colorThresholds[COLOR_RED].maxL) {
            return COLOR_RED;
        }
    } else {
        // 标准范围判断
        if (h >= colorThresholds[COLOR_RED].minH && h <= colorThresholds[COLOR_RED].maxH &&
            s >= colorThresholds[COLOR_RED].minS && s <= colorThresholds[COLOR_RED].maxS &&
            l >= colorThresholds[COLOR_RED].minL && l <= colorThresholds[COLOR_RED].maxL) {
            return COLOR_RED;
        }
    }
    
    // 检查其他颜色
    for (int i = 1; i < COLOR_COUNT; i++) {
        // 跳过已处理的颜色和未知颜色
        if (i == COLOR_RED || i == COLOR_BLACK || i == COLOR_WHITE || i == COLOR_UNKNOWN) {
            continue;
        }
        
        if (h >= colorThresholds[i].minH && h <= colorThresholds[i].maxH &&
            s >= colorThresholds[i].minS && s <= colorThresholds[i].maxS &&
            l >= colorThresholds[i].minL && l <= colorThresholds[i].maxL) {
            return static_cast<ColorCode>(i);
        }
    }
    
    // 未能匹配任何颜色
    return COLOR_UNKNOWN;
}

ColorCode ColorSensor::getColor() {
    uint8_t h, s, l;
    
    // 读取HSL值
    if (!getColorHSL(h, s, l)) {
        return COLOR_UNKNOWN;
    }
    
    // 使用HSL值识别颜色
    return identifyColorHSL(h, s, l);
}

void ColorSensor::debugPrint() {
    if (!initialized) {
        Logger::warning("Color", "颜色传感器未初始化");
        return;
    }
    
    // 打印连接状态
    Logger::debug("Color", "传感器状态: 初始化=%d, 已连接=%d, 地址=0x%02X",
                 initialized, isConnected, i2cAddress);
    
    // 读取最新的RGB值
    uint8_t r, g, b;
    bool rgbSuccess = getColorRGB(r, g, b);
    
    if (rgbSuccess) {
        Logger::debug("Color", "RGB值: R=%d, G=%d, B=%d", r, g, b);
    } else {
        Logger::debug("Color", "RGB值读取失败，使用缓存: R=%d, G=%d, B=%d", lastR, lastG, lastB);
    }
    
    // 读取最新的HSL值
    uint8_t h, s, l;
    bool hslSuccess = getColorHSL(h, s, l);
    
    if (hslSuccess) {
        Logger::debug("Color", "HSL值: H=%d, S=%d, L=%d", h, s, l);
    } else {
        Logger::debug("Color", "HSL值读取失败，使用缓存: H=%d, S=%d, L=%d", lastH, lastS, lastL);
    }
    
    // 读取错误状态
    uint8_t errorByte;
    if (getErrorStatus(errorByte)) {
        Logger::debug("Color", "错误状态: 0x%02X", errorByte);
    }
    
    // 读取固件版本
    uint8_t versionByte;
    if (getFirmwareVersion(versionByte)) {
        Logger::debug("Color", "固件版本: 0x%02X", versionByte);
    }
    
    // 识别颜色
    ColorCode color;
    if (hslSuccess) {
        color = identifyColorHSL(h, s, l);
    } else {
        color = identifyColorHSL(lastH, lastS, lastL);
    }
    
    // 打印颜色名称
    const char* colorName;
    switch (color) {
        case COLOR_RED:    colorName = "红色"; break;
        case COLOR_YELLOW: colorName = "黄色"; break;
        case COLOR_BLUE:   colorName = "蓝色"; break;
        case COLOR_BLACK:  colorName = "黑色"; break;
        case COLOR_WHITE:  colorName = "白色"; break;
        default:           colorName = "未知"; break;
    }
    
    Logger::debug("Color", "检测到的颜色: %s", colorName);
} 
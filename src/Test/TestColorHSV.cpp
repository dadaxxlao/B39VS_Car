#ifdef TEST_COLOR_HSV
#include <Arduino.h>
#include "../src/Sensor/ColorSensor.h"
#include "../src/Utils/Logger.h"

ColorSensor colorSensor;

// 测试模式
enum TestMode {
    MODE_DETECT,     // 颜色检测模式
    MODE_CALIBRATE,  // 颜色校准模式
    MODE_RAW_DATA    // 原始数据模式
};

TestMode currentMode = MODE_DETECT;
ColorCode calibrateColor = COLOR_RED; // 默认校准红色

// 颜色名称映射
const char* colorNames[] = {
    "红色",
    "黄色",
    "蓝色",
    "黑色",
    "白色",
    "未知"
};

void printCommandHelp() {
    Logger::info("======= 颜色识别HSV测试程序 =======");
    Logger::info("可用命令:");
    Logger::info("1 - 切换到颜色检测模式");
    Logger::info("2 - 切换到颜色校准模式");
    Logger::info("3 - 切换到原始数据模式");
    Logger::info("d - 执行颜色检测");
    Logger::info("c - 执行颜色校准");
    Logger::info("r - 显示原始数据");
    Logger::info("n - 切换校准目标颜色");
    Logger::info("h - 显示帮助信息");
    Logger::info("当前模式：%s", 
        currentMode == MODE_DETECT ? "颜色检测模式" : 
        currentMode == MODE_CALIBRATE ? "颜色校准模式" : "原始数据模式");
    
    if (currentMode == MODE_CALIBRATE) {
        Logger::info("当前校准目标：%s", colorNames[calibrateColor]);
    }
}

void switchMode(TestMode newMode) {
    currentMode = newMode;
    
    if (currentMode == MODE_CALIBRATE) {
        Logger::info("进入颜色校准模式");
        Logger::info("校准目标：%s", colorNames[calibrateColor]);
        Logger::info("输入'c'开始校准，输入'n'切换目标颜色");
    } else if (currentMode == MODE_DETECT) {
        Logger::info("进入颜色检测模式");
        Logger::info("输入'd'开始检测");
    } else if (currentMode == MODE_RAW_DATA) {
        Logger::info("进入原始数据模式");
        Logger::info("输入'r'显示原始数据");
    }
}

void switchCalibrateColor() {
    // 切换校准目标颜色
    calibrateColor = static_cast<ColorCode>((calibrateColor + 1) % COLOR_UNKNOWN);
    Logger::info("校准目标切换为：%s", colorNames[calibrateColor]);
}

void setup() {
    Serial.begin(9600);
    delay(1000); // 等待串口准备好
    
    Logger::init();
    Logger::info("颜色识别HSV测试程序启动");
    
    // 初始化颜色传感器
    if (!colorSensor.begin()) {
        Logger::error("颜色传感器初始化失败！");
        while (1) { delay(1000); }
    }
    
    printCommandHelp();
}

void loop() {
    // 处理串口输入
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch (cmd) {
            // 切换模式命令
            case '1':
                switchMode(MODE_DETECT);
                break;
            case '2':
                switchMode(MODE_CALIBRATE);
                break;
            case '3':
                switchMode(MODE_RAW_DATA);
                break;
                
            // 执行操作命令
            case 'd': // 检测
                if (currentMode == MODE_DETECT) {
                    performDetection();
                } else {
                    Logger::info("请先切换到颜色检测模式 (输入'1')");
                }
                break;
                
            case 'c': // 校准
                if (currentMode == MODE_CALIBRATE) {
                    performCalibration();
                } else {
                    Logger::info("请先切换到颜色校准模式 (输入'2')");
                }
                break;
                
            case 'r': // 原始数据
                if (currentMode == MODE_RAW_DATA) {
                    showRawData();
                } else {
                    Logger::info("请先切换到原始数据模式 (输入'3')");
                }
                break;
                
            case 'n': // 切换校准颜色
                if (currentMode == MODE_CALIBRATE) {
                    switchCalibrateColor();
                } else {
                    Logger::info("请先切换到颜色校准模式 (输入'2')");
                }
                break;
                
            case 'h': // 帮助
                printCommandHelp();
                break;
                
            // 处理回车和换行等控制字符
            case '\r':
            case '\n':
            case ' ':
                break;
                
            default:
                Logger::info("未知命令: %c", cmd);
                Logger::info("输入'h'获取帮助");
                break;
        }
        
        // 清空串口缓冲区
        while (Serial.available() > 0) {
            Serial.read();
        }
    }
    
    delay(10);
}

void performDetection() {
    Logger::info("正在进行颜色检测...");
    
    // 使用新的颜色识别算法
    ColorCode colorCode = colorSensor.readColor();
    Logger::info("检测到颜色: %s", colorNames[colorCode]);
    
    // 比较RGB和HSV算法结果
    Logger::info("---HSV和RGB算法比较---");
    uint16_t r, g, b, c;
    colorSensor.getRGB(&r, &g, &b, &c);
    
    ColorCode colorRGB = colorSensor.identifyColorRGB(r, g, b, c);
    ColorCode colorHSV = colorSensor.identifyColorHSV(r, g, b, c);
    
    Logger::info("RGB算法: %s", colorNames[colorRGB]);
    Logger::info("HSV算法: %s", colorNames[colorHSV]);
    Logger::info("综合结果: %s", colorNames[colorCode]);
    
    // 打印传感器详细信息
    colorSensor.debugPrint();
}

void performCalibration() {
    Logger::info("开始校准 %s 颜色...", colorNames[calibrateColor]);
    
    // 使用ColorSensor类的校准功能
    colorSensor.calibrateColor(calibrateColor);
    
    Logger::info("校准完成。输入'c'继续校准，输入'n'切换目标颜色");
}

void showRawData() {
    Logger::info("显示原始传感器数据");
    
    // 更新传感器数据
    colorSensor.update();
    
    // 使用ColorSensor类的原始数据函数
    colorSensor.printRawValues();
    
    Logger::info("数据显示完成。输入'r'继续显示原始数据");
} 
#endif
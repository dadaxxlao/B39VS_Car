#ifdef TEST_COLOR_SENSOR
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/ColorSensor.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 全局对象
ColorSensor colorSensor;
unsigned long lastUpdateTime = 0;
unsigned long updateInterval = 500; // 默认500ms更新一次

// 交互模式控制
bool continuousMode = true;        // 是否持续输出数据
bool verboseMode = false;          // 是否显示详细信息
ColorCode targetColor = COLOR_UNKNOWN; // 当前观察的目标颜色

// 将颜色代码转换为字符串
const char* colorToString(ColorCode color) {
  switch (color) {
    case COLOR_RED:     return "红色";
    case COLOR_BLUE:    return "蓝色";
    case COLOR_YELLOW:  return "黄色";
    case COLOR_BLACK:   return "黑色";
    case COLOR_WHITE:   return "白色";
    case COLOR_UNKNOWN: return "未知";
    default:            return "错误";
  }
}

// 打印传感器状态信息
void printSensorInfo() {
  Logger::info("传感器状态:");
  Logger::info("  初始化: %s", colorSensor.isInitialized() ? "是" : "否");
  
  SensorStatus health = colorSensor.checkHealth();
  const char* healthStr;
  switch (health) {
    case SensorStatus::OK:              healthStr = "正常"; break;
    case SensorStatus::NOT_INITIALIZED: healthStr = "未初始化"; break;
    case SensorStatus::ERROR_COMM:      healthStr = "通信错误"; break;
    default:                            healthStr = "未知状态"; break;
  }
  Logger::info("  健康状态: %s", healthStr);
}

// 读取并打印RGB数据
void readAndPrintRGB() {
  uint8_t r, g, b;
  if (colorSensor.getColorRGB(r, g, b)) {
    Logger::info("RGB: R=%d, G=%d, B=%d", r, g, b);
  } else {
    Logger::error("RGB读取失败");
  }
}

// 读取并打印HSL数据
void readAndPrintHSL() {
  uint8_t h, s, l;
  if (colorSensor.getColorHSL(h, s, l)) {
    Logger::info("HSL: H=%d, S=%d, L=%d", h, s, l);
  } else {
    Logger::error("HSL读取失败");
  }
}

// 打印彩色传感器菜单
void printMenu() {
  Logger::info("======= 感为颜色传感器测试菜单 =======");
  Logger::info("c - 切换连续/手动模式 (当前: %s)", continuousMode ? "连续" : "手动");
  Logger::info("v - 切换详细模式 (当前: %s)", verboseMode ? "开启" : "关闭");
  Logger::info("r - 读取RGB数据");
  Logger::info("h - 读取HSL数据");
  Logger::info("i - 显示传感器信息");
  Logger::info("d - 详细诊断");
  Logger::info("p - ping测试传感器连接");
  Logger::info("f - 读取固件版本");
  Logger::info("e - 读取错误状态");
  Logger::info("s - 重置传感器");
  Logger::info("1-5 - 观察特定颜色 (1=红, 2=蓝, 3=黄, 4=白, 5=黑)");
  Logger::info("? - 显示此菜单");
  Logger::info("===================================");
}

// 执行基本的颜色测试和显示
void performColorTest() {
  // 读取颜色
  ColorCode color = colorSensor.getColor();
  
  // 打印识别的颜色
  Logger::info("检测到颜色: %s", colorToString(color));
  
  // 在详细模式下显示更多信息
  if (verboseMode) {
    readAndPrintRGB();
    readAndPrintHSL();
  }
  
  // 如果目标颜色被设置，检查是否匹配
  if (targetColor != COLOR_UNKNOWN) {
    if (color == targetColor) {
      Logger::info("✓ 成功识别目标颜色: %s", colorToString(targetColor));
    }
  }
}

// 处理串口命令
void processCommand(char command) {
  switch (command) {
    case 'c':
      continuousMode = !continuousMode;
      Logger::info("连续模式: %s", continuousMode ? "开启" : "关闭");
      break;
      
    case 'v':
      verboseMode = !verboseMode;
      Logger::info("详细模式: %s", verboseMode ? "开启" : "关闭");
      break;
      
    case 'r':
      readAndPrintRGB();
      break;
      
    case 'h':
      readAndPrintHSL();
      break;
      
    case 'i':
      printSensorInfo();
      break;
      
    case 'd':
      Logger::info("详细诊断:");
      colorSensor.debugPrint();
      break;
      
    case 'p': {
      Logger::info("Ping测试...");
      if (colorSensor.pingSensor()) {
        Logger::info("Ping成功: 传感器已连接");
      } else {
        Logger::error("Ping失败: 传感器未响应");
      }
      break;
    }
      
    case 'f': {
      uint8_t version;
      if (colorSensor.getFirmwareVersion(version)) {
        Logger::info("固件版本: 0x%02X", version);
      } else {
        Logger::error("无法读取固件版本");
      }
      break;
    }
      
    case 'e': {
      uint8_t errorByte;
      if (colorSensor.getErrorStatus(errorByte)) {
        Logger::info("错误状态: 0x%02X", errorByte);
      } else {
        Logger::error("无法读取错误状态");
      }
      break;
    }
      
    case 's':
      Logger::info("重置传感器...");
      if (colorSensor.resetSensor()) {
        Logger::info("重置成功");
      } else {
        Logger::error("重置失败");
      }
      break;
      
    case '1':
      Logger::info("设置观察目标颜色: 红色");
      targetColor = COLOR_RED;
      break;
      
    case '2':
      Logger::info("设置观察目标颜色: 蓝色");
      targetColor = COLOR_BLUE;
      break;
      
    case '3':
      Logger::info("设置观察目标颜色: 黄色");
      targetColor = COLOR_YELLOW;
      break;
      
    case '4':
      Logger::info("设置观察目标颜色: 白色");
      targetColor = COLOR_WHITE;
      break;
      
    case '5':
      Logger::info("设置观察目标颜色: 黑色");
      targetColor = COLOR_BLACK;
      break;
      
    case '0':
      Logger::info("取消目标颜色设置");
      targetColor = COLOR_UNKNOWN;
      break;
      
    case '?':
      printMenu();
      break;
      
    default:
      if (command != '\n' && command != '\r') {
        Logger::warning("未知命令: %c", command);
        Logger::info("输入 '?' 查看帮助");
      }
      break;
  }
}

void setup() {
  // 初始化日志系统
  Logger::init();
  
  // 设置串口波特率为9600，与测试README文档相符
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  // 设置日志级别
  Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_DEBUG);
  Logger::setLogTag(COMM_SERIAL, "ColorTest");
  
  Logger::info("===============================");
  Logger::info("   感为颜色传感器测试程序");
  Logger::info("===============================");
  Logger::info("正在初始化传感器...");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化颜色传感器
  if (colorSensor.begin()) {
    Logger::info("颜色传感器初始化成功");
    
    // 读取固件版本
    uint8_t version;
    if (colorSensor.getFirmwareVersion(version)) {
      Logger::info("固件版本: 0x%02X", version);
    }
    
    // 检查健康状态
    SensorStatus health = colorSensor.checkHealth();
    Logger::info("传感器状态: %s", 
                health == SensorStatus::OK ? "正常" :
                health == SensorStatus::NOT_INITIALIZED ? "未初始化" :
                health == SensorStatus::ERROR_COMM ? "通信错误" : "未知状态");
  } else {
    Logger::error("颜色传感器初始化失败，请检查连接");
    Logger::error("I2C地址: 0x%02X", GANWEI_COLOR_SENSOR_ADDR);
    Logger::error("请确认传感器连接和地址设置正确");
    
    // 不退出循环，让用户可以看到错误消息
  }
  
  delay(500);
  printMenu();
}

void loop() {
  // 检查是否有串口命令
  if (Serial.available()) {
    char c = Serial.read();
    processCommand(c);
  }
  
  // 在连续模式下，每隔 updateInterval 毫秒更新一次
  if (continuousMode && millis() - lastUpdateTime > updateInterval) {
    performColorTest();
    lastUpdateTime = millis();
  }
  
  // 延时以避免过于频繁的输出
  delay(50);
}
#endif 
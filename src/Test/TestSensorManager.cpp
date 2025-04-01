#ifdef TEST_SENSOR_MANAGER
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/SensorManager.h"
#include "../Sensor/SensorCommon.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

SensorManager sensorManager;

// 将颜色代码转换为字符串
const char* colorToString(ColorCode color) {
  switch (color) {
    case COLOR_RED:
      return "红色";
    case COLOR_BLUE:
      return "蓝色";
    case COLOR_YELLOW:
      return "黄色";
    case COLOR_WHITE:
      return "白色";
    case COLOR_BLACK:
      return "黑色";
    default:
      return "未知";
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
  Logger::setLogTag(COMM_SERIAL, "TestSensor");
  
  Logger::info("传感器管理器综合测试程序");
  
  // 初始化所有传感器
  if (sensorManager.initAllSensors()) {
    Logger::info("所有传感器初始化成功");
  } else {
    Logger::warning("部分传感器初始化失败，测试可能不完整");
  }
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 更新所有传感器数据
  sensorManager.updateAll();
  
  // 测试超声波传感器
  Logger::info("======= 超声波传感器 =======");
  float distance;
  if (sensorManager.getDistanceCm(distance)) {
    Logger::info("距离: %.2f cm", distance);
  } else {
    Logger::warning("无法获取有效的距离值");
  }
  
  // 测试红外传感器
  Logger::info("======= 红外传感器 =======");
  int linePos;
  if (sensorManager.getLinePosition(linePos)) {
    Logger::info("线位置: %d (%s)", linePos, 
                linePos < -50 ? "偏左" : (linePos > 50 ? "偏右" : "居中"));
  } else {
    Logger::warning("无法获取有效的线位置");
    linePos = 0; // 默认值用于后续处理
  }
  
  bool lineDetected = sensorManager.isLineDetected();
  Logger::info("线检测: %s", lineDetected ? "检测到线" : "未检测到线");
  
  // 获取并打印红外传感器原始值
  uint16_t irValues[8];
  if (sensorManager.getInfraredSensorValues(irValues)) {
    Logger::info("传感器值: [%d, %d, %d, %d, %d, %d, %d, %d]", 
                irValues[0], irValues[1], irValues[2], irValues[3], 
                irValues[4], irValues[5], irValues[6], irValues[7]);
  } else {
    Logger::warning("无法获取有效的红外传感器值");
  }
  
  // 测试颜色传感器
  Logger::info("======= 颜色传感器 =======");
  ColorCode color = sensorManager.getColor();
  Logger::info("检测到颜色: %s", colorToString(color));
  
  // 获取并打印RGB原始值
  uint16_t r, g, b, c;
  if (sensorManager.getColorSensorValues(r, g, b, c)) {
    Logger::info("R: %d, G: %d, B: %d, C: %d", r, g, b, c);
  } else {
    Logger::warning("无法获取有效的颜色传感器值");
  }
  
  // 打印详细调试信息
  sensorManager.printSensorDebugInfo(SensorType::COLOR);
  
  // 分隔线
  Logger::info("------------------------------");
  
  delay(1500); // 每1.5秒更新一次
} 
#endif
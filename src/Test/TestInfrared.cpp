#ifdef TEST_INFRARED
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

InfraredArray infraredSensor;

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
  Logger::setLogTag(COMM_SERIAL, "IRTest");
  
  Logger::info("红外线阵列传感器测试程序");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化红外传感器
  if (infraredSensor.begin()) {
    Logger::info("红外传感器初始化成功");
  } else {
    Logger::error("红外传感器初始化失败，请检查连接");
    while (1); // 停止执行
  }
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 更新传感器数据
  infraredSensor.update();
  
  // 获取线位置
  int position;
  if (infraredSensor.getLinePosition(position)) {
    Logger::info("线位置: %d (%s)", position, 
               position < -40 ? "偏左" : (position > 40 ? "偏右" : "居中"));
  } else {
    Logger::warning("无法获取有效的线位置");
  }
  
  // 检测是否有线
  bool lineDetected = infraredSensor.isLineDetected();
  Logger::info("线检测: %s", lineDetected ? "检测到线" : "未检测到线");
  
  // 打印传感器原始值
  uint16_t values[8];
  if (infraredSensor.getAllSensorValues(values)) {
    Logger::info("传感器值: [%d, %d, %d, %d, %d, %d, %d, %d]", 
               values[0], values[1], values[2], values[3], 
               values[4], values[5], values[6], values[7]);
  } else {
    Logger::warning("无法获取有效的传感器值");
  }
  
  delay(500); // 每500ms更新一次
} 
#endif
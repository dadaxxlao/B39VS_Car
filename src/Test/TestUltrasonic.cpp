#ifdef TEST_ULTRASONIC
#include <Arduino.h>
#include "../Sensor/Ultrasonic.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"
#include "../Sensor/SensorCommon.h"

UltrasonicSensor ultrasonicSensor;

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
  Logger::setLogTag(COMM_SERIAL, "UltrasTest");
  
  Logger::info("超声波传感器测试程序");
  
  // 初始化超声波传感器
  if(ultrasonicSensor.init()) {
    Logger::info("超声波传感器初始化成功");
  } else {
    Logger::error("超声波传感器初始化失败");
    while(1);
  }
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 获取距离
  float distance;
  bool success = false;
  
  // 测量新的距离
  unsigned long duration = ultrasonicSensor.measurePulseDuration();
  if (duration > 0 && duration < ULTRASONIC_PULSE_TIMEOUT) {
    distance = ultrasonicSensor.getDistanceCmFromDuration(duration);
    success = true;
  }
  
  // 打印距离信息
  if (success) {
    Logger::info("距离: %.2f cm", distance);
    
    // 检测是否有障碍物
    bool obstacle = (distance <= NO_OBJECT_THRESHOLD);
    Logger::info("障碍物检测: %s", obstacle ? "有障碍物" : "无障碍物");
    
    // 检测是否达到抓取距离
    bool canGrab = (distance <= GRAB_DISTANCE);
    Logger::info("抓取距离: %s", canGrab ? "可以抓取" : "距离太远");
  } else {
    Logger::warning("无法测量有效距离");
  }
  
  delay(500); // 每500ms更新一次
} 
#endif
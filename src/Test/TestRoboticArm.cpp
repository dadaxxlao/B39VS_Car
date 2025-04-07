/*
 * TestRoboticArm.cpp
 * 
 * 机械臂测试程序，用于验证机械臂的基本功能
 * 包括初始化、夹取物块、放置物块和归位等操作
 */

#ifdef TEST_ROBOTIC_ARM

#include <Arduino.h>
#include "../Arm/RoboticArm.h"
#include "../Utils/Logger.h"
#include "../Control/LineFollower.h"
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Control/NavigationController.h"

// 全局对象
RoboticArm roboticArm;
SensorManager sensorManager;
LineFollower lineFollower(sensorManager);
MotionController motionController;
NavigationController navigationController(sensorManager, motionController, lineFollower);
bool hasCompleted = false; // 标记是否已完成一次完整操作
bool isFollowingLine = true; // 标记是否正在循迹

// 初始化函数
void setup() {
  // 初始化日志系统
  Logger::init();
  Serial.begin(115200);
  Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_DEBUG);
  
  // 打印程序版本和启动信息
  Logger::info("Test", "B39VS 机械臂超声波抓取测试程序");
  Logger::info("Test", "初始化中...");
  
  // 初始化机械臂
  roboticArm.init();
  delay(500);
  roboticArm.calibrate();
  
  // 初始化所有传感器
  if (!sensorManager.initAllSensors()) {
    Logger::error("Test", "传感器初始化失败");
    while(1); // 如果传感器初始化失败，停止程序
  }
  
  // 初始化电机控制
  motionController.init();
  
  // 初始化循迹模块
  lineFollower.init();
  
  // 初始化导航控制器
  navigationController.init();
  
  // 打印初始化完成
  Logger::info("Test", "初始化完成，开始循迹前进");
}

// 主循环
void loop() {
  if (hasCompleted) {
    return; // 如果已完成一次操作，直接返回
  }
  
  if (isFollowingLine) {
    // 更新传感器数据
    sensorManager.updateAll();
    
    // 更新导航控制器状态
    navigationController.update();
    
    // 检查导航状态
    NavigationState navState = navigationController.getCurrentNavigationState();
    
    if (navState == NAV_ERROR) {
      Logger::error("Test", "导航控制器处于错误状态！");
      motionController.emergencyStop();
      while(1); // 停止程序
    }
    
    // 检查超声波距离
    if (roboticArm.checkGrabCondition()) {
      // 停止小车
      motionController.emergencyStop();
      isFollowingLine = false;
      Logger::info("Test", "检测到物体，停止循迹，开始抓取");
    }
  } else {
    // 执行机械臂操作
    Logger::info("Test", "1. 执行抓取操作");
    
    // 1.1 移动到抓取位置并打开夹爪
    Logger::info("Test", "移动到抓取位置并打开夹爪");
    roboticArm.adjustArm(2150, 450, -50);
    delay(2000);
    
    // 1.2 闭合夹爪抓取物体
    Logger::info("Test", "闭合夹爪抓取物体");
    roboticArm.adjustArm(2150, 450, 900);
    delay(2000);
    
    // 1.3 抬起物体
    Logger::info("Test", "抬起物体");
    roboticArm.adjustArm(1500, 1600, 900);
    delay(2000);
    
    // 1.4 微微松开夹爪
    Logger::info("Test", "微微松开夹爪");
    roboticArm.adjustArm(1500, 1600, 600);
    delay(3000); // 等待3秒
    
    // 1.5 重新夹紧
    Logger::info("Test", "重新夹紧");
    roboticArm.adjustArm(1500, 1600, 900);
    delay(2000);
    
    // 1.6 放下物体
    Logger::info("Test", "放下物体");
    roboticArm.adjustArm(2150, 450, 900);
    delay(2000);
    
    // 1.7 完全松开夹爪
    Logger::info("Test", "完全松开夹爪");
    roboticArm.adjustArm(2150, 450, -50);
    delay(2000);
    
    Logger::info("Test", "抓取操作完成");
    
    // 2. 复位机械臂
    Logger::info("Test", "2. 复位机械臂");
    roboticArm.reset();
    delay(2000);
    
    Logger::info("Test", "程序执行完成，等待重启");
    hasCompleted = true; // 标记已完成
  }
  
  delay(30); // 主循环延迟，避免过度消耗CPU
}

#endif // TEST_ROBOTIC_ARM 
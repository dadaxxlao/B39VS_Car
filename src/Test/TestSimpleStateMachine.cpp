#ifdef TEST_SIMPLE_STATE_MACHINE
/**
 * SimpleStateMachine简化测试程序
 * 
 * 这是简化版状态机的最小测试程序，只包含：
 * - 初始化所有必要组件
 * - 更新传感器数据
 * - 执行简化版状态机逻辑
 * - 处理基本串口命令（启动/停止）
 * - 支持ESP32串口桥接通信
 */

#include <Arduino.h>
#include "../Sensor/SensorManager.h"    
#include "../Motor/MotionController.h"   
#include "../Arm/RoboticArm.h"           
#include "../Control/NavigationController.h" 
#include "../Control/LineFollower.h"   
#include "../Control/SimpleStateMachine.h"      
#include "../Utils/Logger.h"           
#include "../Utils/Config.h"           

// --- 全局对象 ---
SensorManager sensorManager;
MotionController motionController;
RoboticArm roboticArm;
LineFollower lineFollower(sensorManager); 
NavigationController navigationController(sensorManager, motionController, lineFollower);
SimpleStateMachine stateMachine(sensorManager, motionController, roboticArm, navigationController);

// --- 主程序配置 ---
const int LOOP_DELAY_MS = 50; // 主循环延迟（毫秒）
bool systemInitialized = false;

// --- 初始化函数 ---
void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  delay(1000);
  
  // 初始化Logger系统
  Logger::init();
  
  // 初始化ESP32串口通信 (条件编译，只在ENABLE_ESP启用时)
#if ENABLE_ESP
  Serial2.begin(ESP_BAUD_RATE);
  Serial.println("ESP32通信初始化 (Serial2)");
  
  // 配置Logger使用ESP32通道
  Logger::setStream(COMM_ESP32, &Serial2);
  Logger::enableComm(COMM_ESP32, true);
  Logger::setLogLevel(COMM_ESP32, LOG_LEVEL_INFO); // 设置ESP32日志级别
  
  // 通过ESP32日志通道发送启动消息
  Logger::info("SYSTEM", "Arduino系统启动中，ESP32通信就绪");
#endif
  
  // 初始化所有组件
  if (sensorManager.initAllSensors()) {
    Serial.println("传感器初始化成功");
    Logger::info("SYSTEM", "传感器初始化成功");
  } else {
    Serial.println("传感器初始化失败！系统无法工作");
    Logger::error("SYSTEM", "传感器初始化失败！系统无法工作");
    while (true) { delay(500); }
  }

  motionController.init();
  roboticArm.init();
  roboticArm.calibrate();
  lineFollower.init();
  navigationController.init();
  stateMachine.init();
  
  systemInitialized = true;
  
  Serial.println("系统就绪，按回车键启动任务，输入'q'停止");
  Logger::info("SYSTEM", "系统就绪，按回车键启动任务，输入'q'停止");
}

// --- 处理串口命令通用函数 ---
void processCommand(String command) {
  command.trim();
  
  if (command == "" || command == "\r") {
    // 用户按下回车，启动任务
    stateMachine.handleCommand("START");
    Serial.println("任务已启动");
    Logger::info("CMD", "任务已启动");
  }
  else if (command == "stop" || command == "q" || command == "Q") {
    motionController.emergencyStop();
    navigationController.stop();
    Serial.println("系统已停止");
    Logger::info("CMD", "系统已停止");
  }
  else if (command == "reset") {
    stateMachine.handleCommand("RESET");
    navigationController.init();
    Serial.println("系统已重置");
    Logger::info("CMD", "系统已重置");
  }
}

// --- 主循环函数 ---
void loop() {
  if (!systemInitialized) {
    return;
  }
  
  // 1. 更新传感器数据
  sensorManager.updateAll();
  
  // 2. 更新状态机
  stateMachine.update();
  
  // 3. 处理USB串口命令
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
  
  // 4. 处理ESP32串口命令 (条件编译，只在ENABLE_ESP启用时)
#if ENABLE_ESP
  if (Serial2.available() > 0) {
    String espCommand = Serial2.readStringUntil('\n');
    // 记录收到的ESP命令
    Serial.print("ESP命令: ");
    Serial.println(espCommand);
    // 处理命令
    processCommand(espCommand);
  }
#endif
  
  // 5. 循环延迟
  delay(LOOP_DELAY_MS);
}

#endif // TEST_SIMPLE_STATE_MACHINE
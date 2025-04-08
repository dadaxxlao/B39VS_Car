#ifdef TEST_SIMPLE_STATE_MACHINE
/**
 * SimpleStateMachine简化测试程序
 * 
 * 这是简化版状态机的最小测试程序，只包含：
 * - 初始化所有必要组件
 * - 更新传感器数据
 * - 执行简化版状态机逻辑
 * - 处理基本串口命令（启动/停止）
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
const int LOOP_DELAY_MS = 20; // 主循环延迟（毫秒）
bool systemInitialized = false;

// --- 初始化函数 ---
void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  delay(1000);
  
  // 初始化Logger系统
  Logger::init();
  
  // 初始化所有组件
  if (sensorManager.initAllSensors()) {
    Serial.println("传感器初始化成功");
  } else {
    Serial.println("传感器初始化失败！系统无法工作");
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
  
  // 3. 处理串口命令
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "" || command == "\r") {
      // 用户按下回车，启动任务
      stateMachine.handleCommand("START");
      Serial.println("任务已启动");
    }
    else if (command == "stop" || command == "q" || command == "Q") {
      motionController.emergencyStop();
      navigationController.stop();
      Serial.println("系统已停止");
    }
    else if (command == "reset") {
      stateMachine.handleCommand("RESET");
      navigationController.init();
      Serial.println("系统已重置");
    }
  }
  
  // 4. 循环延迟
  delay(LOOP_DELAY_MS);
}

#endif // TEST_SIMPLE_STATE_MACHINE
#ifdef TEST_STATE_MACHINE
/**
 * StateMachine模块测试文件
 * 
 * 目的：验证StateMachine的功能，包括：
 * - 状态转换逻辑
 * - 路口决策处理
 * - 与NavigationController的交互
 * - 抓取和放置物块的逻辑
 * 
 * 设置：将此文件放置在'src/Test/'目录
 * 确保platformio.ini中的build_flags设置为编译此测试文件而非main.cpp
 * 例如: build_flags = -D TEST_STATE_MACHINE
 * 
 * 硬件要求：Arduino Mega，连接有传感器（通过I2C的红外线阵列）
 * 以及按照Config.h中定义连接的电机
 */

#include <Arduino.h>
#include "../Sensor/SensorManager.h"    
#include "../Motor/MotionController.h"   
#include "../Arm/RoboticArm.h"           
#include "../Control/NavigationController.h" 
#include "../Control/LineFollower.h"   
#include "../Control/StateMachine.h"      
#include "../Utils/Logger.h"           
#include "../Utils/Config.h"           
#include <string.h> // 用于 strcmp, strncmp, strlen
#include <stdlib.h> // 用于 atoi
#include <ctype.h>  // 用于 isspace

// --- 全局对象 ---
SensorManager sensorManager;
MotionController motionController;
RoboticArm roboticArm;
LineFollower lineFollower(sensorManager); 
NavigationController navigationController(sensorManager, motionController, lineFollower);
StateMachine stateMachine(sensorManager, motionController, roboticArm, navigationController);

// --- 测试配置 ---
const int LOOP_DELAY_MS = 50; // 主循环延迟（毫秒）
const char* systemStateToString(SystemState state);
bool waitingForInput = false;

// 添加静态函数声明
static const char* junctionTypeToString(JunctionType type);

// 添加一个函数来去除字符串首尾的空白字符
void trimWhitespace(char* str) {
    if (str == NULL || *str == '\0') {
        return;
    }

    // 去除尾部空白
    char* end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    // 去除首部空白
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // 如果首部有空白，则移动字符串内容
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// --- 初始化函数 ---
void setup() {
  // 初始化串口通信用于日志输出
  Serial.begin(115200);
  // 等待串口连接（对于原生USB端口很重要）
  while (!Serial) {
    delay(10); 
  }

  // 初始化Logger系统
  Logger::init();
  // 设置日志级别为DEBUG以便在测试过程中查看详细信息
  Logger::setGlobalLogLevel(LOG_LEVEL_DEBUG); 
  Logger::info("StateMachineTest", F("--- 状态机测试初始化中 ---"));

  // 初始化传感器管理器
  Logger::info("StateMachineTest", F("正在初始化传感器管理器..."));
  if (sensorManager.initAllSensors()) {
    Logger::info("StateMachineTest", F("传感器管理器初始化成功。"));
  } else {
    Logger::error("StateMachineTest", F("致命错误：传感器管理器初始化失败！停止执行。"));
    // 如果传感器无法初始化，停止执行
    while (true) {
        motionController.emergencyStop(); // 确保电机停止
        delay(1000);
    }
  }

  // 初始化运动控制器
  Logger::info("StateMachineTest", F("正在初始化运动控制器..."));
  motionController.init();
  Logger::info("StateMachineTest", F("运动控制器初始化完成。"));

  // 初始化机械臂
  Logger::info("StateMachineTest", F("正在初始化机械臂..."));
  roboticArm.init();
  Logger::info("StateMachineTest", F("机械臂初始化完成。"));

  // 初始化巡线器
  Logger::info("StateMachineTest", F("正在初始化巡线器..."));
  lineFollower.init();
  Logger::info("StateMachineTest", F("巡线器初始化完成。"));

  // 初始化导航控制器
  Logger::info("StateMachineTest", F("正在初始化导航控制器..."));
  navigationController.init();
  Logger::info("StateMachineTest", F("导航控制器初始化完成。"));

  // 初始化状态机
  Logger::info("StateMachineTest", F("正在初始化状态机..."));
  stateMachine.init();
  Logger::info("StateMachineTest", F("状态机初始化完成。"));

  // 显示命令提示
  Logger::info("StateMachineTest", F("-----------------------------------------"));
  Logger::info("StateMachineTest", F("可用命令："));
  Logger::info("StateMachineTest", F("  start - 启动状态机"));
  Logger::info("StateMachineTest", F("  reset - 重置状态机"));
  Logger::info("StateMachineTest", F("  state - 显示当前状态"));
  Logger::info("StateMachineTest", F("  color [1-5] - 模拟检测到指定颜色"));
  Logger::info("StateMachineTest", F("-----------------------------------------"));
}

// --- 主循环函数 ---
void loop() {
  // 1. 更新传感器数据
  sensorManager.updateAll();

  // 2. 更新状态机
  stateMachine.update();

  // 3. 检查串口命令
  if (Serial.available() > 0) {
    const int MAX_COMMAND_LENGTH = 32; // 定义命令缓冲区的最大长度
    char command_buffer[MAX_COMMAND_LENGTH];

    // 读取命令，最多读取 MAX_COMMAND_LENGTH - 1 个字符，为 null 终止符留空间
    int bytesRead = Serial.readBytesUntil('\n', command_buffer, MAX_COMMAND_LENGTH - 1);
    command_buffer[bytesRead] = '\0'; // 手动添加 null 终止符

    trimWhitespace(command_buffer); // 去除首尾空白

    // 处理命令
    if (strcmp(command_buffer, "start") == 0) {
      Logger::info("StateMachineTest", F("收到启动命令"));
      stateMachine.handleCommand("START");
    }
    else if (strcmp(command_buffer, "reset") == 0) {
      Logger::info("StateMachineTest", F("收到重置命令"));
      stateMachine.handleCommand("RESET");
    }
    else if (strcmp(command_buffer, "state") == 0) {
      SystemState currentState = stateMachine.getCurrentState();
      Logger::info("StateMachineTest", "当前状态: %s", systemStateToString(currentState));
      Logger::info("StateMachineTest", "计数器值: %d", stateMachine.getJunctionCounter());
      Logger::info("StateMachineTest", "检测颜色: %d", stateMachine.getDetectedColor());
    }
    else if (strncmp(command_buffer, "color ", 6) == 0) {
      // 确保 "color " 后面确实有数字
      if (strlen(command_buffer) > 6 && isdigit(command_buffer[6])) {
          int colorValue = atoi(command_buffer + 6); // 从第 6 个字符开始转换
          if (colorValue >= COLOR_RED && colorValue <= COLOR_BLACK) {
              Logger::info("StateMachineTest", "模拟检测颜色: %d", colorValue);
              // 在实际项目中，这里需要一种机制来手动设置颜色
              // 这里只是模拟
          } else {
              Logger::warning("StateMachineTest", "无效的颜色值: %d", colorValue);
          }
      } else {
          Logger::warning("StateMachineTest", F("无效的 color 命令格式"));
      }
    }
    else if (strcmp(command_buffer, "help") == 0) {
      Logger::info("StateMachineTest", F("可用命令："));
      Logger::info("StateMachineTest", F("  start - 启动状态机"));
      Logger::info("StateMachineTest", F("  reset - 重置状态机"));
      Logger::info("StateMachineTest", F("  state - 显示当前状态"));
      Logger::info("StateMachineTest", F("  color [1-5] - 模拟检测到指定颜色"));
    }
    else if (strlen(command_buffer) > 0) { // 避免打印空的未知命令
      Logger::warning("StateMachineTest", "未知命令: %s", command_buffer);
    }
  }

  // 4. 模拟NavigationController中的用户交互
  NavigationState navState = navigationController.getCurrentNavigationState();
  
  if (navState == NAV_AT_JUNCTION && !waitingForInput) {
    JunctionType detectedJunction = navigationController.getDetectedJunctionType();
    
    Serial.print(F("\n检测到路口："));
    Serial.println(junctionTypeToString(detectedJunction));
    Serial.println(F("发送任意字符继续，或输入特定命令:"));
    
    waitingForInput = true;
  } 
  else if (navState != NAV_AT_JUNCTION) {
    waitingForInput = false;
  }

  // 5. 循环延迟
  delay(LOOP_DELAY_MS);
}

// 系统状态转字符串函数
const char* systemStateToString(SystemState state) {
  switch (state) {
    case INITIALIZED: return "INITIALIZED";
    case OBJECT_FIND: return "OBJECT_FIND";
    case ULTRASONIC_DETECT: return "ULTRASONIC_DETECT";
    case ZONE_JUDGE: return "ZONE_JUDGE";
    case ZONE_TO_BASE: return "ZONE_TO_BASE";
    case OBJECT_GRAB: return "OBJECT_GRAB";
    case OBJECT_PLACING: return "OBJECT_PLACING";
    case COUNT_INTERSECTION: return "COUNT_INTERSECTION";
    case OBJECT_RELEASE: return "OBJECT_RELEASE";
    case ERGODIC_JUDGE: return "ERGODIC_JUDGE";
    case RETURN_BASE: return "RETURN_BASE";
    case BASE_ARRIVE: return "BASE_ARRIVE";
    case END: return "END";
    case ERROR_STATE: return "ERROR_STATE";
    default: return "UNKNOWN";
  }
}

// 路口类型转字符串函数（复制或导入自StateMachine.cpp）
static const char* junctionTypeToString(JunctionType type) {
  switch(type) {
    case NO_JUNCTION: return "无路口";
    case T_LEFT: return "T型左路口";
    case T_RIGHT: return "T型右路口";
    case T_FORWARD: return "T型前路口";
    case CROSS: return "十字路口";
    case LEFT_TURN: return "左转弯";
    case RIGHT_TURN: return "右转弯";
    case END_OF_LINE: return "线路终点";
    default: return "未知路口";
  }
}

#endif // TEST_STATE_MACHINE 
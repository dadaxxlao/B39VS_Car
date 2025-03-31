#ifdef TEST_NAVIGATION_CONTROLLER
/**
 * NavigationController模块测试文件
 * 
 * 目的：验证NavigationController的功能，包括：
 * - 使用底层LineFollower进行基本巡线
 * - 检测边缘触发模式
 * - 对路口(T_LEFT, T_RIGHT, LEFT_TURN, RIGHT_TURN)执行"停止检查"程序
 * - 检测T_FORWARD模式
 * - NavigationController内部状态转换
 * - 交互模拟：响应NAV_AT_JUNCTION状态
 * 
 * 状态机关系：
 * - NavigationController是底层控制器，负责导航和识别路口
 * - StateMachine是高层控制器，根据当前状态和路口类型决定行动
 * - 本测试文件模拟了StateMachine的部分功能，特别是处理路口决策的部分
 * 
 * 设置：将此文件放置在'src/Test/'目录或类似位置
 * 确保platformio.ini中的build_flags设置为编译此测试文件而非main.cpp
 * 例如: build_flags = -D TEST_NAVIGATION_CONTROLLER
 * 
 * 硬件要求：Arduino Mega，连接有传感器（通过I2C的红外线阵列）
 * 以及按照Config.h中定义连接的电机
 * 
 * 操作步骤：
 * 1. 将代码上传到Arduino Mega
 * 2. 打开串口监视器（波特率115200）
 * 3. 将机器人放在有各种路口（T型路口，90度转弯）的线路上
 * 4. 观察串口监视器中"NavControllerTest"、"NavCtrl"、"LineFoll"和"LineDet"的日志输出
 * 5. 注意状态转换、路口检测和模拟动作
 */

#include <Arduino.h>
// 根据测试文件相对于实际文件结构调整路径
#include "../Sensor/SensorManager.h"    
#include "../Motor/MotionController.h"   
#include "../Control/LineFollower.h"   
#include "../Control/NavigationController.h" 
#include "../Utils/Logger.h"           
#include "../Utils/Config.h"           

// --- 全局对象 ---
SensorManager sensorManager;
MotionController motionController;
// LineFollower构造需要SensorManager和MotionController
LineFollower lineFollower(sensorManager); 
// NavigationController需要SensorManager、MotionController和LineFollower
NavigationController navigationController(sensorManager, motionController, lineFollower);

// --- 测试配置 ---
const int LOOP_DELAY_MS = 30; // 主循环延迟（毫秒）
                              // 根据性能和观察清晰度需求调整

// --- 函数声明 ---
const char* junctionTypeToString(JunctionType type);

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
  Logger::info("NavControllerTest", "--- 导航控制器测试初始化中 ---");

  // 初始化传感器管理器
  Logger::info("NavControllerTest", "正在初始化传感器管理器...");
  if (sensorManager.initAllSensors()) {
    Logger::info("NavControllerTest", "传感器管理器初始化成功。");
  } else {
    Logger::error("NavControllerTest", "致命错误：传感器管理器初始化失败！停止执行。");
    // 如果传感器无法初始化，停止执行
    while (true) {
        motionController.emergencyStop(); // 确保电机停止
        delay(1000);
    }
  }

  // 初始化运动控制器
  Logger::info("NavControllerTest", "正在初始化运动控制器...");
  motionController.init(); // 初始化电机引脚并停止电机
  Logger::info("NavControllerTest", "运动控制器初始化完成。");

  // 初始化巡线器
  // （构造函数已在上面调用，init重置内部PID状态）
  Logger::info("NavControllerTest", "正在初始化巡线器...");
  lineFollower.init();
  // 可选：为此次测试运行配置特定的PID参数（如有需要）
  // 示例：lineFollower.setPIDParams(1.0, 0.0, 1.0); 
  // 示例：lineFollower.setBaseSpeed(120); 
  Logger::info("NavControllerTest", "巡线器初始化完成。");

  // 初始化导航控制器
  // （构造函数已在上面调用，init重置状态并开始导航）
  Logger::info("NavControllerTest", "正在初始化导航控制器...");
  navigationController.init(); // 设置状态为NAV_FOLLOWING_LINE
  Logger::info("NavControllerTest", "导航控制器初始化完成。开始测试循环。");
  Logger::info("NavControllerTest", "-----------------------------------------");
}

// --- 主循环函数 ---
void loop() {
  // 1. 更新传感器数据（关键步骤！）
  // 这确保NavigationController及其组件
  // 在当前控制周期拥有最新的传感器读数
  // 调用SensorManager中的适当更新函数
  sensorManager.updateAll(); // 假设updateAll()读取必要的传感器（如红外线阵列）
  // 如果只需要红外线，且有特定的updateInfrared()函数，使用：
  // sensorManager.updateInfrared(); 

  // 2. 更新导航控制器状态机
  // 这是驱动NavigationController内部巡线、路口检测
  // 和停止检查逻辑的核心调用
  navigationController.update();

  // 3. 检查导航状态（模拟StateMachine的角色）
  // 获取导航控制器的当前状态
  NavigationState navState = navigationController.getCurrentNavigationState();

  // 如果导航器检测并分类了一个路口...
  if (navState == NAV_AT_JUNCTION) {
    JunctionType detectedJunction = navigationController.getDetectedJunctionType();

    // 停止车辆
    motionController.emergencyStop();
    
    // 记录检测到的路口及其类型
    Serial.print("\n检测到路口：");
    Serial.println(junctionTypeToString(detectedJunction));
    Serial.println("发送任意字符继续...");
    
    // 清空串口缓冲区
    while(Serial.available()) {
      Serial.read();
    }
    
    // 阻塞等待，直到接收到任意串口输入
    while(!Serial.available()) {
      delay(10); // 短延迟，避免过度消耗CPU
    }
    
    // 清空接收到的数据
    while(Serial.available()) {
      Serial.read();
    }
    
    // 收到命令后继续
    Serial.println("继续运行...");
    navigationController.resumeFollowing();
    
    Logger::info("NavControllerTest", "-----------------------------------------");
  } else if (navState == NAV_ERROR) {
    // 如果NavigationController进入错误状态（如传感器故障、丢线超时）
    Logger::error("NavControllerTest", "导航控制器处于NAV_ERROR状态！测试循环停止。");
    motionController.emergencyStop(); // 确保电机停止
    // 出错时无限期暂停测试循环
    while (true) {
        delay(1000);
    }
  } else if (navState == NAV_STOPPED) {
     // 记录NavigationController是否停止（通过调用stop()或初始时）
     Logger::info("NavControllerTest", "导航控制器处于NAV_STOPPED状态。");
     // 对于此测试，我们可能希望在意外停止时自动恢复
     // 或者只是记录状态。这里我们选择记录。
     // 恢复示例：navigationController.resumeFollowing();
  }
  // 其他状态（NAV_FOLLOWING_LINE, NAV_MOVING_TO_STOP, NAV_STOPPED_FOR_CHECK）
  // 由navigationController.update()内部处理并记录
  // 在此测试循环中，除了调用update()外，我们不需要针对它们的特定操作

  // 4. 循环延迟
  // 添加小延迟防止循环运行过快，
  // 这可能使串口输出难以阅读并可能饿死其他进程（如果有的话）
  // 根据观察到的性能和日志可读性调整此值
  delay(LOOP_DELAY_MS); 
}

// 将枚举JunctionType转换为可读字符串
const char* junctionTypeToString(JunctionType type) {
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

#endif // TEST_NAVIGATION_CONTROLLER
#ifdef TEST_JUNCTION_FOLLOWING
#include <Arduino.h>
#include <Wire.h>
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Control/LineDetector.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 创建运动控制器、传感器管理器和线路检测器实例
MotionController motionController;
SensorManager sensorManager;
LineDetector lineDetector;

// PID控制参数
float Kp = 0.5;  // 比例系数
float Kd = 0.1;  // 微分系数
int centerPosition = 3500; // 线位于中央的位置值
int lastError = 0; // 上一次误差

// 测试配置
bool testRunning = false;
bool newCommandReceived = false;
String receivedCommand = "";
unsigned long lastUpdateTime = 0;
unsigned long junctionLastDetectedTime = 0;
const unsigned long JUNCTION_DETECTION_COOLDOWN = 1500; // 路口检测冷却时间（毫秒）

// 路口计数
int junctionCounter = 0;
JunctionType lastJunctionType = NO_JUNCTION;

// 记录路口历史
#define MAX_JUNCTION_HISTORY 10
JunctionType junctionHistory[MAX_JUNCTION_HISTORY];
int junctionHistoryIndex = 0;

// 实现巡线函数
void followLine(int position) {
  // 计算误差
  int error = position - centerPosition;
  int errorChange = error - lastError;
  lastError = error;
  
  // PID计算转向量
  float turnAmount = Kp * error + Kd * errorChange;
  
  // 约束转向量
  turnAmount = constrain(turnAmount, -100, 100);
  
  // 发送速度指令给麦克纳姆轮运动控制器
  float forwardSpeed = 0.8;  // 前进速度分量
  float rotationSpeed = turnAmount / 150.0;  // 旋转分量，归一化到-1.0到1.0
  
  // 限制旋转分量范围
  rotationSpeed = constrain(rotationSpeed, -0.8, 0.8);
  
  // 使用运动控制器的mecanumDrive直接控制
  motionController.mecanumDrive(0, forwardSpeed, rotationSpeed);
}

// 转换路口类型为字符串
const char* junctionTypeToString(JunctionType junction) {
  switch (junction) {
    case NO_JUNCTION: return "无路口";
    case T_LEFT: return "左T字路口";
    case T_RIGHT: return "右T字路口";
    case T_FORWARD: return "倒T字路口";
    case CROSS: return "十字路口";
    case LEFT_TURN: return "左转弯";
    case RIGHT_TURN: return "右转弯";
    case END_OF_LINE: return "线路终点";
    default: return "未知路口";
  }
}

// 执行掉头动作
void performUTurn() {
  Logger::info("执行掉头动作");
  motionController.emergencyStop();
  delay(200);
  
  // 执行掉头动作
  motionController.turnLeft(SHARP_TURN_SPEED);
  delay(1000); // 根据实际电机速度调整
  
  // 等待中央红外检测到线
  while (!sensorManager.isLineDetected()) {
    delay(10);
  }
  
  motionController.emergencyStop();
  delay(200);
}

// 执行路口动作
void executeJunctionAction(JunctionType junction) {
  switch (junction) {
    case T_LEFT:
      Logger::info("左T字路口：左转");
      motionController.turnLeft(TURN_SPEED);
      break;
      
    case T_RIGHT:
      Logger::info("右T字路口：右转");
      motionController.turnRight(TURN_SPEED);
      break;
      
    case T_FORWARD:
      Logger::info("倒T字路口：右转");
      motionController.turnRight(TURN_SPEED);
      break;
      
    case CROSS:
      Logger::info("十字路口：直行");
      motionController.moveForward(FOLLOW_SPEED);
      break;
      
    case LEFT_TURN:
      Logger::info("左转弯：左转");
      motionController.turnLeft(TURN_SPEED);
      break;
      
    case RIGHT_TURN:
      Logger::info("右转弯：右转");
      motionController.turnRight(TURN_SPEED);
      break;
      
    case END_OF_LINE:
      Logger::info("线路终点：停止并掉头");
      motionController.emergencyStop();
      delay(500);
      performUTurn();
      break;
      
    default:
      motionController.moveForward(FOLLOW_SPEED);
      break;
  }
  
  // 记录路口历史
  junctionHistory[junctionHistoryIndex] = junction;
  junctionHistoryIndex = (junctionHistoryIndex + 1) % MAX_JUNCTION_HISTORY;
  junctionCounter++;
}

// 检测路口类型
JunctionType detectJunction() {
  // 使用LineDetector分析传感器数据
  const uint16_t* sensorValues = sensorManager.getInfraredSensorValues();
  return lineDetector.detectJunction(sensorValues);
}

// 处理串口命令
void processCommand(String command) {
  command.trim();
  
  if (command.startsWith("start")) {
    testRunning = true;
    junctionCounter = 0;
    junctionHistoryIndex = 0;
    Logger::info("路口巡线测试启动");
  } 
  else if (command.startsWith("stop")) {
    testRunning = false;
    motionController.emergencyStop();
    Logger::info("路口巡线测试停止");
  }
  else if (command.startsWith("uturn")) {
    performUTurn();
    Logger::info("执行掉头动作");
  }
  else if (command.startsWith("kp")) {
    float value = command.substring(3).toFloat();
    if (value > 0) {
      Kp = value;
      Logger::info("设置 Kp = %.2f", Kp);
    }
  }
  else if (command.startsWith("kd")) {
    float value = command.substring(3).toFloat();
    if (value >= 0) {
      Kd = value;
      Logger::info("设置 Kd = %.2f", Kd);
    }
  }
  else if (command.startsWith("reset")) {
    junctionCounter = 0;
    junctionHistoryIndex = 0;
    lastJunctionType = NO_JUNCTION;
    Logger::info("重置路口计数");
  }
  else if (command.startsWith("history")) {
    Serial.println("\n路口历史记录:");
    for (int i = 0; i < MAX_JUNCTION_HISTORY; i++) {
      int idx = (junctionHistoryIndex - i - 1 + MAX_JUNCTION_HISTORY) % MAX_JUNCTION_HISTORY;
      if (i < junctionCounter) {
        Serial.print(i+1);
        Serial.print(": ");
        Serial.println(junctionTypeToString(junctionHistory[idx]));
      }
    }
  }
  else if (command.startsWith("help")) {
    Serial.println("\n可用命令:");
    Serial.println("start - 开始路口巡线测试");
    Serial.println("stop - 停止路口巡线测试");
    Serial.println("uturn - 手动执行掉头动作");
    Serial.println("kp[value] - 设置Kp值 (例如: kp0.5)");
    Serial.println("kd[value] - 设置Kd值 (例如: kd0.1)");
    Serial.println("reset - 重置路口计数");
    Serial.println("history - 显示路口历史记录");
    Serial.println("help - 显示帮助信息");
  }
}

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  // 设置日志级别
  Logger::setLogLevel(LOG_LEVEL_DEBUG);
  
  // 输出欢迎信息
  Serial.println("\n===== 路口巡线测试程序 =====");
  Serial.println("使用此程序测试小车的路口检测和导航功能");
  Serial.println("输入'help'查看可用命令");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化运动控制器
  motionController.init();
  
  // 初始化传感器
  sensorManager.initAllSensors();
  
  // 初始化历史记录数组
  for (int i = 0; i < MAX_JUNCTION_HISTORY; i++) {
    junctionHistory[i] = NO_JUNCTION;
  }
  
  Logger::info("初始化完成，准备开始测试");
  delay(1000); // 等待系统稳定
}

void loop() {
  // 处理串口命令
  if (Serial.available() > 0) {
    receivedCommand = Serial.readStringUntil('\n');
    newCommandReceived = true;
  }
  
  if (newCommandReceived) {
    processCommand(receivedCommand);
    newCommandReceived = false;
  }
  
  // 更新传感器数据
  sensorManager.update();
  
  // 根据测试状态执行相应操作
  if (testRunning) {
    // 获取线位置
    int position = sensorManager.getLinePosition();
    bool lineDetected = sensorManager.isLineDetected();
    
    // 检查是否检测到线
    if (!lineDetected) {
      // 可能到达线路末端
      motionController.emergencyStop();
      Logger::warning("未检测到线，停止运动");
      
      // 短暂延迟，确认是否确实未检测到线
      delay(200);
      sensorManager.update();
      if (!sensorManager.isLineDetected()) {
        // 仍然未检测到线，可能是要掉头
        Logger::info("确认未检测到线，执行掉头操作");
        performUTurn();
      }
      return;
    }
    
    // 检测路口
    JunctionType junction = detectJunction();
    
    // 路口检测和处理
    unsigned long currentTime = millis();
    if (junction != NO_JUNCTION && junction != lastJunctionType && 
        (currentTime - junctionLastDetectedTime > JUNCTION_DETECTION_COOLDOWN)) {
      
      // 检测到新的路口
      Logger::info("检测到路口: %s", junctionTypeToString(junction));
      
      // 执行路口动作
      executeJunctionAction(junction);
      
      // 更新状态
      lastJunctionType = junction;
      junctionLastDetectedTime = currentTime;
    } 
    else if (junction == NO_JUNCTION) {
      // 正常巡线
      followLine(position);
      lastJunctionType = NO_JUNCTION;
    }
    
    // 每500ms输出一次状态信息
    if (currentTime - lastUpdateTime > 500) {
      lastUpdateTime = currentTime;
      
      // 获取并打印红外传感器原始值
      const uint16_t* irValues = sensorManager.getInfraredSensorValues();
      Serial.print("传感器值: [");
      for (int i = 0; i < 8; i++) {
        Serial.print(irValues[i]);
        if (i < 7) Serial.print(", ");
      }
      Serial.println("]");
      
      Serial.print("线位置: ");
      Serial.print(position);
      Serial.print(", 当前路口: ");
      Serial.println(junctionTypeToString(junction));
      
      Serial.print("路口计数: ");
      Serial.print(junctionCounter);
      Serial.print(", 上一个路口: ");
      Serial.println(junctionTypeToString(lastJunctionType));
    }
  }
  else {
    // 如果测试未运行，确保车辆停止
    motionController.emergencyStop();
    delay(100);
  }
}
#endif 
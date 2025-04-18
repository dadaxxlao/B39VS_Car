#ifdef TEST_LINE_FOLLOWING
#include <Arduino.h>
#include <Wire.h>
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Sensor/SensorCommon.h"
#include "../Control/LineFollower.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 创建运动控制器和传感器管理器实例
MotionController motionController;
SensorManager sensorManager;
LineFollower* lineFollower; // 使用指针，因为需要在setup中初始化

// 测试配置
bool testRunning = true;
unsigned long lastUpdateTime = 0;
bool autoMode = true;

// 串口命令处理
String receivedCommand = "";
bool newCommandReceived = false;

// 显示传感器状态
void displaySensorStatus() {
  // 获取传感器值
  uint16_t values[8];
  if (sensorManager.getInfraredSensorValues(values)) {
    Logger::info("传感器值: [%d, %d, %d, %d, %d, %d, %d, %d]", 
               values[0], values[1], values[2], values[3], 
               values[4], values[5], values[6], values[7]);
  } else {
    Logger::warning("无法获取有效的红外传感器值");
    return;
  }
  
  // 打印线位置
  int position;
  if (sensorManager.getLinePosition(position)) {
    Logger::info("线位置: %d (%s)", position, 
               position < -40 ? "偏左" : (position > 40 ? "偏右" : "居中"));
  } else {
    Logger::warning("无法获取有效的线位置");
  }
  
  // 显示是否检测到线
  bool lineDetected = sensorManager.isLineDetected();
  Logger::info("线检测: %s", lineDetected ? "检测到线" : "未检测到线");
  
  // 如果当前处于丢线状态，显示丢线时间
  unsigned long lineLastDetectedTime = 0; // 这只是为了显示，实际值在LineFollower类内部
  if (lineLastDetectedTime > 0) {
    unsigned long lostTime = millis() - lineLastDetectedTime;
    Logger::info("丢线时间: %lu/%lu ms", lostTime, lineFollower->getMaxLineLostTime());
  }
}

// 处理串口命令
void processCommand(String command) {
  command.trim();
  
  if (command == "start" || command == "s") {
    testRunning = true;
    autoMode = true;
    lineFollower->reset(); // 重置巡线状态
    Serial.println("巡线测试已启动(自动模式)");
  }
  else if (command == "stop" || command == "x") {
    testRunning = false;
    motionController.emergencyStop();
    Serial.println("巡线测试已停止");
  }
  else if (command.startsWith("kp")) {
    float value = command.substring(3).toFloat();
    if (value >= 0) {
      lineFollower->setPIDParams(value, lineFollower->getKi(), lineFollower->getKd());
      Serial.print("设置Kp = ");
      Serial.println(value);
    }
  }
  else if (command.startsWith("ki")) {
    float value = command.substring(3).toFloat();
    if (value >= 0) {
      lineFollower->setPIDParams(lineFollower->getKp(), value, lineFollower->getKd());
      Serial.print("设置Ki = ");
      Serial.println(value);
    }
  }
  else if (command.startsWith("kd")) {
    float value = command.substring(3).toFloat();
    if (value >= 0) {
      lineFollower->setPIDParams(lineFollower->getKp(), lineFollower->getKi(), value);
      Serial.print("设置Kd = ");
      Serial.println(value);
    }
  }
  else if (command.startsWith("lost")) {
    // 设置最长允许丢线时间
    int value = command.substring(5).toInt();
    if (value >= 0) {
      lineFollower->setLineLostParams(value);
      Serial.print("设置最长允许丢线时间 = ");
      Serial.print(value);
      Serial.println("ms");
    }
  }
  else if (command == "status") {
    displaySensorStatus();
    Serial.print("当前PID参数: Kp=");
    Serial.print(lineFollower->getKp());
    Serial.print(" Ki=");
    Serial.print(lineFollower->getKi());
    Serial.print(" Kd=");
    Serial.println(lineFollower->getKd());
    Serial.print("最长允许丢线时间: ");
    Serial.print(lineFollower->getMaxLineLostTime());
    Serial.println("ms");
  }
  else if (command == "manual" || command == "m") {
    autoMode = false;
    testRunning = true;
    Serial.println("巡线测试已启动(手动模式)");
  }
  else if (command == "auto" || command == "a") {
    autoMode = true;
    testRunning = true;
    Serial.println("巡线测试已启动(自动模式)");
  }
  else if (command == "reset") {
    // 重置PID参数到默认值
    lineFollower->setPIDParams(1.0, 0.0, 1.0);
    lineFollower->setLineLostParams(2000);
    lineFollower->setBaseSpeed(FOLLOW_SPEED);
    lineFollower->reset();
    Serial.println("已重置所有参数到默认值");
  }
  else if (command == "help") {
    Serial.println("可用命令:");
    Serial.println("start/s - 开始巡线测试(自动模式)");
    Serial.println("stop/x - 停止巡线测试");
    Serial.println("manual/m - 手动模式");
    Serial.println("auto/a - 自动模式");
    Serial.println("status - 显示传感器状态和PID参数");
    Serial.println("kp[值] - 设置比例系数");
    Serial.println("ki[值] - 设置积分系数");
    Serial.println("kd[值] - 设置微分系数");
    Serial.println("lost[值] - 设置最长允许丢线时间(毫秒)");
    Serial.println("speed[值] - 设置基础速度(0-255)");
    Serial.println("reset - 重置参数到默认值");
    Serial.println("help - 显示帮助信息");
    Serial.println("test - 测试电机基本功能");
    Serial.println("fw - 前进3秒");
    Serial.println("bw - 后退3秒");
    Serial.println("sl - 原地左转3秒");
    Serial.println("sr - 原地右转3秒");
    Serial.println("tl - 左转弯3秒");
    Serial.println("tr - 右转弯3秒");
  }
  else if (command == "test") {
    Serial.println("执行电机测试序列...");
    
    // 测试前进
    Serial.println("测试前进");
    motionController.moveForward(DEFAULT_SPEED);
    delay(1000);
    motionController.emergencyStop();
    delay(500);
    
    // 测试后退
    Serial.println("测试后退");
    motionController.moveBackward(DEFAULT_SPEED);
    delay(1000);
    motionController.emergencyStop();
    delay(500);
    
    // 测试左转
    Serial.println("测试左转");
    motionController.turnLeft(TURN_SPEED);
    delay(1000);
    motionController.emergencyStop();
    delay(500);
    
    // 测试右转
    Serial.println("测试右转");
    motionController.turnRight(TURN_SPEED);
    delay(1000);
    motionController.emergencyStop();
    delay(500);
    
    // 测试原地左转
    Serial.println("测试原地左转");
    motionController.spinLeft(TURN_SPEED);
    delay(1000);
    motionController.emergencyStop();
    delay(500);
    
    // 测试原地右转
    Serial.println("测试原地右转");
    motionController.spinRight(TURN_SPEED);
    delay(1000);
    motionController.emergencyStop();
    
    Serial.println("电机测试完成");
  }
  else if (command == "fw") {
    Serial.println("前进3秒");
    motionController.moveForward(DEFAULT_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command == "bw") {
    Serial.println("后退3秒");
    motionController.moveBackward(DEFAULT_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command == "sl") {
    Serial.println("原地左转3秒");
    motionController.spinLeft(TURN_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command == "sr") {
    Serial.println("原地右转3秒");
    motionController.spinRight(TURN_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command == "tl") {
    Serial.println("左转弯3秒");
    motionController.turnLeft(TURN_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command == "tr") {
    Serial.println("右转弯3秒");
    motionController.turnRight(TURN_SPEED);
    delay(3000);
    motionController.emergencyStop();
    Serial.println("测试完成");
  }
  else if (command.startsWith("speed")) {
    // 设置基础速度
    int value = command.substring(6).toInt();
    if (value > 0 && value <= 255) {
      lineFollower->setBaseSpeed(value);
      Serial.print("设置基础速度 = ");
      Serial.println(value);
    }
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
  
  // 设置日志级别和标签
  Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_DEBUG);
  Logger::setLogTag(COMM_SERIAL, "LineFollow");
  
  Serial.println("\n===== 麦克纳姆轮小车巡线测试程序 =====");
  Serial.println("输入'help'获取命令列表");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化所有传感器
  if (sensorManager.initAllSensors()) {
    Logger::info("所有传感器初始化成功");
  } else {
    Logger::error("部分传感器初始化失败，测试可能不完整");
  }
  
  // 初始化运动控制器
  motionController.init();
  Logger::info("运动控制器初始化成功");
  
  // 创建并初始化巡线控制器
  // 假设LineFollower已被修改为接受SensorManager而不是InfraredArray
  lineFollower = new LineFollower(sensorManager);
  lineFollower->init();
  Logger::info("巡线控制器初始化成功");
  
  // 等待传感器稳定
  delay(1000);
  
  Serial.println("系统就绪，输入'start'开始测试");
}

void loop() {
  // 更新所有传感器数据
  sensorManager.updateAll();
  
  // 处理串口输入
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (receivedCommand.length() > 0) {
        newCommandReceived = true;
      }
    } else {
      receivedCommand += c;
    }
  }
  
  // 处理新命令
  if (newCommandReceived) {
    processCommand(receivedCommand);
    receivedCommand = "";
    newCommandReceived = false;
  }
  
  // 根据测试状态执行操作
  if (testRunning) {
    unsigned long currentTime = millis();
    
    // 自动模式下执行巡线
    if (autoMode) {
      // 使用LineFollower类的update方法
      lineFollower->update();
      
      // 每100毫秒更新一次状态
      if (currentTime - lastUpdateTime > 100) {
        displaySensorStatus();
        lastUpdateTime = currentTime;
      }
    } else {
      // 手动模式下只显示状态，不执行自动巡线
      if (currentTime - lastUpdateTime > 500) {
        displaySensorStatus();
        lastUpdateTime = currentTime;
      }
    }
  }
  
  // 短暂延时，防止过于频繁读取传感器
  delay(20);
}
#endif 
#ifdef TEST_LINE_FOLLOWING
#include <Arduino.h>
#include <Wire.h>
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 创建运动控制器和传感器管理器实例
MotionController motionController;
SensorManager sensorManager;

// PID控制参数
float Kp = 0.5;  // 比例系数
float Kd = 0.1;  // 微分系数
int centerPosition = 3500; // 线位于中央的位置值
int baseSpeed = FOLLOW_SPEED; // 基础速度
int lastError = 0; // 上一次误差

// 测试模式
enum TestMode {
  MODE_PID_TUNING,   // PID参数调整模式
  MODE_FULL_TEST     // 完整巡线测试模式
};

TestMode currentMode = MODE_FULL_TEST;
unsigned long lastUpdateTime = 0;
bool testRunning = false;
bool newCommandReceived = false;
String receivedCommand = "";

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
  // 对于麦克纳姆轮，使用mecanumDrive来实现差速巡线
  float forwardSpeed = 0.8;  // 前进速度分量
  float rotationSpeed = turnAmount / 150.0;  // 旋转分量，归一化到-1.0到1.0
  
  // 限制旋转分量范围
  rotationSpeed = constrain(rotationSpeed, -0.8, 0.8);
  
  // 使用运动控制器的mecanumDrive直接控制
  motionController.mecanumDrive(0, forwardSpeed, rotationSpeed);
  
  // 打印调试信息
  Logger::debug("位置: %d, 误差: %d, 转向: %.2f, 旋转: %.2f", 
                position, error, turnAmount, rotationSpeed);
}

// 处理串口命令
void processCommand(String command) {
  command.trim();
  
  if (command.startsWith("start")) {
    testRunning = true;
    Logger::info("巡线测试启动");
  } 
  else if (command.startsWith("stop")) {
    testRunning = false;
    motionController.emergencyStop();
    Logger::info("巡线测试停止");
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
  else if (command.startsWith("speed")) {
    int value = command.substring(6).toInt();
    if (value > 0 && value <= 255) {
      baseSpeed = value;
      Logger::info("设置速度 = %d", baseSpeed);
    }
  }
  else if (command.startsWith("mode")) {
    int mode = command.substring(5).toInt();
    if (mode == 0) {
      currentMode = MODE_PID_TUNING;
      Logger::info("切换到PID调整模式");
    } else if (mode == 1) {
      currentMode = MODE_FULL_TEST;
      Logger::info("切换到完整巡线测试模式");
    }
  }
  else if (command.startsWith("help")) {
    Serial.println("\n可用命令:");
    Serial.println("start - 开始巡线测试");
    Serial.println("stop - 停止巡线测试");
    Serial.println("kp[value] - 设置Kp值 (例如: kp0.5)");
    Serial.println("kd[value] - 设置Kd值 (例如: kd0.1)");
    Serial.println("speed[value] - 设置速度 (例如: speed180)");
    Serial.println("mode0 - PID调整模式");
    Serial.println("mode1 - 完整巡线测试模式");
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
  Serial.println("\n===== 巡线功能测试程序 =====");
  Serial.println("使用此程序测试和调优小车的巡线功能");
  Serial.println("输入'help'查看可用命令");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化运动控制器
  motionController.init();
  
  // 初始化传感器
  sensorManager.initAllSensors();
  
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
  
  // 根据测试模式执行相应操作
  if (testRunning) {
    // 获取线位置
    int position = sensorManager.getLinePosition();
    bool lineDetected = sensorManager.isLineDetected();
    
    // 检查是否检测到线
    if (!lineDetected) {
      motionController.emergencyStop();
      Logger::warning("未检测到线，停止运动");
      return;
    }
    
    // 执行巡线
    followLine(position);
    
    // 每500ms输出一次状态信息
    unsigned long currentTime = millis();
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
      if (position < -50) {
        Serial.println(" (偏左)");
      } else if (position > 50) {
        Serial.println(" (偏右)");
      } else {
        Serial.println(" (居中)");
      }
      
      Serial.print("PID参数: Kp=");
      Serial.print(Kp);
      Serial.print(", Kd=");
      Serial.println(Kd);
    }
  }
  else {
    // 如果测试未运行，确保车辆停止
    motionController.emergencyStop();
    delay(100);
  }
}
#endif 
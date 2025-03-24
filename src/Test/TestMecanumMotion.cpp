#ifdef TEST_MECANUM_MOTION
#include <Arduino.h>
#include "../Motor/MotionController.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

// 用于编译时选择测试行为的宏定义
// 注意：一次只能启用一个测试宏
// #define TEST_FORWARD_MOTION    // 测试前进
// #define TEST_LEFT_TURN         // 测试左转
// #define TEST_RIGHT_TURN        // 测试右转
// #define TEST_U_TURN            // 测试原地掉头
// #define TEST_LATERAL_LEFT      // 测试左平移
// #define TEST_LATERAL_RIGHT     // 测试右平移
// #define TEST_AUTO_SEQUENCE      // 测试自动序列（默认）

// 转向角度校准测试
 #define TEST_LEFT_TURN_90      // 测试左转90度校准
// #define TEST_RIGHT_TURN_90     // 测试右转90度校准
// #define TEST_U_TURN_180        // 测试原地掉头180度校准

// 创建运动控制器实例
MotionController motionCtrl;

// 定义测试动作
enum TestAction {
  TEST_FORWARD,
  TEST_TURN_LEFT,
  TEST_TURN_RIGHT,
  TEST_U_TURN,
  TEST_LATERAL_LEFT,
  TEST_LATERAL_RIGHT,
  TEST_STOP
};

// 当前测试状态
TestAction currentTest = TEST_STOP;
unsigned long actionStartTime = 0;
int testSpeed = 100; // 测试速度

// 测试时间配置（毫秒）
const unsigned long FORWARD_TIME = 2000;   // 前进时间
const unsigned long TURN_TIME = 950;      // 转弯时间
const unsigned long U_TURN_TIME = 1800;    // 掉头时间
const unsigned long LATERAL_TIME = 2000;   // 平移时间
const unsigned long STOP_TIME = 1000;      // 停止时间
const unsigned long COMPLETE_CYCLE = 15000; // 完成一个周期的时间
const unsigned long STARTUP_DELAY = 3000;  // 启动前的延迟（给予放置小车的时间）

// 90: 100 950ms
// 180: 100 2000ms
// 各种转向角度对应的默认时间参数（可通过测试调整）
unsigned long LEFT_TURN_90_TIME = 950;    // 左转90度所需时间（毫秒）
unsigned long RIGHT_TURN_90_TIME = 950;   // 右转90度所需时间（毫秒）
unsigned long U_TURN_180_TIME = 1900;     // 原地掉头180度所需时间（毫秒）

// LED指示灯引脚（如果有）
const int STATUS_LED_PIN = LED_BUILTIN; // 使用内置LED或其他可用引脚

// EEPROM保存参数位置
const int EEPROM_LEFT_90_ADDR = 0;   // 左转90度时间参数保存地址
const int EEPROM_RIGHT_90_ADDR = 4;  // 右转90度时间参数保存地址
const int EEPROM_U_TURN_ADDR = 8;    // 原地掉头时间参数保存地址

// 执行指定的测试动作
void executeTest(TestAction action) {
  currentTest = action;
  actionStartTime = millis();
  
  switch(action) {
    case TEST_FORWARD:
      // LED快闪1次表示前进测试
      digitalWrite(STATUS_LED_PIN, HIGH);
      delay(200);
      digitalWrite(STATUS_LED_PIN, LOW);
      
      motionCtrl.moveForward(testSpeed);
      break;
    
    case TEST_TURN_LEFT:
      // LED快闪2次表示左转测试
      for (int i = 0; i < 2; i++) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(200);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
      }
      
      motionCtrl.turnLeft(testSpeed);
      break;
    
    case TEST_TURN_RIGHT:
      // LED快闪3次表示右转测试
      for (int i = 0; i < 3; i++) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(200);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
      }
      
      motionCtrl.turnRight(testSpeed);
      break;
    
    case TEST_U_TURN:
      // LED快闪4次表示原地掉头测试
      for (int i = 0; i < 4; i++) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(200);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
      }
      
      motionCtrl.spinLeft(testSpeed);
      break;
    
    case TEST_LATERAL_LEFT:
      // LED快闪5次表示左平移测试
      for (int i = 0; i < 5; i++) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(200);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
      }
      
      motionCtrl.lateralLeft(testSpeed);
      break;
    
    case TEST_LATERAL_RIGHT:
      // LED快闪6次表示右平移测试
      for (int i = 0; i < 6; i++) {
        digitalWrite(STATUS_LED_PIN, HIGH);
        delay(200);
        digitalWrite(STATUS_LED_PIN, LOW);
        delay(200);
      }
      
      motionCtrl.lateralRight(testSpeed);
      break;
    
    case TEST_STOP:
      // LED常亮表示停止状态
      digitalWrite(STATUS_LED_PIN, HIGH);
      
      motionCtrl.emergencyStop();
      break;
  }
}

// 单一行为测试流程
void runSingleTest(TestAction action) {
  static enum {
    INIT,          // 初始化阶段
    EXECUTE_TEST,  // 执行测试
    FINISH_TEST    // 完成测试
  } testPhase = INIT;
  
  static unsigned long testStartTime = 0;
  
  switch(testPhase) {
    case INIT:
      // 初始化阶段
      digitalWrite(STATUS_LED_PIN, HIGH);  // LED亮起表示准备中
      testStartTime = millis();
      testPhase = EXECUTE_TEST;
      break;
      
    case EXECUTE_TEST:
      // 如果已经过了初始延迟时间，开始执行测试
      if (millis() - testStartTime > STARTUP_DELAY) {
        executeTest(action);
        testPhase = FINISH_TEST;
      }
      break;
      
    case FINISH_TEST:
      // 检查测试是否已完成
      unsigned long duration = 0;
      
      // 根据不同测试类型设置持续时间
      switch(action) {
        case TEST_FORWARD:
          duration = FORWARD_TIME;
          break;
        
        case TEST_TURN_LEFT:
          duration = TURN_TIME;
          break;
        
        case TEST_TURN_RIGHT:
          duration = TURN_TIME;
          break;
        
        case TEST_U_TURN:
          duration = U_TURN_TIME;
          break;
        
        case TEST_LATERAL_LEFT:
        case TEST_LATERAL_RIGHT:
          duration = LATERAL_TIME;
          break;
        
        default:
          duration = 0;
          break;
      }
      
      // 如果测试持续时间已到，停止并进入完成状态
      if (duration > 0 && millis() - actionStartTime > duration) {
        executeTest(TEST_STOP);
        
        // 闪烁LED指示测试完成
        for (int i = 0; i < 3; i++) {
          digitalWrite(STATUS_LED_PIN, HIGH);
          delay(500);
          digitalWrite(STATUS_LED_PIN, LOW);
          delay(500);
        }
        
        // 维持停止状态（程序结束前不再执行其他动作）
        while(true) {
          // 确保小车保持停止状态
          motionCtrl.emergencyStop();
          delay(1000);
        }
      }
      break;
  }
}

// 自动测试序列
void runAutoTestSequence() {
  static enum {
    INIT,
    RUNNING,
    COMPLETED
  } sequenceState = INIT;
  
  static unsigned long sequenceStartTime = 0;
  
  switch(sequenceState) {
    case INIT:
      // 初始化测试序列
      digitalWrite(STATUS_LED_PIN, HIGH);
      delay(500);
      digitalWrite(STATUS_LED_PIN, LOW);
      delay(500);
      
      sequenceStartTime = millis();
      sequenceState = RUNNING;
      break;
      
    case RUNNING:
      // 计算序列已运行时间
      unsigned long elapsedTime = millis() - sequenceStartTime;
      
      // 根据时间决定执行哪个测试
      if (elapsedTime < STARTUP_DELAY) {
        // 启动延迟，等待小车放置好
        digitalWrite(STATUS_LED_PIN, (millis() % 500) < 250 ? HIGH : LOW); // 慢闪
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME) {
        if (currentTest != TEST_FORWARD) {
          executeTest(TEST_FORWARD);
        }
      } 
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME) {
        if (currentTest != TEST_STOP) {
          executeTest(TEST_STOP);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME + TURN_TIME) {
        if (currentTest != TEST_TURN_LEFT) {
          executeTest(TEST_TURN_LEFT);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME*2 + TURN_TIME) {
        if (currentTest != TEST_STOP) {
          executeTest(TEST_STOP);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME*2 + TURN_TIME*2) {
        if (currentTest != TEST_TURN_RIGHT) {
          executeTest(TEST_TURN_RIGHT);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME*3 + TURN_TIME*2) {
        if (currentTest != TEST_STOP) {
          executeTest(TEST_STOP);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + FORWARD_TIME + STOP_TIME*3 + TURN_TIME*2 + U_TURN_TIME) {
        if (currentTest != TEST_U_TURN) {
          executeTest(TEST_U_TURN);
        }
      }
      else if (elapsedTime < STARTUP_DELAY + COMPLETE_CYCLE) {
        if (currentTest != TEST_STOP) {
          executeTest(TEST_STOP);
        }
      }
      else {
        // 完成测试序列
        sequenceState = COMPLETED;
      }
      break;
      
    case COMPLETED:
      // 测试完成，闪烁LED指示
      digitalWrite(STATUS_LED_PIN, (millis() % 1000) < 500 ? HIGH : LOW); // 慢闪
      
      // 确保小车保持停止状态
      motionCtrl.emergencyStop();
      break;
  }
}

// 角度校准测试 - 左转90度
void runLeftTurn90Test() {
  static enum {
    INIT,
    WAIT_START,
    TURNING,
    COMPLETED
  } calibState = INIT;
  
  static unsigned long testStartTime = 0;
  
  switch(calibState) {
    case INIT:
      // 初始化校准测试
      Serial.println("左转90度校准测试");
      Serial.println("当前设定时间: " + String(LEFT_TURN_90_TIME) + " ms");
      Serial.println("放置小车，并按下回车键开始测试...");
      
      // 进入等待状态
      calibState = WAIT_START;
      break;
      
    case WAIT_START:
      // 等待用户通过串口触发测试开始
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\n' || cmd == '\r') {
          Serial.println("开始测试左转90度...");
          
          // 短暂延迟，给用户时间放开按键
          delay(500);
          
          // 开始转向
          motionCtrl.spinLeft(testSpeed);
          testStartTime = millis();
          calibState = TURNING;
        }
      }
      break;
      
    case TURNING:
      // 转向过程中
      // 检查是否已经转过了当前设定的时间
      if (millis() - testStartTime >= LEFT_TURN_90_TIME) {
        // 停止转向
        motionCtrl.emergencyStop();
        // motionCtrl.moveForward(testSpeed);
        // delay(500);
        // motionCtrl.emergencyStop();
        
        Serial.println("转向完成，请检查是否正好转过90度？");
        Serial.println("输入 '+' 增加时间，输入 '-' 减少时间，输入 's' 保存当前值，输入 'r' 重新测试");
        
        calibState = COMPLETED;
      }
      break;
      
    case COMPLETED:
      // 测试完成，等待用户调整参数
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch(cmd) {
          case '+':
            // 增加时间
            LEFT_TURN_90_TIME += 50;
            Serial.println("调整为: " + String(LEFT_TURN_90_TIME) + " ms");
            break;
            
          case '-':
            // 减少时间
            LEFT_TURN_90_TIME = max(50UL, LEFT_TURN_90_TIME - 50);
            Serial.println("调整为: " + String(LEFT_TURN_90_TIME) + " ms");
            break;
            
          case 's':
            // 保存当前值
            // TODO: 如果需要持久化存储，可以使用EEPROM保存
            Serial.println("左转90度时间参数已保存: " + String(LEFT_TURN_90_TIME) + " ms");
            Serial.println("建议将此值更新到代码中的LEFT_TURN_90_TIME常量中");
            break;
            
          case 'r':
            // 重新测试
            Serial.println("重新开始测试...");
            calibState = WAIT_START;
            break;
        }
      }
      break;
  }
}

// 角度校准测试 - 右转90度
void runRightTurn90Test() {
  static enum {
    INIT,
    WAIT_START,
    TURNING,
    COMPLETED
  } calibState = INIT;
  
  static unsigned long testStartTime = 0;
  
  switch(calibState) {
    case INIT:
      // 初始化校准测试
      Serial.println("右转90度校准测试");
      Serial.println("当前设定时间: " + String(RIGHT_TURN_90_TIME) + " ms");
      Serial.println("放置小车，并按下回车键开始测试...");
      
      // 进入等待状态
      calibState = WAIT_START;
      break;
      
    case WAIT_START:
      // 等待用户通过串口触发测试开始
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\n' || cmd == '\r') {
          Serial.println("开始测试右转90度...");
          
          // 短暂延迟，给用户时间放开按键
          delay(500);
          
          // 开始转向
          motionCtrl.spinRight(testSpeed);
          testStartTime = millis();
          calibState = TURNING;
        }
      }
      break;
      
    case TURNING:
      // 转向过程中
      // 检查是否已经转过了当前设定的时间
      if (millis() - testStartTime >= RIGHT_TURN_90_TIME) {
        // 停止转向
        motionCtrl.emergencyStop();

        // motionCtrl.moveForward(testSpeed);
        // delay(200);
        // motionCtrl.emergencyStop();
        
        Serial.println("转向完成，请检查是否正好转过90度？");
        Serial.println("输入 '+' 增加时间，输入 '-' 减少时间，输入 's' 保存当前值，输入 'r' 重新测试");
        
        calibState = COMPLETED;
      }
      break;
      
    case COMPLETED:
      // 测试完成，等待用户调整参数
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch(cmd) {
          case '+':
            // 增加时间
            RIGHT_TURN_90_TIME += 50;
            Serial.println("调整为: " + String(RIGHT_TURN_90_TIME) + " ms");
            break;
            
          case '-':
            // 减少时间
            RIGHT_TURN_90_TIME = max(50UL, RIGHT_TURN_90_TIME - 50);
            Serial.println("调整为: " + String(RIGHT_TURN_90_TIME) + " ms");
            break;
            
          case 's':
            // 保存当前值
            // TODO: 如果需要持久化存储，可以使用EEPROM保存
            Serial.println("右转90度时间参数已保存: " + String(RIGHT_TURN_90_TIME) + " ms");
            Serial.println("建议将此值更新到代码中的RIGHT_TURN_90_TIME常量中");
            break;
            
          case 'r':
            // 重新测试
            Serial.println("重新开始测试...");
            calibState = WAIT_START;
            break;
        }
      }
      break;
  }
}

// 角度校准测试 - 原地掉头180度
void runUTurn180Test() {
  static enum {
    INIT,
    WAIT_START,
    TURNING,
    COMPLETED
  } calibState = INIT;
  
  static unsigned long testStartTime = 0;
  
  switch(calibState) {
    case INIT:
      // 初始化校准测试
      Serial.println("原地掉头180度校准测试");
      Serial.println("当前设定时间: " + String(U_TURN_180_TIME) + " ms");
      Serial.println("放置小车，并按下回车键开始测试...");
      
      // 进入等待状态
      calibState = WAIT_START;
      break;
      
    case WAIT_START:
      // 等待用户通过串口触发测试开始
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\n' || cmd == '\r') {
          Serial.println("开始测试原地掉头180度...");
          
          // 短暂延迟，给用户时间放开按键
          delay(500);
          
          // 开始转向
          motionCtrl.spinLeft(testSpeed);
          testStartTime = millis();
          calibState = TURNING;
        }
      }
      break;
      
    case TURNING:
      // 转向过程中
      // 检查是否已经转过了当前设定的时间
      if (millis() - testStartTime >= U_TURN_180_TIME) {
        // 停止转向
        motionCtrl.emergencyStop();
        
        Serial.println("转向完成，请检查是否正好转过180度？");
        Serial.println("输入 '+' 增加时间，输入 '-' 减少时间，输入 's' 保存当前值，输入 'r' 重新测试");
        
        calibState = COMPLETED;
      }
      break;
      
    case COMPLETED:
      // 测试完成，等待用户调整参数
      if (Serial.available() > 0) {
        char cmd = Serial.read();
        
        switch(cmd) {
          case '+':
            // 增加时间
            U_TURN_180_TIME += 50;
            Serial.println("调整为: " + String(U_TURN_180_TIME) + " ms");
            break;
            
          case '-':
            // 减少时间
            U_TURN_180_TIME = max(50UL, U_TURN_180_TIME - 50);
            Serial.println("调整为: " + String(U_TURN_180_TIME) + " ms");
            break;
            
          case 's':
            // 保存当前值
            // TODO: 如果需要持久化存储，可以使用EEPROM保存
            Serial.println("原地掉头180度时间参数已保存: " + String(U_TURN_180_TIME) + " ms");
            Serial.println("建议将此值更新到代码中的U_TURN_180_TIME常量中");
            break;
            
          case 'r':
            // 重新测试
            Serial.println("重新开始测试...");
            calibState = WAIT_START;
            break;
        }
      }
      break;
  }
}

void setup() {
  // 初始化状态LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);

  // 初始化串口（用于参数调整反馈）
  Serial.begin(9600);
  
  // 初始化运动控制器
  motionCtrl.init();
  
  // 初始设置为停止状态
  motionCtrl.emergencyStop();
  
  // 启动时LED闪烁表示程序已启动
  for (int i = 0; i < 2; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(500);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(500);
  }

  // 如果是参数校准测试，显示使用说明
  #if defined(TEST_LEFT_TURN_90) || defined(TEST_RIGHT_TURN_90) || defined(TEST_U_TURN_180)
    Serial.println("=== 麦克纳姆轮转向参数校准程序 ===");
    Serial.println("测试将通过串口交互调整转向时间参数");
    Serial.println("按照提示完成测试即可找到最佳参数");
  #endif
}

void loop() {
  // 根据编译时选择的测试选项决定执行的测试
  #if defined(TEST_FORWARD_MOTION)
    runSingleTest(TEST_FORWARD);
  #elif defined(TEST_LEFT_TURN)
    runSingleTest(TEST_TURN_LEFT);
  #elif defined(TEST_RIGHT_TURN)
    runSingleTest(TEST_TURN_RIGHT);
  #elif defined(TEST_U_TURN)
    runSingleTest(TEST_U_TURN);
  #elif defined(TEST_LATERAL_LEFT)
    runSingleTest(TEST_LATERAL_LEFT);
  #elif defined(TEST_LATERAL_RIGHT)
    runSingleTest(TEST_LATERAL_RIGHT);
  #elif defined(TEST_LEFT_TURN_90)
    runLeftTurn90Test();
  #elif defined(TEST_RIGHT_TURN_90)
    runRightTurn90Test();
  #elif defined(TEST_U_TURN_180)
    runUTurn180Test();
  #elif defined(TEST_AUTO_SEQUENCE)
    runAutoTestSequence();
  #else
    // 默认行为：自动测试序列
    runAutoTestSequence();
  #endif
}
#endif 
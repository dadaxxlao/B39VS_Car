#ifdef TEST_ACCURATE_TURN
/**
 * AccurateTurn模块测试文件
 * 
 * 目的：验证AccurateTurn的功能，包括：
 * - 启动精确左转、右转、U型转弯。
 * - 验证在检测到黑线时停止 (AT_COMPLETED)。
 * - 验证在超时后停止 (AT_TIMED_OUT)。
 * - 观察状态机转换 (AT_IDLE -> AT_INITIAL_SPIN -> AT_TURNING -> AT_COMPLETED/AT_TIMED_OUT)。
 * - 验证初始旋转延迟是否能防止在黑线上启动时立即停止。
 * 
 * 设置：
 * 1. 将此文件放置在'src/Test/'目录。
 * 2. 确保platformio.ini中的build_flags设置为编译此测试文件而非main.cpp。
 *    例如: build_flags = -D TEST_ACCURATE_TURN
 *    (注释或移除其他如-D TEST_NAVIGATION_CONTROLLER的标志)
 * 
 * 硬件要求：
 * - Arduino Mega 或类似控制器
 * - 连接好并能正常工作的SensorManager (特别是I2C红外线阵列)。
 * - 连接好并能正常工作的MotionController (电机和驱动器)。
 * 
 * 操作步骤：
 * 1. 上传代码到Arduino。
 * 2. 打开串口监视器 (波特率 115200)。
 * 3. 按照提示按下回车键开始初始化。
 * 4. 将小车放置在测试场地上（可以在线上或线外开始）。
 * 5. 通过串口输入命令:
 *    - 'l' 或 'L': 启动精确左转。
 *    - 'r' 或 'R': 启动精确右转。
 *    - 'u' 或 'U': 启动U型转弯 (向左转)。
 *    - 'q' 或 'Q': 终止测试。
 * 6. 观察串口日志输出，检查状态转换和最终结果 (完成或超时)。
 * 7. **重要:** 当前版本的AccurateTurn在完成一次转弯(成功或超时)后会停留在最终状态。
 *    要进行下一次转弯测试，需要**重置Arduino开发板**。
 */

#include <Arduino.h>
#include "../Sensor/SensorManager.h"    
#include "../Motor/MotionController.h"   
#include "../Control/AccurateTurn.h" 
#include "../Utils/Logger.h"           
#include "../Utils/Config.h"           // For TURN_SPEED potentially

// --- 全局对象 ---
SensorManager sensorManager;
MotionController motionController;
AccurateTurn accurateTurn(motionController, sensorManager);

// --- 测试配置 ---
const int LOOP_DELAY_MS = 50; // 主循环延迟（毫秒）

// --- 函数声明 ---
const char* stateToString(AccurateTurnState state);

// --- 初始化函数 ---
void setup() {
  // 初始化串口通信用于日志输出
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }

  // 初始化Logger系统
  Logger::init();
  Logger::setGlobalLogLevel(LOG_LEVEL_DEBUG); // Use DEBUG to see detailed logs
  Logger::info("AccTurnTest", "--- 精确转弯模块测试就绪 ---");
  
  Serial.println("\n请将小车放置好，然后按下回车键开始初始化...");
  Serial.println("控制命令:");
  Serial.println("  l - 精确左转");
  Serial.println("  r - 精确右转");
  Serial.println("  u - U型转弯 (向左)");
  Serial.println("  q - 终止测试");
  Serial.println("注意: 每次转弯完成后需重置开发板进行下次测试。");
  
  // 清空串口缓冲区
  while(Serial.available()) {
    Serial.read();
  }
  
  // 等待用户按下回车键
  while(true) {
    if(Serial.available()) {
      char c = Serial.read();
      if(c == '\n' || c == '\r') {
        break; 
      }
    }
    delay(10); 
  }
  
  Serial.println("开始初始化...");
  Logger::info("AccTurnTest", "--- 测试初始化中 ---");

  // 初始化传感器管理器
  Logger::info("AccTurnTest", "初始化 SensorManager...");
  if (sensorManager.initAllSensors()) {
    Logger::info("AccTurnTest", "SensorManager 初始化成功.");
  } else {
    Logger::error("AccTurnTest", "错误: SensorManager 初始化失败! 停止执行.");
    while (true) {
        motionController.emergencyStop(); 
        delay(1000);
    }
  }

  // 初始化运动控制器
  Logger::info("AccTurnTest", "初始化 MotionController...");
  motionController.init(); 
  Logger::info("AccTurnTest", "MotionController 初始化完成.");

  // 初始化精确转弯模块
  Logger::info("AccTurnTest", "初始化 AccurateTurn...");
  accurateTurn.init(); 
  Logger::info("AccTurnTest", "AccurateTurn 初始化完成.");

  delay(500);
  Logger::info("AccTurnTest", "初始化完成. 等待命令...");
  Serial.println("\n初始化完成。请输入命令 (l, r, u, q):");
  Logger::info("AccTurnTest", "-----------------------------------------");
}

// --- 主循环函数 ---
void loop() {
  static AccurateTurnState lastState = AT_IDLE; // Track state changes

  // 1. 处理串口输入命令
  if (Serial.available()) {
    char cmd = Serial.read();
    cmd = tolower(cmd); // Convert to lowercase

    if (cmd == 'q') {
      motionController.emergencyStop();
      Serial.println("\n收到终止命令 'q'. 测试停止.");
      Logger::info("AccTurnTest", "用户手动终止测试.");
      while (true) { // Halt execution
        motionController.emergencyStop();
        delay(500);
      }
    }

    // Only accept new turn commands if the module is idle
    if (accurateTurn.getCurrentState() == AT_IDLE) {
      switch (cmd) {
        case 'l':
          Serial.println("指令: 启动左转...");
          Logger::info("AccTurnTest", "用户指令: startTurnLeft()");
          accurateTurn.startTurnLeft(); 
          break;
        case 'r':
          Serial.println("指令: 启动右转...");
          Logger::info("AccTurnTest", "用户指令: startTurnRight()");
          accurateTurn.startTurnRight();
          break;
        case 'u':
          Serial.println("指令: 启动U型转弯 (左转)...");
          Logger::info("AccTurnTest", "用户指令: startUTurn()");
          accurateTurn.startUTurn();
          break;
        default:
          // Ignore other characters if idle
          break; 
      }
    } else {
        if (cmd == 'l' || cmd == 'r' || cmd == 'u') {
            Serial.println("指令忽略: 精确转弯模块当前正忙.");
            Logger::warning("AccTurnTest", "用户指令 (%c) 被忽略，模块非空闲 (State: %s)", cmd, stateToString(accurateTurn.getCurrentState()));
        }
    }
     // Clear any other characters received in this loop iteration
    while(Serial.available()) { Serial.read(); }
  }

  // 2. 更新传感器数据 (假定需要)
  sensorManager.updateAll(); // Or specific sensor update if applicable

  // 3. 更新精确转弯模块的状态机
  accurateTurn.update();

  // 4. 记录状态变化 (可选)
  AccurateTurnState currentState = accurateTurn.getCurrentState();
  if (currentState != lastState) {
    Logger::info("AccTurnTest", "状态转换: %s -> %s", stateToString(lastState), stateToString(currentState));
    if (currentState == AT_COMPLETED || currentState == AT_TIMED_OUT) {
        Serial.print("转弯结束. 最终状态: ");
        Serial.println(stateToString(currentState));
        Serial.println("请重置开发板进行下一次测试。");
    }
    lastState = currentState;
  }

  // 5. 循环延迟
  delay(LOOP_DELAY_MS); 
}

// 将枚举AccurateTurnState转换为可读字符串
const char* stateToString(AccurateTurnState state) {
  switch(state) {
    case AT_IDLE:               return "AT_IDLE";
    case AT_TURNING_LEFT:       return "AT_TURNING_LEFT";
    case AT_TURNING_RIGHT:      return "AT_TURNING_RIGHT";
    case AT_TURNING_UTURN:      return "AT_TURNING_UTURN";
    case AT_COMPLETED:          return "AT_COMPLETED";
    case AT_TIMED_OUT:          return "AT_TIMED_OUT";
    default:                    return "未知状态";
  }
}

#endif // TEST_ACCURATE_TURN 
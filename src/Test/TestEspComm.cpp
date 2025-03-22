#ifdef TEST_ESP_COMM
#include <Arduino.h>
#include "../Utils/EspBridge.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

// 模拟传感器数据
uint16_t dummySensorValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int linePosition = 3500;

// 上次数据发送时间
unsigned long lastDataSendTime = 0;
const unsigned long DATA_SEND_INTERVAL = 1000; // 每秒发送一次数据

// 当前系统状态
SystemStateCode systemState = STATE_IDLE;

void setup() {
  // 初始化Arduino串口
  Serial.begin(115200);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  // 初始化日志系统
  Logger::init();
  Logger::setLogLevel(LOG_LEVEL_DEBUG);
  
  // 初始化ESP通信
  if (EspComm.begin(ESP_RX_PIN, ESP_TX_PIN, ESP_BAUD_RATE)) {
    Logger::info("ESP32通信初始化成功");
  } else {
    Logger::error("ESP32通信初始化失败，请检查连接");
  }
  
  Logger::info("ESP32通信测试程序开始运行");
}

void loop() {
  // 处理ESP32发来的命令
  EspComm.update();
  
  // 周期性发送状态和数据
  unsigned long currentTime = millis();
  if (currentTime - lastDataSendTime >= DATA_SEND_INTERVAL) {
    lastDataSendTime = currentTime;
    
    // 发送系统状态
    EspComm.sendState(systemState, "Arduino状态正常");
    
    // 更新模拟传感器数据
    for (int i = 0; i < 8; i++) {
      dummySensorValues[i] = random(0, 1000);
    }
    linePosition = random(0, 7000);
    
    // 构造传感器数据字符串
    char sensorData[128];
    sprintf(sensorData, "line_pos=%d,sensors=[%d,%d,%d,%d,%d,%d,%d,%d]",
            linePosition,
            dummySensorValues[0], dummySensorValues[1],
            dummySensorValues[2], dummySensorValues[3],
            dummySensorValues[4], dummySensorValues[5],
            dummySensorValues[6], dummySensorValues[7]);
    
    // 发送传感器数据
    EspComm.sendData(DATA_SENSORS, sensorData);
    
    // 输出日志
    Logger::debug("已发送传感器数据: %s", sensorData);
  }
  
  // 处理串口命令（用于测试，可以模拟发送命令给ESP32）
  if (Serial.available() > 0) {
    String inputCmd = Serial.readStringUntil('\n');
    
    // 如果输入以$CMD:开头，模拟ESP32发送命令
    if (inputCmd.startsWith("$CMD:")) {
      Logger::info("模拟ESP32命令: %s", inputCmd.c_str());
      // 添加消息结束符
      inputCmd += "#";
      
      // 将命令放入ESP通信缓冲区
      for (unsigned int i = 0; i < inputCmd.length(); i++) {
        EspComm.processReceivedData();
      }
    } 
    // 否则当作状态更改命令
    else if (inputCmd.startsWith("state:")) {
      int newState = inputCmd.substring(6).toInt();
      if (newState >= 0 && newState <= STATE_RETURN_BASE) {
        systemState = (SystemStateCode)newState;
        Logger::info("系统状态已更改为: %d", systemState);
      } else {
        Logger::warning("无效的状态值: %d", newState);
      }
    }
    // 发送日志测试
    else if (inputCmd.startsWith("log:")) {
      String logMsg = inputCmd.substring(4);
      Logger::info("%s", logMsg.c_str());
    }
  }
  
  // 简单延时
  delay(10);
}
#endif 
#ifdef TEST_BLUETOOTH
#include <Arduino.h>
#include "../Utils/BluetoothSerial.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

String receivedCommand = "";
bool newCommandReceived = false;
unsigned long lastDataSendTime = 0;
const unsigned long DATA_SEND_INTERVAL = 1000; // 每秒发送一次数据

// 模拟传感器数据
uint16_t dummySensorValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int linePosition = 3500;
bool isLineDetected = true;

// 处理命令
void processCommand(const char* command) {
  String cmd = String(command);
  cmd.trim();
  
  // 检查是否是带前缀的命令
  if (cmd.startsWith("$CMD:")) {
    cmd = cmd.substring(5); // 去除前缀
  }
  
  Logger::info("收到指令: %s", cmd.c_str());
  
  if (cmd.startsWith("led")) {
    // 模拟LED控制命令
    int ledValue = cmd.substring(3).toInt();
    Logger::info("设置LED亮度: %d", ledValue);
    BtSerial.sendResponse("led", true);
  }
  else if (cmd.startsWith("speed")) {
    // 模拟速度设置命令
    int speedValue = cmd.substring(5).toInt();
    Logger::info("设置速度: %d", speedValue);
    BtSerial.sendResponse("speed", true);
  }
  else if (cmd.startsWith("debug")) {
    // 设置日志级别
    int debugLevel = cmd.substring(5).toInt();
    Logger::setLogLevel(debugLevel);
    Logger::info("设置日志级别: %d", debugLevel);
    BtSerial.sendResponse("debug", true);
  }
  else if (cmd == "start") {
    Logger::info("开始测试");
    BtSerial.sendResponse("start", true);
  }
  else if (cmd == "stop") {
    Logger::info("停止测试");
    BtSerial.sendResponse("stop", true);
  }
  else if (cmd == "scan") {
    // 模拟传感器扫描
    Logger::info("扫描中...");
    
    // 生成随机传感器数据
    for (int i = 0; i < 8; i++) {
      dummySensorValues[i] = random(0, 1000);
    }
    
    // 模拟线位置变化
    linePosition = random(0, 7000);
    isLineDetected = (linePosition > 1000 && linePosition < 6000);
    
    BtSerial.sendSensorData(dummySensorValues, linePosition);
    BtSerial.sendResponse("scan", true);
  }
  else if (cmd == "help") {
    String helpMsg = "可用命令:\n";
    helpMsg += "led[0-255] - 设置LED亮度\n";
    helpMsg += "speed[0-255] - 设置速度\n";
    helpMsg += "debug[1-4] - 设置日志级别\n";
    helpMsg += "start - 开始测试\n";
    helpMsg += "stop - 停止测试\n";
    helpMsg += "scan - 模拟传感器扫描\n";
    helpMsg += "help - 显示帮助";
    
    // 分行发送帮助信息
    int start = 0;
    int end = helpMsg.indexOf('\n');
    while (end >= 0) {
      String line = helpMsg.substring(start, end);
      BtSerial.println(line.c_str());
      start = end + 1;
      end = helpMsg.indexOf('\n', start);
    }
    
    // 发送最后一行
    if (start < helpMsg.length()) {
      BtSerial.println(helpMsg.substring(start).c_str());
    }
    
    BtSerial.sendResponse("help", true);
  }
  else {
    Logger::warning("未知命令: %s", cmd.c_str());
    BtSerial.sendResponse(cmd.c_str(), false);
  }
}

void setup() {
  // 初始化串口
  Serial.begin(115200);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  // 初始化日志系统
  Logger::init();
  Logger::setLogLevel(LOG_LEVEL_DEBUG);
  
  // 初始化蓝牙
  if (BtSerial.begin(BT_RX_PIN, BT_TX_PIN, BT_BAUD_RATE)) {
    Logger::info("蓝牙HM-10模块初始化成功");
    
    // 设置Logger使用蓝牙输出
    Logger::setBtStream(&BtSerial);
    Logger::enableBluetooth(true);
  } else {
    Logger::error("蓝牙HM-10模块初始化失败，请检查连接");
  }
  
  Logger::info("蓝牙测试程序开始运行");
  BtSerial.println("HM-10蓝牙模块测试");
  BtSerial.println("输入'help'查看可用命令");
  
  // 随机数种子
  randomSeed(analogRead(0));
}

void loop() {
  // 处理蓝牙接收的数据
  if (BtSerial.processReceivedData()) {
    const char* command = BtSerial.getLastCommand();
    processCommand(command);
  }
  
  // 周期性发送测试数据
  unsigned long currentTime = millis();
  if (currentTime - lastDataSendTime >= DATA_SEND_INTERVAL) {
    lastDataSendTime = currentTime;
    
    // 稍微改变一下传感器值，模拟数值变化
    for (int i = 0; i < 8; i++) {
      dummySensorValues[i] = constrain(dummySensorValues[i] + random(-100, 100), 0, 1000);
    }
    
    // 位置随机变动
    linePosition = constrain(linePosition + random(-300, 300), 0, 7000);
    
    // 发送传感器数据
    BtSerial.sendSensorData(dummySensorValues, linePosition);
    
    // 发送一些调试信息
    Logger::debug("线位置: %d, 检测状态: %s", 
                 linePosition, 
                 isLineDetected ? "检测到线" : "未检测到线");
  }
  
  // 处理硬件串口命令 (可同时使用硬件串口和蓝牙控制)
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    processCommand(cmd.c_str());
  }
  
  // 简单延时
  delay(10);
}
#endif 
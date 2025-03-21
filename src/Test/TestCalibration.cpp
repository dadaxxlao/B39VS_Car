#ifdef TEST_CALIBRATION
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"

InfraredArray infraredSensor;

// 校准状态
enum CalibrationState {
  WAIT_FOR_STABILIZATION,
  CALIBRATION_INSTRUCTIONS,
  MONITORING,
  COMPLETE
};

CalibrationState state = WAIT_FOR_STABILIZATION;
unsigned long startTime = 0;
const int STABILIZATION_TIME = 20000; // 20秒稳定时间

// I2C命令常量
const uint8_t CALIBRATION_REGISTER = 0x01;
const uint8_t CALIBRATION_ENTER = 1;
const uint8_t CALIBRATION_EXIT = 0;
const uint8_t SENSOR_STATUS_REGISTER = 0x30;

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("红外传感器校准程序 (I2C模式)");
  Serial.println("\n红外传感器初始化和校准流程:");
  Serial.println("1. 首先进行传感器稳定 (20秒等待)");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化传感器
  if (infraredSensor.begin(INFRARED_ARRAY_ADDR)) {
    Serial.println("红外传感器初始化成功");
    Serial.println("开始等待传感器稳定...");
    startTime = millis();
    state = WAIT_FOR_STABILIZATION;
  } else {
    Serial.println("红外传感器初始化失败，请检查连接");
    while (1); // 停止执行
  }
}

// 向模块发送I2C校准命令
void sendCalibrationCommand(bool enterCalibration) {
  Wire.beginTransmission(INFRARED_ARRAY_ADDR);
  Wire.write(CALIBRATION_REGISTER);
  Wire.write(enterCalibration ? CALIBRATION_ENTER : CALIBRATION_EXIT);
  Wire.endTransmission();
  
  if (enterCalibration) {
    Serial.println("已发送进入校准模式命令");
  } else {
    Serial.println("已发送退出校准模式命令");
  }
}

// 读取传感器状态值（8位二进制，每位代表一个传感器）
uint8_t readSensorStatus() {
  Wire.beginTransmission(INFRARED_ARRAY_ADDR);
  Wire.write(SENSOR_STATUS_REGISTER);
  Wire.endTransmission();
  
  Wire.requestFrom(INFRARED_ARRAY_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

// 显示传感器状态的二进制和十进制表示
void displaySensorStatus(uint8_t status) {
  Serial.print("传感器状态: 0b");
  for (int i = 7; i >= 0; i--) {
    Serial.print((status >> i) & 0x01);
  }
  Serial.print(" (0x");
  Serial.print(status, HEX);
  Serial.print(", ");
  Serial.print(status, DEC);
  Serial.println(")");
  
  // 显示每个传感器的状态
  Serial.print("传感器: ");
  for (int i = 0; i < 8; i++) {
    bool sensorOn = !((status >> (7-i)) & 0x01); // 注意：0表示检测到黑线
    Serial.print("X");
    Serial.print(i+1);
    Serial.print(":");
    Serial.print(sensorOn ? "■" : "□");
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  // 处理串口输入
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    switch (state) {
      case CALIBRATION_INSTRUCTIONS:
        if (input == 'c' || input == 'C') {
          // 发送进入校准命令
          sendCalibrationCommand(true);
          Serial.println("\n模块现在应处于校准模式。");
          Serial.println("请按照以下步骤操作:");
          Serial.println("1. 确保红灯常亮 (表示进入校准模式)");
          Serial.println("2. 将8个探头对准黑线 (确保都能检测到黑线)");
          Serial.println("3. 等待校准完成 (红灯熄灭)");
          Serial.println("4. 按 'E' 退出校准模式");
        } else if (input == 'e' || input == 'E') {
          // 发送退出校准命令
          sendCalibrationCommand(false);
          state = MONITORING;
          Serial.println("\n已退出校准模式，进入监控模式");
          Serial.println("系统将持续显示传感器状态");
          Serial.println("按 'R' 重新开始");
        } else if (input == 'm' || input == 'M') {
          state = MONITORING;
          Serial.println("\n进入监控模式");
          Serial.println("系统将持续显示传感器状态");
          Serial.println("按 'R' 重新开始");
        }
        break;
        
      case MONITORING:
        if (input == 'r' || input == 'R') {
          Serial.println("\n重新开始校准流程");
          Serial.println("等待传感器稳定 (20秒)...");
          startTime = millis();
          state = WAIT_FOR_STABILIZATION;
        }
        break;
        
      default:
        if (input == 's' || input == 'S') {
          // 跳过等待
          state = CALIBRATION_INSTRUCTIONS;
          Serial.println("\n跳过等待时间");
          showCalibrationInstructions();
        }
        break;
    }
  }
  
  // 状态处理
  switch (state) {
    case WAIT_FOR_STABILIZATION:
      {
        unsigned long currentTime = millis();
        if (currentTime - startTime >= STABILIZATION_TIME) {
          state = CALIBRATION_INSTRUCTIONS;
          showCalibrationInstructions();
        } else {
          // 显示倒计时
          static unsigned long lastDisplayTime = 0;
          if (currentTime - lastDisplayTime >= 1000) { // 每秒更新一次
            lastDisplayTime = currentTime;
            int secondsLeft = (STABILIZATION_TIME - (currentTime - startTime)) / 1000;
            Serial.print("等待传感器稳定: 剩余 ");
            Serial.print(secondsLeft);
            Serial.println(" 秒");
          }
        }
      }
      break;
      
    case MONITORING:
      {
        // 实时显示传感器状态
        static unsigned long lastUpdateTime = 0;
        unsigned long currentTime = millis();
        if (currentTime - lastUpdateTime >= 500) { // 每500ms更新一次
          lastUpdateTime = currentTime;
          
          // 更新传感器数据
          infraredSensor.update();
          const uint16_t* values = infraredSensor.getAllSensorValues();
          
          // 打印当前值为一行
          Serial.print("传感器值: [");
          for (int i = 0; i < 8; i++) {
            Serial.print(values[i]);
            if (i < 7) Serial.print(", ");
          }
          Serial.println("]");
          
          // 显示原始I2C状态值
          uint8_t sensorStatus = readSensorStatus();
          displaySensorStatus(sensorStatus);
          
          Serial.println("------------------------------");
        }
      }
      break;
  }
}

void showCalibrationInstructions() {
  Serial.println("\n传感器已稳定，可以开始校准");
  Serial.println("按手册说明，有两种校准方式:");
  Serial.println("1. 硬件校准: 长按模块上的key1按键，等待红灯常亮，进入校准模式");
  Serial.println("2. 软件校准: 按 'C' 发送I2C校准命令 (地址0x12，寄存器0x01，值1)");
  Serial.println("\n完成校准后，按 'E' 退出校准模式");
  Serial.println("或按 'M' 直接进入监控模式，查看传感器状态");
}
#endif
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"

InfraredArray infraredSensor;

// 存储上一个传感器状态
bool lastSensorState[8] = {false, false, false, false, false, false, false, false};
// 路口计数
int junctionCount = 0;
// 上次路口检测时间
unsigned long lastJunctionTime = 0;
// 路口检测去抖动延迟(ms)
const unsigned long JUNCTION_DEBOUNCE_DELAY = 1000;

// 判断传感器是否指示白色(未检测到线)
bool isWhite(uint16_t value) {
  return value < LINE_THRESHOLD;
}

// 判断传感器是否指示黑色(检测到线)
bool isBlack(uint16_t value) {
  return value >= LINE_THRESHOLD;
}

// 检测路口类型
JunctionType detectJunction(const uint16_t* sensorValues) {
  // 将传感器值转换为二进制状态 (true = 黑线, false = 白色)
  bool sensorState[8];
  for (int i = 0; i < 8; i++) {
    sensorState[i] = isBlack(sensorValues[i]);
  }
  
  // 打印传感器状态
  Serial.print("传感器状态: [");
  for (int i = 0; i < 8; i++) {
    Serial.print(sensorState[i] ? "■" : "□");
    if (i < 7) Serial.print(" ");
  }
  Serial.println("]");
  
  // 测试不同路口模式
  
  // 左转路口: 左侧传感器全部检测到线
  if (sensorState[0] && sensorState[1] && sensorState[2] && !sensorState[7]) {
    return LEFT_TURN;
  }
  
  // 右转路口: 右侧传感器全部检测到线
  if (!sensorState[0] && sensorState[5] && sensorState[6] && sensorState[7]) {
    return RIGHT_TURN;
  }
  
  // T形路口(左): 左侧和中间检测到线
  if (sensorState[0] && sensorState[1] && sensorState[2] && 
      sensorState[3] && sensorState[4] && !sensorState[7]) {
    return T_LEFT;
  }
  
  // T形路口(右): 右侧和中间检测到线
  if (!sensorState[0] && sensorState[3] && sensorState[4] && 
      sensorState[5] && sensorState[6] && sensorState[7]) {
    return T_RIGHT;
  }
  
  // 倒T形路口: 中间和两侧部分检测到线
  if (!sensorState[0] && !sensorState[7] && 
      sensorState[2] && sensorState[3] && sensorState[4] && sensorState[5]) {
    return T_FORWARD;
  }
  
  // 十字路口: 几乎所有传感器都检测到线
  if (sensorState[1] && sensorState[2] && sensorState[3] && 
      sensorState[4] && sensorState[5] && sensorState[6]) {
    return CROSS;
  }
  
  // 线路终点: 所有传感器都没有检测到线
  bool allWhite = true;
  for (int i = 0; i < 8; i++) {
    if (sensorState[i]) {
      allWhite = false;
      break;
    }
  }
  if (allWhite) {
    return END_OF_LINE;
  }
  
  // 不是特殊路口
  return NO_JUNCTION;
}

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("路口检测测试程序");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化红外传感器
  if (infraredSensor.begin(INFRARED_ARRAY_ADDR)) {
    Serial.println("红外传感器初始化成功");
  } else {
    Serial.println("红外传感器初始化失败，请检查连接");
    while (1); // 停止执行
  }
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 更新传感器数据
  infraredSensor.update();
  
  // 获取传感器值
  const uint16_t* sensorValues = infraredSensor.getAllSensorValues();
  
  // 检测路口类型
  JunctionType junction = detectJunction(sensorValues);
  
  // 打印传感器原始值
  Serial.print("传感器值: [");
  for (int i = 0; i < 8; i++) {
    Serial.print(sensorValues[i]);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("]");
  
  // 打印线位置
  int position = infraredSensor.getLinePosition();
  Serial.print("线位置: ");
  Serial.println(position);
  
  // 处理路口检测
  unsigned long currentTime = millis();
  if (junction != NO_JUNCTION && currentTime - lastJunctionTime > JUNCTION_DEBOUNCE_DELAY) {
    lastJunctionTime = currentTime;
    junctionCount++;
    
    // 打印路口信息
    Serial.print("检测到路口: ");
    switch (junction) {
      case T_LEFT:
        Serial.println("T型路口(左)");
        break;
      case T_RIGHT:
        Serial.println("T型路口(右)");
        break;
      case T_FORWARD:
        Serial.println("倒T型路口");
        break;
      case CROSS:
        Serial.println("十字路口");
        break;
      case LEFT_TURN:
        Serial.println("左转");
        break;
      case RIGHT_TURN:
        Serial.println("右转");
        break;
      case END_OF_LINE:
        Serial.println("线路终点");
        break;
      default:
        Serial.println("未知类型");
    }
    
    Serial.print("路口计数: ");
    Serial.println(junctionCount);
  }
  
  delay(100); // 每100ms更新一次
} 
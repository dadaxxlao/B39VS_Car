#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/SensorManager.h"
#include "../Utils/Config.h"

SensorManager sensorManager;

// 将颜色代码转换为字符串
const char* colorToString(ColorCode color) {
  switch (color) {
    case COLOR_RED:
      return "红色";
    case COLOR_BLUE:
      return "蓝色";
    case COLOR_YELLOW:
      return "黄色";
    case COLOR_WHITE:
      return "白色";
    case COLOR_BLACK:
      return "黑色";
    default:
      return "未知";
  }
}

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("传感器管理器综合测试程序");
  
  // 初始化所有传感器
  sensorManager.initAllSensors();
  
  Serial.println("所有传感器初始化完成");
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 更新所有传感器数据
  sensorManager.update();
  
  // 测试超声波传感器
  Serial.println("======= 超声波传感器 =======");
  float distance = sensorManager.getUltrasonicDistance();
  Serial.print("距离: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // 测试红外传感器
  Serial.println("======= 红外传感器 =======");
  int linePos = sensorManager.getLinePosition();
  bool lineDetected = sensorManager.isLineDetected();
  
  Serial.print("线位置: ");
  Serial.print(linePos);
  Serial.print(" (");
  if (linePos < -50) {
    Serial.println("偏左)");
  } else if (linePos > 50) {
    Serial.println("偏右)");
  } else {
    Serial.println("居中)");
  }
  
  Serial.print("线检测: ");
  Serial.println(lineDetected ? "检测到线" : "未检测到线");
  
  // 获取并打印红外传感器原始值
  const uint16_t* irValues = sensorManager.getInfraredSensorValues();
  Serial.print("传感器值: [");
  for (int i = 0; i < 8; i++) {
    Serial.print(irValues[i]);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("]");
  
  // 测试颜色传感器
  Serial.println("======= 颜色传感器 =======");
  ColorCode color = sensorManager.getColor();
  Serial.print("检测到颜色: ");
  Serial.println(colorToString(color));
  
  // 获取并打印RGB原始值
  uint16_t r, g, b, c;
  sensorManager.getColorSensorValues(&r, &g, &b, &c);
  
  Serial.print("R: ");
  Serial.print(r);
  Serial.print(", G: ");
  Serial.print(g);
  Serial.print(", B: ");
  Serial.print(b);
  Serial.print(", C: ");
  Serial.println(c);
  
  // 打印详细调试信息
  sensorManager.debugColorSensor();
  
  // 分隔线
  Serial.println("------------------------------");
  
  delay(1500); // 每1.5秒更新一次
} 
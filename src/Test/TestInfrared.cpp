#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"

InfraredArray infraredSensor;

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("红外线阵列传感器测试程序");
  
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
  
  // 获取线位置
  int position = infraredSensor.getLinePosition();
  
  // 检测是否有线
  bool lineDetected = infraredSensor.isLineDetected();
  
  // 打印传感器原始值
  const uint16_t* values = infraredSensor.getAllSensorValues();
  Serial.print("传感器值: [");
  for (int i = 0; i < 8; i++) {
    Serial.print(values[i]);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("]");
  
  // 打印巡线信息
  Serial.print("线位置: ");
  Serial.print(position);
  Serial.print(" (");
  if (position < -50) {
    Serial.println("偏左)");
  } else if (position > 50) {
    Serial.println("偏右)");
  } else {
    Serial.println("居中)");
  }
  
  Serial.print("线检测: ");
  Serial.println(lineDetected ? "检测到线" : "未检测到线");
  
  delay(500); // 每500ms更新一次
} 
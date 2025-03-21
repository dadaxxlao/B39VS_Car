#ifdef TEST_ULTRASONIC
#include <Arduino.h>
#include "../Sensor/Ultrasonic.h"
#include "../Utils/Config.h"

UltrasonicSensor ultrasonicSensor;

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("超声波传感器测试程序");
  
  // 初始化超声波传感器
  ultrasonicSensor.init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 获取距离
  float distance = ultrasonicSensor.getDistance();
  
  // 打印距离信息
  Serial.print("距离: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // 检测是否有障碍物
  bool obstacle = ultrasonicSensor.isObstacleDetected(NO_OBJECT_THRESHOLD);
  Serial.print("障碍物检测: ");
  Serial.println(obstacle ? "有障碍物" : "无障碍物");
  
  // 检测是否达到抓取距离
  bool canGrab = ultrasonicSensor.isObstacleDetected(GRAB_DISTANCE);
  Serial.print("抓取距离: ");
  Serial.println(canGrab ? "可以抓取" : "距离太远");
  
  delay(500); // 每500ms更新一次
} 
#endif
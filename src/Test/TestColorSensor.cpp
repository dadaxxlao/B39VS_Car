#ifdef TEST_COLOR_SENSOR
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/ColorSensor.h"
#include "../Utils/Config.h"

ColorSensor colorSensor;

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
  
  Serial.println("颜色传感器测试程序");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化颜色传感器
  if (colorSensor.begin(COLOR_SENSOR_ADDR)) {
    Serial.println("颜色传感器初始化成功");
  } else {
    Serial.println("颜色传感器初始化失败，请检查连接");
    while (1); // 停止执行
  }
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 更新传感器数据
  colorSensor.update();
  
  // 获取当前颜色
  ColorCode color = colorSensor.readColor();
  
  // 打印颜色信息
  Serial.print("检测到颜色: ");
  Serial.println(colorToString(color));
  
  // 获取并打印RGB原始值
  uint16_t r, g, b, c;
  colorSensor.getRGB(&r, &g, &b, &c);
  
  Serial.println("原始RGB值:");
  Serial.print("R: ");
  Serial.print(r);
  Serial.print(", G: ");
  Serial.print(g);
  Serial.print(", B: ");
  Serial.print(b);
  Serial.print(", C: ");
  Serial.println(c);
  
  // 计算并打印归一化RGB值
  float normR, normG, normB;
  colorSensor.calculateNormalizedRGB(r, g, b, c, &normR, &normG, &normB);
  
  Serial.println("归一化RGB值 (0-255):");
  Serial.print("R: ");
  Serial.print(normR);
  Serial.print(", G: ");
  Serial.print(normG);
  Serial.print(", B: ");
  Serial.println(normB);
  
  // 打印详细调试信息
  colorSensor.debugPrint();
  
  // 分隔线
  Serial.println("------------------------------");
  
  delay(1000); // 每1秒更新一次
} 
#endif
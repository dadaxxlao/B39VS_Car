#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/ColorSensor.h"
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"

ColorSensor colorSensor;
InfraredArray infraredSensor;

// 校准状态
enum CalibrationState {
  SELECT_SENSOR,
  COLOR_SENSOR,
  IR_SENSOR,
  COMPLETE
};

CalibrationState state = SELECT_SENSOR;
ColorCode currentColorToCalibrate = COLOR_RED;
bool samplingColor = false;
unsigned long lastSampleTime = 0;
int sampleCount = 0;
const int SAMPLES_NEEDED = 10;

// 颜色求平均值的累计
uint32_t sumR = 0, sumG = 0, sumB = 0, sumC = 0;

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("传感器校准程序");
  Serial.println("请选择要校准的传感器:");
  Serial.println("1 - 颜色传感器");
  Serial.println("2 - 红外线传感器");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化传感器
  colorSensor.begin(COLOR_SENSOR_ADDR);
  infraredSensor.begin(INFRARED_ARRAY_ADDR);
  
  delay(1000); // 等待传感器稳定
}

void loop() {
  // 处理串口输入
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    switch (state) {
      case SELECT_SENSOR:
        if (input == '1') {
          state = COLOR_SENSOR;
          Serial.println("\n开始颜色传感器校准");
          Serial.println("请将传感器放置在红色区域上，然后按 'S' 开始采样");
          currentColorToCalibrate = COLOR_RED;
        } else if (input == '2') {
          state = IR_SENSOR;
          Serial.println("\n开始红外传感器校准");
          Serial.println("请将传感器放置在黑线上，然后按 'S' 开始采样");
        }
        break;
        
      case COLOR_SENSOR:
        if (input == 's' || input == 'S') {
          // 开始采样当前颜色
          samplingColor = true;
          sumR = sumG = sumB = sumC = 0;
          sampleCount = 0;
          Serial.print("开始采样");
          Serial.print(getColorName(currentColorToCalibrate));
          Serial.println("，请保持传感器位置稳定...");
        }
        break;
        
      case IR_SENSOR:
        if (input == 's' || input == 'S') {
          // 采样红外传感器数据
          infraredSensor.update();
          const uint16_t* values = infraredSensor.getAllSensorValues();
          
          Serial.println("红外传感器值:");
          for (int i = 0; i < 8; i++) {
            Serial.print("传感器 ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(values[i]);
          }
          
          Serial.println("\n请将传感器放置在白色区域上，然后按 'S' 采样");
        } else if (input == 'c' || input == 'C') {
          state = COMPLETE;
          Serial.println("红外传感器校准完成");
        }
        break;
        
      case COMPLETE:
        if (input == 'r' || input == 'R') {
          // 重置校准
          state = SELECT_SENSOR;
          Serial.println("\n请选择要校准的传感器:");
          Serial.println("1 - 颜色传感器");
          Serial.println("2 - 红外线传感器");
        }
        break;
    }
  }
  
  // 处理颜色采样
  if (state == COLOR_SENSOR && samplingColor) {
    unsigned long currentTime = millis();
    if (currentTime - lastSampleTime > 200) { // 每200ms采样一次
      lastSampleTime = currentTime;
      
      // 更新传感器数据
      colorSensor.update();
      
      // 获取RGB值
      uint16_t r, g, b, c;
      colorSensor.getRGB(&r, &g, &b, &c);
      
      // 累加值
      sumR += r;
      sumG += g;
      sumB += b;
      sumC += c;
      
      Serial.print(".");
      sampleCount++;
      
      if (sampleCount >= SAMPLES_NEEDED) {
        // 计算平均值
        uint16_t avgR = sumR / SAMPLES_NEEDED;
        uint16_t avgG = sumG / SAMPLES_NEEDED;
        uint16_t avgB = sumB / SAMPLES_NEEDED;
        uint16_t avgC = sumC / SAMPLES_NEEDED;
        
        Serial.println("\n采样完成!");
        Serial.print(getColorName(currentColorToCalibrate));
        Serial.println("的平均值:");
        Serial.print("R: ");
        Serial.print(avgR);
        Serial.print(", G: ");
        Serial.print(avgG);
        Serial.print(", B: ");
        Serial.print(avgB);
        Serial.print(", C: ");
        Serial.println(avgC);
        
        // 切换到下一个颜色
        samplingColor = false;
        
        if (currentColorToCalibrate < COLOR_COUNT - 1) {
          currentColorToCalibrate = static_cast<ColorCode>(currentColorToCalibrate + 1);
          Serial.print("\n请将传感器放置在");
          Serial.print(getColorName(currentColorToCalibrate));
          Serial.println("区域上，然后按 'S' 开始采样");
        } else {
          Serial.println("\n所有颜色校准完成!");
          Serial.println("按 'R' 重新开始校准");
          state = COMPLETE;
        }
      }
    }
  }
}

// 获取颜色名称
const char* getColorName(ColorCode color) {
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
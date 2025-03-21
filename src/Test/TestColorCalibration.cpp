#ifdef TEST_COLOR_CALIBRATION
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/ColorSensor.h"
#include "../Utils/Config.h"

ColorSensor colorSensor;

// 校准步骤
enum CalibrationStep {
  SELECT_MODE,
  CALIBRATE_RED,
  CALIBRATE_BLUE,
  CALIBRATE_YELLOW,
  CALIBRATE_WHITE,
  CALIBRATE_BLACK,
  TEST_RECOGNITION,
  COMPLETE
};

CalibrationStep currentStep = SELECT_MODE;

void setup() {
  // 初始化串口
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  Serial.println("颜色传感器校准和测试程序");
  Serial.println("请选择模式:");
  Serial.println("1 - 校准模式");
  Serial.println("2 - 测试模式");
  
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

const char* getColorName(ColorCode color) {
  switch (color) {
    case COLOR_RED:    return "红色";
    case COLOR_BLUE:   return "蓝色";
    case COLOR_YELLOW: return "黄色";
    case COLOR_WHITE:  return "白色";
    case COLOR_BLACK:  return "黑色";
    default:           return "未知";
  }
}

void loop() {
  // 处理串口输入
  if (Serial.available() > 0) {
    char input = Serial.read();
    
    switch (currentStep) {
      case SELECT_MODE:
        if (input == '1') {
          currentStep = CALIBRATE_RED;
          Serial.println("\n开始颜色校准流程");
          Serial.println("请将传感器放在红色表面上，然后按 'S' 开始采样");
        } else if (input == '2') {
          currentStep = TEST_RECOGNITION;
          Serial.println("\n进入颜色识别测试模式");
          Serial.println("系统将持续识别颜色，按 'Q' 退出");
        }
        break;
        
      case CALIBRATE_RED:
        if (input == 's' || input == 'S') {
          Serial.println("开始校准红色...");
          colorSensor.calibrateColor(COLOR_RED);
          currentStep = CALIBRATE_BLUE;
          Serial.println("\n请将传感器放在蓝色表面上，然后按 'S' 开始采样");
        }
        break;
        
      case CALIBRATE_BLUE:
        if (input == 's' || input == 'S') {
          Serial.println("开始校准蓝色...");
          colorSensor.calibrateColor(COLOR_BLUE);
          currentStep = CALIBRATE_YELLOW;
          Serial.println("\n请将传感器放在黄色表面上，然后按 'S' 开始采样");
        }
        break;
        
      case CALIBRATE_YELLOW:
        if (input == 's' || input == 'S') {
          Serial.println("开始校准黄色...");
          colorSensor.calibrateColor(COLOR_YELLOW);
          currentStep = CALIBRATE_WHITE;
          Serial.println("\n请将传感器放在白色表面上，然后按 'S' 开始采样");
        }
        break;
        
      case CALIBRATE_WHITE:
        if (input == 's' || input == 'S') {
          Serial.println("开始校准白色...");
          colorSensor.calibrateColor(COLOR_WHITE);
          currentStep = CALIBRATE_BLACK;
          Serial.println("\n请将传感器放在黑色表面上，然后按 'S' 开始采样");
        }
        break;
        
      case CALIBRATE_BLACK:
        if (input == 's' || input == 'S') {
          Serial.println("开始校准黑色...");
          colorSensor.calibrateColor(COLOR_BLACK);
          currentStep = COMPLETE;
          Serial.println("\n颜色校准完成!");
          Serial.println("请将以上显示的阈值设置复制到ColorSensor.cpp的initColorThresholds()函数中");
          Serial.println("按 'T' 进入测试模式，或 'R' 重新开始校准");
        }
        break;
        
      case TEST_RECOGNITION:
        if (input == 'q' || input == 'Q') {
          currentStep = COMPLETE;
          Serial.println("\n测试结束");
          Serial.println("按 'R' 重新开始校准或测试");
        }
        break;
        
      case COMPLETE:
        if (input == 'r' || input == 'R') {
          currentStep = SELECT_MODE;
          Serial.println("\n请选择模式:");
          Serial.println("1 - 校准模式");
          Serial.println("2 - 测试模式");
        } else if (input == 't' || input == 'T') {
          currentStep = TEST_RECOGNITION;
          Serial.println("\n进入颜色识别测试模式");
          Serial.println("系统将持续识别颜色，按 'Q' 退出");
        }
        break;
    }
  }
  
  // 测试识别模式
  if (currentStep == TEST_RECOGNITION) {
    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastUpdateTime > 500) { // 每500ms更新一次
      lastUpdateTime = currentTime;
      
      // 获取识别到的颜色
      ColorCode detectedColor = colorSensor.readColor();
      
      // 打印原始值、比例和识别结果
      colorSensor.printRawValues();
      Serial.print("识别结果: ");
      Serial.println(getColorName(detectedColor));
      Serial.println("------------------------------");
    }
  }
}
#endif 
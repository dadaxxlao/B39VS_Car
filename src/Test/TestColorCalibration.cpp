#ifdef TEST_COLOR_CALIBRATION
#include <Arduino.h>
#include <Wire.h>
#include "../Sensor/ColorSensor.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 全局对象
ColorSensor colorSensor;

// 校准参数
const int SAMPLES_PER_COLOR = 10;  // 每种颜色采样次数
const int SAMPLE_DELAY_MS = 200;   // 每次采样间隔
unsigned long lastSampleTime = 0;  // 上次采样时间

// 校准状态
enum CalibrationState {
  STATE_IDLE,            // 空闲状态
  STATE_AWAIT_RED,       // 等待红色样本
  STATE_SAMPLING_RED,    // 采样红色
  STATE_AWAIT_BLUE,      // 等待蓝色样本
  STATE_SAMPLING_BLUE,   // 采样蓝色
  STATE_AWAIT_YELLOW,    // 等待黄色样本
  STATE_SAMPLING_YELLOW, // 采样黄色
  STATE_AWAIT_BLACK,     // 等待黑色样本
  STATE_SAMPLING_BLACK,  // 采样黑色
  STATE_AWAIT_WHITE,     // 等待白色样本
  STATE_SAMPLING_WHITE,  // 采样白色
  STATE_COMPLETE         // 校准完成
};

CalibrationState currentState = STATE_IDLE;
int sampleCount = 0;

// 采样数据结构
struct SampleData {
  uint8_t minH, maxH;
  uint8_t minS, maxS;
  uint8_t minL, maxL;
  uint8_t samples;
};

// 各颜色的采样数据
SampleData redSample = {255, 0, 255, 0, 255, 0, 0};
SampleData blueSample = {255, 0, 255, 0, 255, 0, 0};
SampleData yellowSample = {255, 0, 255, 0, 255, 0, 0};
SampleData blackSample = {255, 0, 255, 0, 255, 0, 0};
SampleData whiteSample = {255, 0, 255, 0, 255, 0, 0};

// 更新样本数据
void updateSampleData(SampleData& data, uint8_t h, uint8_t s, uint8_t l) {
  // 更新H分量（注意：红色可能跨越0度）
  if (currentState == STATE_SAMPLING_RED) {
    // 红色特殊处理：检查是否跨越0度
    if (h > 180) { // 假设红色在240-20度之间
      data.minH = min(data.minH, h);
      data.maxH = max(data.maxH, (uint8_t)20); // 假设跨越到20度
    } else if (h < 60) {
      data.minH = min(data.minH, (uint8_t)220); // 假设从220度开始
      data.maxH = max(data.maxH, h);
    } else {
      // 如果样本不在预期范围内，仍然记录
      data.minH = min(data.minH, h);
      data.maxH = max(data.maxH, h);
    }
  } else {
    // 其他颜色正常处理
    data.minH = min(data.minH, h);
    data.maxH = max(data.maxH, h);
  }
  
  // 更新S分量
  data.minS = min(data.minS, s);
  data.maxS = max(data.maxS, s);
  
  // 更新L分量
  data.minL = min(data.minL, l);
  data.maxL = max(data.maxL, l);
  
  // 增加样本计数
  data.samples++;
}

// 将当前状态转换为字符串
const char* stateToString(CalibrationState state) {
  switch (state) {
    case STATE_IDLE:            return "空闲";
    case STATE_AWAIT_RED:       return "等待红色";
    case STATE_SAMPLING_RED:    return "采样红色";
    case STATE_AWAIT_BLUE:      return "等待蓝色";
    case STATE_SAMPLING_BLUE:   return "采样蓝色";
    case STATE_AWAIT_YELLOW:    return "等待黄色";
    case STATE_SAMPLING_YELLOW: return "采样黄色";
    case STATE_AWAIT_BLACK:     return "等待黑色";
    case STATE_SAMPLING_BLACK:  return "采样黑色";
    case STATE_AWAIT_WHITE:     return "等待白色";
    case STATE_SAMPLING_WHITE:  return "采样白色";
    case STATE_COMPLETE:        return "校准完成";
    default:                    return "未知状态";
  }
}

// 打印菜单
void printMenu() {
  Logger::info("======= 颜色传感器校准程序 =======");
  Logger::info("s - 开始校准流程");
  Logger::info("r - 重置所有校准数据");
  Logger::info("p - 打印当前校准结果");
  Logger::info("c - 继续到下一步");
  Logger::info("i - 显示传感器状态");
  Logger::info("? - 显示此菜单");
  Logger::info("===================================");
}

// 打印当前校准结果
void printCalibrationResults() {
  Logger::info("====== 颜色校准结果 ======");
  
  if (redSample.samples > 0) {
    Logger::info("红色 (样本数: %d):", redSample.samples);
    Logger::info("  H: %d - %d", redSample.minH, redSample.maxH);
    Logger::info("  S: %d - %d", redSample.minS, redSample.maxS);
    Logger::info("  L: %d - %d", redSample.minL, redSample.maxL);
  }
  
  if (blueSample.samples > 0) {
    Logger::info("蓝色 (样本数: %d):", blueSample.samples);
    Logger::info("  H: %d - %d", blueSample.minH, blueSample.maxH);
    Logger::info("  S: %d - %d", blueSample.minS, blueSample.maxS);
    Logger::info("  L: %d - %d", blueSample.minL, blueSample.maxL);
  }
  
  if (yellowSample.samples > 0) {
    Logger::info("黄色 (样本数: %d):", yellowSample.samples);
    Logger::info("  H: %d - %d", yellowSample.minH, yellowSample.maxH);
    Logger::info("  S: %d - %d", yellowSample.minS, yellowSample.maxS);
    Logger::info("  L: %d - %d", yellowSample.minL, yellowSample.maxL);
  }
  
  if (blackSample.samples > 0) {
    Logger::info("黑色 (样本数: %d):", blackSample.samples);
    Logger::info("  H: %d - %d", blackSample.minH, blackSample.maxH);
    Logger::info("  S: %d - %d", blackSample.minS, blackSample.maxS);
    Logger::info("  L: %d - %d", blackSample.minL, blackSample.maxL);
  }
  
  if (whiteSample.samples > 0) {
    Logger::info("白色 (样本数: %d):", whiteSample.samples);
    Logger::info("  H: %d - %d", whiteSample.minH, whiteSample.maxH);
    Logger::info("  S: %d - %d", whiteSample.minS, whiteSample.maxS);
    Logger::info("  L: %d - %d", whiteSample.minL, whiteSample.maxL);
  }
  
  Logger::info("==========================");
  
  if (currentState == STATE_COMPLETE) {
    Logger::info("阈值设置建议 (可复制到ColorSensor.cpp中):");
    Logger::info("// 红色阈值 (H接近0或接近240)");
    Logger::info("colorThresholds[COLOR_RED].minH = %d;", redSample.minH);
    Logger::info("colorThresholds[COLOR_RED].maxH = %d; // 跨0判断", redSample.maxH);
    Logger::info("colorThresholds[COLOR_RED].minS = %d;", redSample.minS);
    Logger::info("colorThresholds[COLOR_RED].maxS = %d;", redSample.maxS);
    Logger::info("colorThresholds[COLOR_RED].minL = %d;", redSample.minL);
    Logger::info("colorThresholds[COLOR_RED].maxL = %d;", redSample.maxL);
    
    Logger::info("// 蓝色阈值");
    Logger::info("colorThresholds[COLOR_BLUE].minH = %d;", blueSample.minH);
    Logger::info("colorThresholds[COLOR_BLUE].maxH = %d;", blueSample.maxH);
    Logger::info("colorThresholds[COLOR_BLUE].minS = %d;", blueSample.minS);
    Logger::info("colorThresholds[COLOR_BLUE].maxS = %d;", blueSample.maxS);
    Logger::info("colorThresholds[COLOR_BLUE].minL = %d;", blueSample.minL);
    Logger::info("colorThresholds[COLOR_BLUE].maxL = %d;", blueSample.maxL);
    
    Logger::info("// 黄色阈值");
    Logger::info("colorThresholds[COLOR_YELLOW].minH = %d;", yellowSample.minH);
    Logger::info("colorThresholds[COLOR_YELLOW].maxH = %d;", yellowSample.maxH);
    Logger::info("colorThresholds[COLOR_YELLOW].minS = %d;", yellowSample.minS);
    Logger::info("colorThresholds[COLOR_YELLOW].maxS = %d;", yellowSample.maxS);
    Logger::info("colorThresholds[COLOR_YELLOW].minL = %d;", yellowSample.minL);
    Logger::info("colorThresholds[COLOR_YELLOW].maxL = %d;", yellowSample.maxL);
    
    Logger::info("// 黑色阈值");
    Logger::info("colorThresholds[COLOR_BLACK].minH = %d;", blackSample.minH);
    Logger::info("colorThresholds[COLOR_BLACK].maxH = %d;", blackSample.maxH);
    Logger::info("colorThresholds[COLOR_BLACK].minS = %d;", blackSample.minS);
    Logger::info("colorThresholds[COLOR_BLACK].maxS = %d;", blackSample.maxS);
    Logger::info("colorThresholds[COLOR_BLACK].minL = %d;", blackSample.minL);
    Logger::info("colorThresholds[COLOR_BLACK].maxL = %d;", blackSample.maxL);
    
    Logger::info("// 白色阈值");
    Logger::info("colorThresholds[COLOR_WHITE].minH = %d;", whiteSample.minH);
    Logger::info("colorThresholds[COLOR_WHITE].maxH = %d;", whiteSample.maxH);
    Logger::info("colorThresholds[COLOR_WHITE].minS = %d;", whiteSample.minS);
    Logger::info("colorThresholds[COLOR_WHITE].maxS = %d;", whiteSample.maxS);
    Logger::info("colorThresholds[COLOR_WHITE].minL = %d;", whiteSample.minL);
    Logger::info("colorThresholds[COLOR_WHITE].maxL = %d;", whiteSample.maxL);
  }
}

// 开始校准流程
void startCalibration() {
  // 重置所有样本数据
  redSample = {255, 0, 255, 0, 255, 0, 0};
  blueSample = {255, 0, 255, 0, 255, 0, 0};
  yellowSample = {255, 0, 255, 0, 255, 0, 0};
  blackSample = {255, 0, 255, 0, 255, 0, 0};
  whiteSample = {255, 0, 255, 0, 255, 0, 0};
  
  // 设置初始状态
  currentState = STATE_AWAIT_RED;
  sampleCount = 0;
  
  Logger::info("校准流程开始");
  Logger::info("请将红色物体放置在传感器前，然后按'c'继续");
}

// 重置校准数据
void resetCalibration() {
  redSample = {255, 0, 255, 0, 255, 0, 0};
  blueSample = {255, 0, 255, 0, 255, 0, 0};
  yellowSample = {255, 0, 255, 0, 255, 0, 0};
  blackSample = {255, 0, 255, 0, 255, 0, 0};
  whiteSample = {255, 0, 255, 0, 255, 0, 0};
  
  currentState = STATE_IDLE;
  sampleCount = 0;
  
  Logger::info("校准数据已重置");
}

// 采样指定颜色
void sampleColor() {
  uint8_t h, s, l;
  
  // 读取HSL值
  if (colorSensor.getColorHSL(h, s, l)) {
    SampleData* targetSample = nullptr;
    
    // 根据当前状态选择目标样本
    switch (currentState) {
      case STATE_SAMPLING_RED:
        targetSample = &redSample;
        break;
      case STATE_SAMPLING_BLUE:
        targetSample = &blueSample;
        break;
      case STATE_SAMPLING_YELLOW:
        targetSample = &yellowSample;
        break;
      case STATE_SAMPLING_BLACK:
        targetSample = &blackSample;
        break;
      case STATE_SAMPLING_WHITE:
        targetSample = &whiteSample;
        break;
      default:
        return; // 不在采样状态
    }
    
    // 更新样本数据
    updateSampleData(*targetSample, h, s, l);
    
    // 显示当前值
    Logger::info("样本 %d/%d - H:%d, S:%d, L:%d", 
                sampleCount + 1, SAMPLES_PER_COLOR, h, s, l);
    
    // 增加样本计数
    sampleCount++;
    
    // 检查是否采样完成
    if (sampleCount >= SAMPLES_PER_COLOR) {
      switch (currentState) {
        case STATE_SAMPLING_RED:
          currentState = STATE_AWAIT_BLUE;
          Logger::info("红色采样完成");
          Logger::info("请将蓝色物体放置在传感器前，然后按'c'继续");
          break;
        case STATE_SAMPLING_BLUE:
          currentState = STATE_AWAIT_YELLOW;
          Logger::info("蓝色采样完成");
          Logger::info("请将黄色物体放置在传感器前，然后按'c'继续");
          break;
        case STATE_SAMPLING_YELLOW:
          currentState = STATE_AWAIT_BLACK;
          Logger::info("黄色采样完成");
          Logger::info("请将黑色物体放置在传感器前，然后按'c'继续");
          break;
        case STATE_SAMPLING_BLACK:
          currentState = STATE_AWAIT_WHITE;
          Logger::info("黑色采样完成");
          Logger::info("请将白色物体放置在传感器前，然后按'c'继续");
          break;
        case STATE_SAMPLING_WHITE:
          currentState = STATE_COMPLETE;
          Logger::info("白色采样完成");
          Logger::info("校准流程完成！使用'p'打印结果");
          break;
        default:
          break;
      }
      
      // 重置样本计数
      sampleCount = 0;
    }
  } else {
    Logger::error("HSL读取失败");
  }
}

// 处理用户命令
void processCommand(char command) {
  switch (command) {
    case 's':
      startCalibration();
      break;
      
    case 'r':
      resetCalibration();
      break;
      
    case 'p':
      printCalibrationResults();
      break;
      
    case 'c':
      // 继续到下一步
      switch (currentState) {
        case STATE_AWAIT_RED:
          currentState = STATE_SAMPLING_RED;
          Logger::info("开始采样红色...");
          sampleCount = 0;
          break;
        case STATE_AWAIT_BLUE:
          currentState = STATE_SAMPLING_BLUE;
          Logger::info("开始采样蓝色...");
          sampleCount = 0;
          break;
        case STATE_AWAIT_YELLOW:
          currentState = STATE_SAMPLING_YELLOW;
          Logger::info("开始采样黄色...");
          sampleCount = 0;
          break;
        case STATE_AWAIT_BLACK:
          currentState = STATE_SAMPLING_BLACK;
          Logger::info("开始采样黑色...");
          sampleCount = 0;
          break;
        case STATE_AWAIT_WHITE:
          currentState = STATE_SAMPLING_WHITE;
          Logger::info("开始采样白色...");
          sampleCount = 0;
          break;
        default:
          Logger::info("当前状态无法继续: %s", stateToString(currentState));
          break;
      }
      break;
      
    case 'i':
      Logger::info("传感器状态:");
      colorSensor.debugPrint();
      break;
      
    case '?':
      printMenu();
      break;
      
    default:
      if (command != '\n' && command != '\r') {
        Logger::warning("未知命令: %c", command);
        Logger::info("输入 '?' 查看帮助");
      }
      break;
  }
}

void setup() {
  // 初始化日志系统
  Logger::init();
  
  // 设置串口波特率为9600，与测试README文档相符
  Serial.begin(9600);
  while (!Serial) {
    ; // 等待串口连接
  }
  
  // 设置日志级别
  Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_DEBUG);
  Logger::setLogTag(COMM_SERIAL, "ColorCal");
  
  Logger::info("===============================");
  Logger::info("   感为颜色传感器校准程序");
  Logger::info("===============================");
  Logger::info("正在初始化传感器...");
  
  // 初始化I2C
  Wire.begin();
  
  // 初始化颜色传感器
  if (colorSensor.begin()) {
    Logger::info("颜色传感器初始化成功");
  } else {
    Logger::error("颜色传感器初始化失败，请检查连接");
    Logger::error("I2C地址: 0x%02X", GANWEI_COLOR_SENSOR_ADDR);
    Logger::error("请确认传感器连接和地址设置正确");
  }
  
  delay(500);
  printMenu();
}

void loop() {
  // 检查是否有串口命令
  if (Serial.available()) {
    char c = Serial.read();
    processCommand(c);
  }
  
  // 如果处于采样状态，按照采样间隔执行采样
  if ((currentState == STATE_SAMPLING_RED ||
       currentState == STATE_SAMPLING_BLUE ||
       currentState == STATE_SAMPLING_YELLOW ||
       currentState == STATE_SAMPLING_BLACK ||
       currentState == STATE_SAMPLING_WHITE) &&
      (millis() - lastSampleTime >= SAMPLE_DELAY_MS)) {
    
    sampleColor();
    lastSampleTime = millis();
  }
  
  // 延时以避免过于频繁的输出
  delay(50);
}
#endif 
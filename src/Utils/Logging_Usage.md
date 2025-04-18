# Arduino 增强日志系统使用手册

## 1. 简介

这个增强的日志系统专为Arduino项目设计，提供了灵活、高效且功能丰富的日志记录能力。系统特点包括：

- 支持多种输出通道（串口、蓝牙、ESP32等）
- 不同日志级别（ERROR、WARNING、INFO、DEBUG）
- 时间戳和标签支持
- 针对每个通道的独立配置
- 按标签过滤日志
- 内存优化设计

## 2. 基本使用

### 初始化日志系统

在`setup()`函数中初始化日志系统：

```cpp
void setup() {
  // 初始化日志系统（会自动初始化Serial）
  Logger::init();
  
  // 其他初始化代码...
}
```

### 记录不同级别的日志

```cpp
// 错误日志 - 严重问题导致程序不能继续运行
Logger::error("电机驱动失败，错误代码: %d", errorCode);

// 警告日志 - 可能导致异常但不会阻止程序运行的问题
Logger::warning("电池电量低 (%d%%)", batteryLevel);

// 信息日志 - 正常运行的重要信息
Logger::info("系统初始化完成，版本: %s", VERSION);

// 调试日志 - 详细的调试信息，仅在开发时使用
Logger::debug("PID参数: P=%f, I=%f, D=%f", kP, kI, kD);
```

## 3. 配置日志系统

### 设置日志级别

```cpp
// 设置全局日志级别（对所有通道生效）
Logger::setGlobalLogLevel(LOG_LEVEL_DEBUG);  // 显示所有日志

// 为特定通道设置日志级别
Logger::setLogLevel(COMM_SERIAL, LOG_LEVEL_INFO);  // 串口只显示INFO及以上级别
Logger::setLogLevel(COMM_BT, LOG_LEVEL_ERROR);     // 蓝牙只显示ERROR级别
Logger::setLogLevel(COMM_ESP32, LOG_LEVEL_DEBUG);  // ESP32显示所有日志级别
```

### 配置通信通道

```cpp
// 设置蓝牙流
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN);
void setup() {
  // 初始化日志系统
  Logger::init();
  
  // 初始化并设置蓝牙串口
  btSerial.begin(9600);
  Logger::setStream(COMM_BT, &btSerial);
  Logger::enableComm(COMM_BT, true);
  
  // 初始化并设置ESP32串口
  Serial2.begin(ESP_BAUD_RATE);
  Logger::setStream(COMM_ESP32, &Serial2);
  Logger::enableComm(COMM_ESP32, true);
}
```

### 启用/禁用通道

```cpp
// 禁用蓝牙日志（节省资源）
Logger::enableComm(COMM_BT, false);

// 启用ESP32日志
Logger::enableComm(COMM_ESP32, true);

// 使用Config.h中的定义控制通道开关
Logger::enableComm(COMM_BT, ENABLE_BLUETOOTH);
Logger::enableComm(COMM_ESP32, ENABLE_ESP);
```

## 4. 高级特性

### 使用日志标签

标签可以帮助识别日志来源，例如区分不同模块的日志。

```cpp
// 方式1：为当前通道设置默认标签
Logger::setLogTag(COMM_SERIAL, "MOTOR");
Logger::info("电机速度: %d", motorSpeed);  // 输出: [INFO][MOTOR] 电机速度: 123

// 方式2：直接在日志调用中指定标签（推荐）
Logger::info("MOTOR", "电机速度: %d", motorSpeed);  // 输出: [INFO][MOTOR] 电机速度: 123

// 设置全局标签（所有通道）
Logger::setGlobalLogTag("SYSTEM");
```

### 按标签过滤日志

新特性：可以为特定标签设置日志级别，实现更精细的日志过滤。

```cpp
// 默认情况下，全局日志级别设置为INFO
Logger::setGlobalLogLevel(LOG_LEVEL_INFO);

// 为特定标签设置更详细的日志级别
Logger::setLogLevelForTag("MOTOR", LOG_LEVEL_DEBUG);  // 显示MOTOR标签的所有DEBUG级别日志
Logger::setLogLevelForTag("WIFI", LOG_LEVEL_ERROR);   // 仅显示WIFI标签的ERROR级别日志

// 重置特定标签的日志级别（恢复为全局设置）
Logger::resetLogLevelForTag("MOTOR");

// 重置所有标签的日志级别（恢复为全局设置）
Logger::resetAllTagLogLevels();
```

这样，当全局日志级别设置为INFO时，只有MOTOR模块会显示DEBUG级别的日志，而WIFI模块只会显示ERROR级别的日志，其他模块则遵循全局设置。

### 日志格式配置

可以为每个通道自定义日志配置：

```cpp
// 创建自定义日志配置
LoggerConfig config;
config.logLevel = LOG_LEVEL_INFO;
config.useTimestamp = true;
config.usePrefix = false;
strncpy(config.tag, "MOTOR", sizeof(config.tag));

// 应用配置到蓝牙通道
Logger::configureChannel(COMM_BT, config);
```

## 5. 内部工作原理

- 共享静态缓冲区减少内存使用
- `F()`宏将常量字符串存储在Flash中节省RAM
- 时间戳基于系统启动后的运行时间
- 日志采用统一的内部处理函数提高代码复用
- 标签日志级别使用小型内存池存储（最多支持10个标签）

## 6. 完整示例

```cpp
#include "Utils/Logger.h"
#include <SoftwareSerial.h>
#include "Utils/Config.h"

// 定义引脚
#define BT_RX_PIN 2
#define BT_TX_PIN 3

// 创建蓝牙串口
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN);

void setup() {
  // 初始化日志系统
  Logger::init();
  
  // 配置蓝牙
  btSerial.begin(9600);
  Logger::setStream(COMM_BT, &btSerial);
  Logger::enableComm(COMM_BT, ENABLE_BLUETOOTH);
  
  // 配置ESP32 (使用Serial2)
  Serial2.begin(ESP_BAUD_RATE);
  Logger::setStream(COMM_ESP32, &Serial2);
  Logger::enableComm(COMM_ESP32, ENABLE_ESP);
  
  // 配置日志级别
  Logger::setGlobalLogLevel(LOG_LEVEL_INFO);
  
  // 为特定模块启用调试日志
  Logger::setLogLevelForTag("MOTOR", LOG_LEVEL_DEBUG);
  Logger::setLogLevelForTag("SENSOR", LOG_LEVEL_DEBUG);
  
  // 记录启动信息
  Logger::info("SYSTEM", "系统启动完成");
  Logger::debug("SYSTEM", "调试模式已启用"); // 不会显示，因为SYSTEM标签没有设置为DEBUG级别
}

void loop() {
  // 电机控制示例
  static int motorSpeed = 0;
  motorSpeed = (motorSpeed + 10) % 255;
  
  // 使用标签记录电机信息
  Logger::info("MOTOR", "电机速度更新: %d", motorSpeed);
  Logger::debug("MOTOR", "PWM值: %d, 方向: 正向", motorSpeed); // 会显示，因为MOTOR标签设为DEBUG级别
  
  // 读取传感器示例
  int sensorValue = analogRead(A0);
  if(sensorValue > 1000) {
    Logger::warning("SENSOR", "传感器读数异常: %d", sensorValue);
  } else {
    Logger::debug("SENSOR", "传感器读数: %d", sensorValue); // 会显示，因为SENSOR标签设为DEBUG级别
  }
  
  delay(1000);
}
```

## 7. 故障排除

- **没有日志输出**：检查通道是否启用、日志级别是否正确设置、串口是否正确初始化
- **只有部分日志显示**：检查标签日志级别和通道日志级别的组合设置
- **内存不足**：减小缓冲区大小，减少标签数量（MAX_TAGS默认为10）
- **格式化不正确**：Arduino的`vsnprintf`支持有限，避免复杂的格式化字符串

## 8. 后续改进方向

- SD卡日志支持
- 循环缓冲区记录
- 更多的输出格式选项
- 已实现ESP32无线日志传输
- 支持更多无线通信协议（如LoRa等）
- 动态内存分配的标签支持

---

*文档版本: 2.1* 
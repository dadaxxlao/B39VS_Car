# Arduino 增强日志系统使用手册

## 1. 简介

这个增强的日志系统专为Arduino项目设计，提供了灵活、高效且功能丰富的日志记录能力。系统特点包括：

- 支持多种输出通道（串口、蓝牙等）
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
}
```

**新增：配置与其他 UART 设备通信 (例如 ESP32 on Arduino Mega)**

如果你使用的是像 Arduino Mega 这样具有多个硬件串口的板子，或者使用 SoftwareSerial 连接到特定引脚的外部设备（如 ESP32），你可以配置 Logger 将日志发送到该串口。

```cpp
// 假设 Arduino Mega 通过引脚 16(TX2), 17(RX2) 连接 ESP32
// #include "Utils/Config.h" // 假设 ESP_BAUD_RATE 在此定义

void setup() {
  // 初始化本地 USB 串口
  Serial.begin(115200);
  
  // 初始化 Logger
  Logger::init();

  // 初始化连接 ESP32 的硬件串口 Serial2
  Serial2.begin(ESP_BAUD_RATE); 

  // 配置 Logger 使用 Serial2
  Logger::setStream(COMM_ESP32, &Serial2);   // 关联 Serial2 到 COMM_ESP32
  Logger::enableComm(COMM_ESP32, true);    // 启用 COMM_ESP32 通道
  Logger::setLogLevel(COMM_ESP32, LOG_LEVEL_DEBUG); // 设置该通道的日志级别

  // (可选) 禁用 Logger 通过默认 Serial (USB口) 输出日志
  // Logger::enableComm(COMM_SERIAL, false);

  Logger::info("SYSTEM", "Logger configured for ESP32 via Serial2.");
}
```

对于使用 SoftwareSerial 的情况，只需将 `&Serial2` 替换为你的 `SoftwareSerial` 对象指针即可。

### 启用/禁用通道

```cpp
// 禁用蓝牙日志（节省资源）
Logger::enableComm(COMM_BT, false);

// 禁用 COMM_ESP32 通道日志
Logger::enableComm(COMM_ESP32, false);

// 禁用默认串口 (USB) 日志输出
Logger::enableComm(COMM_SERIAL, false);

// 启用其他通信日志...
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
#include "Utils/Config.h" // 假设包含 ESP_BAUD_RATE
// #include <SoftwareSerial.h> // 如果使用 SoftwareSerial

// 如果使用 SoftwareSerial，取消注释下一行并定义引脚
// SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN);

void setup() {
  // 初始化本地 USB 串口
  Serial.begin(115200);
  
  // 初始化 Logger 系统
  Logger::init();
  
  // --- 配置与 ESP32 通信 (使用 Serial2 on Mega) ---
  Serial2.begin(ESP_BAUD_RATE);
  Logger::setStream(COMM_ESP32, &Serial2);
  Logger::enableComm(COMM_ESP32, true);
  // --- End ESP32 Config ---

  // --- 或者配置与 ESP32 通信 (使用 SoftwareSerial) ---
  // espSerial.begin(ESP_BAUD_RATE);
  // Logger::setStream(COMM_ESP32, &espSerial);
  // Logger::enableComm(COMM_ESP32, true);
  // --- End SoftwareSerial Config ---
  
  // 配置日志级别
  Logger::setGlobalLogLevel(LOG_LEVEL_INFO); // 全局 INFO
  Logger::setLogLevel(COMM_ESP32, LOG_LEVEL_DEBUG); // ESP32 通道接收 DEBUG 信息
  // Logger::enableComm(COMM_SERIAL, false); // (可选) 禁用 USB 日志
  
  // 为特定模块启用调试日志 (会通过 COMM_ESP32 发送)
  Logger::setLogLevelForTag("MOTOR", LOG_LEVEL_DEBUG);
  Logger::setLogLevelForTag("SENSOR", LOG_LEVEL_DEBUG);
  
  // 记录启动信息
  Logger::info("SYSTEM", "系统启动完成"); // 通过 Serial 和 Serial2 发送 (如果 COMM_SERIAL 未禁用)
  Logger::debug("SYSTEM", "调试模式已启用"); // 只通过 Serial2 发送
}

void loop() {
  // 电机控制示例
  static int motorSpeed = 0;
  motorSpeed = (motorSpeed + 10) % 255;
  
  // 使用标签记录电机信息 (会通过 Serial2 发送)
  Logger::info("MOTOR", "电机速度更新: %d", motorSpeed);
  Logger::debug("MOTOR", "PWM值: %d, 方向: 正向", motorSpeed); 
  
  // 读取传感器示例
  int sensorValue = analogRead(A0);
  if(sensorValue > 1000) {
    Logger::warning("SENSOR", "传感器读数异常: %d", sensorValue);
  } else {
    Logger::debug("SENSOR", "传感器读数: %d", sensorValue); 
  }

  // --- 示例: 直接通过 Serial2 与 ESP32 通信 --- 
  // if (Serial2.available() > 0) { // 检查来自 ESP32 的数据
  //   String cmd = Serial2.readStringUntil('\n');
  //   Serial.print("Received via Serial2: "); Serial.println(cmd); // 本地调试
  //   // process command...
  // }
  // static unsigned long lastPing = 0;
  // if (millis() - lastPing > 5000) { // 每5秒发送一次 PING
  //   Serial2.println("PING_FROM_ARDUINO");
  //   lastPing = millis();
  // }
  // --- End Serial2 Example ---
  
  delay(1000);
}
```

## 7. 故障排除

- **没有日志输出**：检查通道是否启用 (`Logger::enableComm`)、日志级别是否正确设置 (`Logger::setLogLevel`, `Logger::setGlobalLogLevel`, `Logger::setLogLevelForTag`)、对应的串口（`Serial`, `Serial2`, `btSerial`, `espSerial`等）是否正确初始化 (`.begin()`) 并已关联 (`Logger::setStream`)。
- **只有部分日志显示**：检查标签日志级别 (`Logger::setLogLevelForTag`) 和通道日志级别 (`Logger::setLogLevel`) 的组合设置。
- **内存不足**：减小缓冲区大小，减少标签数量（MAX_TAGS默认为10）
- **格式化不正确**：Arduino的`vsnprintf`支持有限，避免复杂的格式化字符串

## 8. 后续改进方向

- SD卡日志支持
- 循环缓冲区记录
- 更多的输出格式选项
- 无线日志传输
- 动态内存分配的标签支持

---

*文档版本: 2.1* 
# ESP32-Arduino 通信指南

本文档介绍如何使用ESP32C3与Arduino进行通信，包括基本设置、远程调试、透明传输模式等功能。

## 1. 硬件连接

Arduino和ESP32C3之间通过UART串口进行通信：

- Arduino的`ESP_RX_PIN`(18) → ESP32的TX引脚(19)
- Arduino的`ESP_TX_PIN`(19) → ESP32的RX引脚(18)
- 共享地线(GND)

## 2. 配置设置

### 2.1 Arduino端配置

在`Config.h`文件中确保ESP通信已启用：

```cpp
// 通信模式配置
// 设置为1启用对应的通信方式，设置为0禁用
// ESP32通信功能配置
#define ENABLE_ESP           1  // 确保这个值为1

// ESP32通信引脚和参数
#define ESP_RX_PIN           18  // Arduino接收，连接到ESP32的TX
#define ESP_TX_PIN           19  // Arduino发送，连接到ESP32的RX
#define ESP_BAUD_RATE        115200
```

### 2.2 ESP32端配置

ESP32固件中的串口初始化：

```cpp
// 初始化Arduino串口 (UART1)
ArduinoSerial.begin(115200, SERIAL_8N1, 18, 19); // RX=18, TX=19
```

**注意**：确保两端波特率设置相同(115200)，否则会导致通信错误。

## 3. Arduino实现

### 3.1 初始化ESP桥接

```cpp
#include "Utils/EspBridge.h"

// 创建ESP桥接对象
EspBridge espBridge;

void setup() {
  // 初始化Serial用于调试输出
  Serial.begin(115200);
  
  // 初始化ESP桥接
  if (espBridge.begin(ESP_RX_PIN, ESP_TX_PIN, ESP_BAUD_RATE)) {
    Serial.println("ESP桥接初始化成功");
  } else {
    Serial.println("ESP桥接初始化失败或已禁用");
  }
}
```

### 3.2 处理通信数据

```cpp
void loop() {
  // 处理来自ESP的数据
  espBridge.processReceivedData();
  
  // 其他循环代码...
}
```

### 3.3 发送数据到ESP32

```cpp
// 发送日志
Logger::debug("这是调试信息");
Logger::info("这是信息");
Logger::warning("这是警告");
Logger::error("这是错误");

// 发送状态
espBridge.sendState(STATE_RUNNING, "正常运行");

// 发送传感器数据
char sensorData[64];
sprintf(sensorData, "temp=25.5,humid=60,dist=45");
espBridge.sendData(DATA_SENSORS, sensorData);
```

## 4. 远程调试功能

ESP32固件实现了远程串口调试功能，可通过WiFi网络访问Arduino的串口。

### 4.1 通过Web界面控制

1. 连接到ESP32的WiFi或确保ESP32已连接到您的WiFi网络
2. 访问ESP32的Web界面：`http://[ESP32_IP_地址]`
3. 在"远程调试"区域点击"开启/关闭远程调试"按钮
4. 调试状态将显示当前是否已连接和开启透明模式

### 4.2 使用TCP客户端连接

可使用telnet或其他TCP客户端连接ESP32进行调试：

```
telnet 192.168.x.x 8880
```

连接成功后，将显示欢迎信息和可用命令。

### 4.3 调试命令

透明传输模式下可使用以下命令：

- `$transparent` - 进入透明传输模式，所有输入直接发送到Arduino
- `$normal` - 返回普通模式
- `$help` - 显示帮助信息

## 5. 透明传输模式

透明传输模式可让您通过网络直接与Arduino通信，就像直接连接到Arduino的串口一样。

### 5.1 开启透明传输模式

通过TCP客户端发送：
```
$transparent
```

或通过Web界面点击"开启/关闭远程调试"按钮开启。

### 5.2 使用透明传输模式

开启透明传输模式后：

1. 所有输入都会直接转发到Arduino
2. Arduino发送的所有数据都会显示在TCP客户端中
3. 无需特殊格式化，直接与Arduino通信

### 5.3 关闭透明传输模式

发送：
```
$normal
```

或通过Web界面点击"开启/关闭远程调试"按钮关闭。

## 6. 通信协议

ESP32和Arduino之间使用以下格式进行通信：

- 命令消息：`$CMD:命令名[,参数]#`
- 状态消息：`$STATE:状态码[,参数]#`
- 日志消息：`$LOG:级别,消息内容#`
- 数据消息：`$DATA:数据类型,数据内容#`

其中`#`是消息结束标记。

## 7. 故障排除

1. **无法连接到ESP32**
   - 检查WiFi连接是否正常
   - 确认ESP32的IP地址是否正确

2. **无法与Arduino通信**
   - 检查Arduino和ESP32之间的连线是否正确
   - 确认波特率设置是否匹配(115200)
   - 检查`ENABLE_ESP`是否设为1

3. **数据传输错误或乱码**
   - 检查波特率设置
   - 确保发送的数据没有超过缓冲区大小
   - 检查硬件连接

4. **ESP32无响应**
   - 检查电源供应是否稳定
   - 尝试重启ESP32和Arduino

## 8. 扩展功能

可考虑实现以下扩展功能：

1. 加密通信以提高安全性
2. 实现更多调试命令
3. 添加数据记录和分析功能
4. 实现固件OTA更新

---

*此文档由B39VS小车项目团队维护，最后更新：2023年* 
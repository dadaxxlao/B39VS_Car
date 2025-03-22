# ESP32C3 通信扩展模块

该模块用于为B39VS智能小车项目提供Wi-Fi和蓝牙通信功能，可通过串口与Arduino主控板进行通信，实现远程控制、状态监控和数据传输功能。

## 功能特性

- **双无线通信**：同时支持Wi-Fi和蓝牙(BLE)
- **Web控制界面**：提供基于WebSocket的实时控制页面
- **API接口**：提供RESTful API用于远程控制
- **状态监控**：实时显示小车状态和传感器数据
- **日志系统**：远程查看系统日志
- **桥接通信**：与Arduino主控板通过串口通信

## 硬件需求

- ESP32-C3开发板 ([合宙ESP32C3-CORE开发板](https://wiki.luatos.com/chips/esp32c3/board.html))
- USB数据线（用于供电和编程）
- 连接Arduino的杜邦线（2根用于TX/RX）

## 引脚连接

ESP32C3与Arduino的连接方式：

| ESP32C3引脚 | Arduino引脚 | 功能        |
|------------|------------|------------|
| U0_TX (9)  | 自定义RX引脚(18)  | 串口发送     |
| U0_RX (8)  | 自定义TX引脚(19)  | 串口接收     |
| GND        | GND        | 共同接地     |
| GPIO12    | -          | 蓝牙状态LED  |
| GPIO13    | -          | WiFi状态LED |

## 开发环境搭建

### 使用PlatformIO进行开发

1. 安装[Visual Studio Code](https://code.visualstudio.com/)
2. 安装[PlatformIO扩展](https://platformio.org/install/ide?install=vscode)
3. 打开项目目录
4. 编译并上传固件到ESP32C3开发板

### 配置WiFi和通信参数

编辑`src/main.cpp`文件，修改以下参数：

```cpp
// WiFi配置
const char* WIFI_SSID = "YourWiFiSSID";     // 修改为您的WiFi名称
const char* WIFI_PASSWORD = "YourPassword"; // 修改为您的WiFi密码
const char* HOSTNAME = "b39vs-car";         // 主机名
```

## 使用方法

### ESP32侧配置

1. 根据上述说明配置WiFi参数
2. 设置通信参数（默认已配置）：
   ```cpp
   #define SERIAL_BAUD_RATE 115200  // 与Arduino通信的波特率
   ```
3. 如需修改BLE服务名称和特征值UUID，编辑以下参数：
   ```cpp
   #define BLE_SERVICE_NAME "B39VS_Car"
   #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
   #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
   ```

### Arduino侧配置

1. 在Arduino项目的`Config.h`文件中启用ESP32通信：
   ```cpp
   #define ENABLE_ESP 1
   ```
2. 确认Arduino的ESP32通信引脚设置：
   ```cpp
   #define ESP_RX_PIN 18  // Arduino接收，连接到ESP32的TX
   #define ESP_TX_PIN 19  // Arduino发送，连接到ESP32的RX
   #define ESP_BAUD_RATE 115200
   ```

### Arduino与ESP32交互

#### 初始化ESP32通信

```cpp
// 初始化ESP32通信
if (EspComm.begin(ESP_RX_PIN, ESP_TX_PIN, ESP_BAUD_RATE)) {
  Logger::info("ESP32通信初始化成功");
} else {
  Logger::error("ESP32通信初始化失败，请检查连接");
}
```

#### 发送数据到ESP32

```cpp
// 发送系统状态
EspComm.sendState(STATE_RUNNING, "任务执行中");

// 发送传感器数据
char sensorData[128];
sprintf(sensorData, "line_pos=%d,sensors=[%d,%d,%d,%d,%d,%d,%d,%d]",
        linePosition, sensorValues[0], sensorValues[1], 
        sensorValues[2], sensorValues[3], sensorValues[4], 
        sensorValues[5], sensorValues[6], sensorValues[7]);
EspComm.sendData(DATA_SENSORS, sensorData);

// 发送日志（通过Logger系统自动发送）
Logger::info("系统初始化完成");
Logger::warning("电池电量低");
```

#### 接收ESP32命令

```cpp
void loop() {
  // 处理ESP32发来的命令
  EspComm.update();
  
  // 其他代码...
}
```

### 测试模式

为了测试ESP32C3模块的功能，可以使用测试程序：

1. 将`src/test_esp_serial.cpp`重命名为`main.cpp`（备份原有的main.cpp）
2. 编译并上传到ESP32C3开发板
3. 打开串口监视器，波特率设置为115200
4. 测试程序会模拟Arduino发送状态和传感器数据，并响应命令

### 正常使用

1. 确保`src/main.cpp`包含正式的桥接代码
2. 编译并上传到ESP32C3开发板
3. 连接ESP32C3与Arduino主控板
4. 通过WiFi或蓝牙连接到ESP32C3
5. 使用Web界面或BLE应用进行控制

### Web界面访问

1. ESP32C3连接到WiFi后，查看串口输出获取IP地址
2. 在浏览器中访问`http://<ESP32C3的IP地址>/`
3. 或者ESP32C3会创建一个AP热点（如果无法连接WiFi），名称为`b39vs-car`，密码：`12345678`
4. Web界面功能：
   - 实时状态监控
   - 远程控制车辆
   - 查看传感器数据
   - 实时日志显示

### 蓝牙连接

1. 使用BLE扫描器应用查找并连接名为`B39VS_Car`的设备
2. 蓝牙应用功能：
   - 发送控制命令
   - 接收状态更新
   - 查看传感器数据
   - 监控日志信息

## 通信协议

ESP32C3与Arduino之间的通信协议使用特定前缀和终止符来区分不同类型的消息：

- **命令消息**：`$CMD:command,params#`
  - 例如：`$CMD:START#` 或 `$CMD:CONFIG,speed=100,mode=1#`
  
- **状态消息**：`$STATE:stateCode,details#`
  - 例如：`$STATE:1,运行中#` 或 `$STATE:0,已停止#`
  
- **日志消息**：`$LOG:level,message#`
  - 例如：`$LOG:INFO,系统初始化完成#` 或 `$LOG:ERROR,传感器连接失败#`
  
- **数据消息**：`$DATA:dataType,values#`
  - 例如：`$DATA:SENSORS,line_pos=3500,sensors=[100,200,300,400,500,600,700,800]#`

所有消息都以`#`字符结尾作为消息终止标记。

### 命令类型

ESP32可以发送以下命令到Arduino：

| 命令名称 | 描述 | 参数示例 |
|---------|------|---------|
| START | 开始执行任务 | 无参数 |
| STOP | 停止执行 | 无参数 |
| RESET | 重置系统 | 无参数 |
| CONFIG | 配置系统参数 | speed=100,mode=1 |
| GET_STATUS | 获取当前状态 | 无参数 |
| CUSTOM_MOTION | 自定义运动指令 | direction=forward,speed=150,time=1000 |

### 状态代码

Arduino会向ESP32发送以下状态代码：

| 状态代码 | 描述 |
|---------|------|
| 0 | 空闲状态 |
| 1 | 运行状态 |
| 2 | 暂停状态 |
| 3 | 错误状态 |
| 4 | 低电量状态 |
| 5 | 返回基地状态 |

## Web API接口

ESP32C3提供以下RESTful API接口：

### 获取状态

```
GET /api/status
```

响应示例：
```json
{
  "state": 1,
  "stateText": "运行中",
  "battery": 85,
  "connected": true
}
```

### 发送命令

```
POST /api/command
```

请求体示例：
```json
{
  "command": "START",
  "params": {}
}
```

或：
```json
{
  "command": "CUSTOM_MOTION",
  "params": {
    "direction": "forward",
    "speed": 150,
    "time": 1000
  }
}
```

响应示例：
```json
{
  "success": true,
  "message": "命令已发送"
}
```

### 获取最新传感器数据

```
GET /api/data
```

响应示例：
```json
{
  "linePosition": 3500,
  "sensors": [100, 200, 300, 400, 500, 600, 700, 800],
  "timestamp": 1621234567
}
```

## 测试和调试

1. 使用串口监视器查看调试输出
2. Web界面中的日志面板可实时显示系统日志
3. 查看板载LED状态（GPIO12：蓝牙连接状态，GPIO13：WiFi连接状态）
4. 使用内置的`TEST_ESP_COMM`测试程序在Arduino端进行通信测试

## 常见问题

1. **无法连接WiFi**：
   - 检查WiFi凭据设置
   - ESP32C3将自动创建AP热点(SSID: b39vs-car, 密码: 12345678)
   
2. **串口通信问题**：
   - 确保波特率设置为115200
   - 检查TX/RX连接线是否正确连接（需要交叉连接）
   - 使用串口监视器观察通信过程

3. **Web界面无响应**：
   - 检查浏览器控制台是否有错误信息
   - 确认WebSocket连接建立成功
   - 尝试刷新页面或重启ESP32

4. **蓝牙连接失败**：
   - 尝试重启设备
   - 检查BLE设备名称是否正确
   - 确认手机蓝牙功能已开启

5. **命令未被执行**：
   - 确认命令格式正确，包含前缀和终止符
   - 检查命令名称是否匹配预定义命令
   - 使用串口监视器观察命令是否被ESP32接收和转发

## 项目结构

```
esp_firmware/
├── platformio.ini           # PlatformIO配置文件
├── src/                    # 源代码目录
│   ├── main.cpp            # 主程序
│   ├── BLEService.cpp      # 蓝牙服务实现
│   ├── BLEService.h        # 蓝牙服务头文件
│   ├── WebServer.cpp       # Web服务器实现
│   ├── WebServer.h         # Web服务器头文件
│   ├── ArduinoBridge.cpp   # Arduino通信桥实现
│   ├── ArduinoBridge.h     # Arduino通信桥头文件
│   └── test_esp_serial.cpp # 测试程序
└── shared/                 # 共享代码目录
    └── CommProtocol.h      # 通信协议定义
``` 
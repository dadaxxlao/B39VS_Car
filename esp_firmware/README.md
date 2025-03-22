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
| U0_TX (9)  | 自定义RX引脚  | 串口发送     |
| U0_RX (8)  | 自定义TX引脚  | 串口接收     |
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

### 蓝牙连接

使用BLE扫描器应用查找并连接名为`B39VS_Car`的设备。

## 通信协议

ESP32C3与Arduino之间的通信协议：

- **命令消息**：`$CMD:command,params#`
- **状态消息**：`$STATE:stateCode,details#`
- **日志消息**：`$LOG:level,message#`
- **数据消息**：`$DATA:dataType,values#`

## 测试和调试

1. 使用串口监视器查看调试输出
2. Web界面中的日志面板可实时显示系统日志
3. 查看板载LED状态（GPIO12：蓝牙连接状态，GPIO13：WiFi连接状态）

## 常见问题

1. **无法连接WiFi**：检查WiFi凭据设置，ESP32C3将自动创建AP热点
2. **串口通信问题**：确保波特率设置为115200，检查连接线
3. **Web界面无响应**：检查浏览器控制台是否有错误信息
4. **蓝牙连接失败**：尝试重启设备，检查BLE设备名称

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
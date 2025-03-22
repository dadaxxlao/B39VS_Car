# B39VS_Car 智能小车项目

这是一个基于Arduino的智能小车项目，包含传感器控制、电机驱动、路径规划等功能。项目使用PlatformIO作为开发环境。

## 项目结构

```
B39VS_Car/
├── src/                    # 源代码目录
│   ├── Arm/                # 机械臂控制模块
│   ├── Control/            # 控制算法模块
│   ├── Motor/              # 电机驱动模块
│   ├── Sensor/             # 传感器接口模块
│   ├── Test/               # 测试程序
│   └── Utils/              # 工具类和配置
├── platformio.ini          # PlatformIO配置文件
├── Design.md               # 设计文档
└── get-platformio.py       # PlatformIO安装辅助脚本
```

## 硬件要求

- Arduino Mega 2560 (ATmega2560)
- 电机驱动模块 (L298N或类似)
- 红外线传感器阵列 (I2C接口)
- TCS34725颜色传感器 (I2C接口)
- 超声波传感器 (HC-SR04或类似)
- 舵机 (SG90或类似)
- RGB LED灯环 (可选)
- HM-10蓝牙模块 (可选)

## 环境搭建

### 安装PlatformIO

本项目使用PlatformIO作为开发环境，它是一个跨平台的嵌入式开发工具。

#### 方法1：通过VS Code扩展安装

1. 安装[Visual Studio Code](https://code.visualstudio.com/)
2. 打开VS Code扩展市场，搜索并安装"PlatformIO IDE"扩展
3. 安装完成后重启VS Code

#### 方法2：使用项目附带的安装脚本

```bash
python get-platformio.py
```

这个脚本会自动下载和安装PlatformIO核心组件，并指导您完成初始设置。

### 克隆项目

```bash
git clone https://github.com/yourusername/B39VS_Car.git
cd B39VS_Car
```

### 打开项目

1. 在VS Code中，选择"文件" -> "打开文件夹"，然后选择克隆的B39VS_Car目录
2. PlatformIO将自动识别项目并加载配置

## 编译和上传

### 编译项目

在VS Code中，点击底部状态栏的"Build"按钮，或使用快捷键:
- Windows/Linux: `Ctrl+Alt+B`
- macOS: `Cmd+Alt+B`

### 上传到Arduino

1. 将Arduino Mega通过USB连接到电脑
2. 在VS Code中，点击底部状态栏的"Upload"按钮，或使用快捷键:
   - Windows/Linux: `Ctrl+Alt+U`
   - macOS: `Cmd+Alt+U`

## 运行测试

项目包含多个测试模块，每次只能运行一个测试。要选择要运行的测试，需要修改`platformio.ini`文件中的`build_flags`：

```ini
[env:arduino_mega]
platform = atmelavr
board = megaatmega2560
framework = arduino
build_flags = -D TEST_INFRARED  # 修改这一行来选择测试
```

可用的测试选项:
- `TEST_INFRARED` - 红外线传感器测试
- `TEST_COLOR_SENSOR` - 颜色传感器测试
- `TEST_COLOR_CALIBRATION` - 颜色传感器校准测试
- `TEST_CALIBRATION` - 传感器校准测试
- `TEST_ULTRASONIC` - 超声波传感器测试
- `TEST_MULTIPLE_JUNCTION_DETECTION` - 路口检测测试
- `TEST_SENSOR_MANAGER` - 传感器管理器测试
- `TEST_MECANUM_MOTION` - 麦克纳姆轮运动测试
- `TEST_LINE_FOLLOWING` - 巡线功能测试
- `TEST_JUNCTION_FOLLOWING` - 路口巡线测试
- `TEST_BLUETOOTH` - 蓝牙通信测试

修改后保存文件，然后执行编译和上传操作。

## 串口监视器

测试程序会通过串口输出调试信息。要查看这些信息：

1. 在VS Code中，点击底部状态栏的"Serial Monitor"按钮
2. 或者使用外部串口工具，波特率设置为9600

## 麦克纳姆轮运动控制

本项目支持麦克纳姆轮全向移动，提供灵活的运动控制能力。

### 麦克纳姆轮特性

- 支持全向移动（前进、后退、左右平移、对角线移动）
- 支持原地旋转和弧线运动
- 通过电机补偿系数保证稳定运动

### 使用方法

使用`MotionController`类控制麦克纳姆轮运动：

```cpp
// 初始化
motionController.init();

// 基本运动
motionController.moveForward(speed);  // 前进
motionController.moveBackward(speed); // 后退
motionController.turnLeft(speed);     // 左转
motionController.turnRight(speed);    // 右转
motionController.moveLeft(speed);     // 左平移
motionController.moveRight(speed);    // 右平移

// 高级运动控制
// 参数: x (-1.0到1.0), y (-1.0到1.0), rotation (-1.0到1.0)
motionController.mecanumDrive(x, y, rotation);

// 紧急停止
motionController.emergencyStop();
```

### 测试和校准

使用`TEST_MECANUM_MOTION`测试程序校准电机参数和运动性能。测试程序支持：
- 单独动作测试（前进、转向等）
- 自动测试序列
- 精确角度转向校准（90度、180度转向）

## 蓝牙通信功能

本项目支持通过HM-10蓝牙模块实现远程控制和数据传输。

### 蓝牙功能特性

- 远程控制小车运动和功能
- 实时接收传感器数据和状态信息
- 支持结构化消息格式便于移动应用开发
- 集成日志系统，便于远程调试

### 使用方法

#### 硬件连接

蓝牙模块连接到Arduino上：
- HM-10 TX → Arduino 引脚16 (RX)
- HM-10 RX → Arduino 引脚17 (TX)
- VCC → 3.3V
- GND → GND

#### 启用蓝牙功能

在`Config.h`中设置：
```cpp
#define ENABLE_BLUETOOTH 1
```

#### 初始化蓝牙

```cpp
// 初始化蓝牙通信
if (BtSerial.begin(BT_RX_PIN, BT_TX_PIN, BT_BAUD_RATE)) {
  // 设置Logger使用蓝牙输出
  Logger::setBtStream(&BtSerial);
  Logger::enableBluetooth(true);
}
```

#### 接收和处理蓝牙命令

```cpp
// 在循环中处理蓝牙数据
if (BtSerial.processReceivedData()) {
  const char* command = BtSerial.getLastCommand();
  // 处理命令...
}
```

### 蓝牙通信协议

使用前缀标识不同类型的消息：
- 命令: `$CMD:command,param`
- 数据: `$DATA:type,value1,value2,...`
- 日志: `$LOG:level,message`
- 响应: `$RESP:command,result`

### 测试蓝牙功能

使用`TEST_BLUETOOTH`测试程序验证蓝牙通信功能。此测试提供命令接口用于：
- 控制LED和电机
- 设置调试级别
- 获取传感器数据
- 远程启动/停止功能

## 颜色传感器校准和使用

颜色传感器对于小车的寻路功能至关重要，需要正确校准以确保准确识别不同颜色。

### 颜色传感器校准流程

1. **运行校准程序**：
   - 在`platformio.ini`中设置：`build_flags = -D TEST_COLOR_CALIBRATION`
   - 编译并上传程序到Arduino
   - 打开串口监视器（波特率9600）

2. **校准步骤**：
   - 选择"1"进入校准模式
   - 将传感器依次放在红色、蓝色、黄色、白色和黑色表面上
   - 每种颜色放好后按"S"键开始采样
   - 系统会采集10个样本并计算平均值
   - 完成所有颜色校准后，系统将显示建议的阈值设置

3. **应用校准结果**：
   - 记录校准过程中显示的阈值建议值
   - 将这些值复制到`src/Sensor/ColorSensor.cpp`文件的`initColorThresholds()`函数中
   - 或者使用代码中已实现的基于RGB比例的识别算法

4. **测试颜色识别**：
   - 校准完成后，按"T"进入测试模式
   - 或者重新启动后选择"2"直接进入测试模式
   - 系统将持续显示识别结果，可以验证校准效果

### 颜色传感器实现原理

我们的颜色传感器基于TCS34725芯片，实现了两种颜色识别方法：

1. **阈值法**：使用固定的RGB阈值范围判断颜色
2. **RGB比例法**：计算RGB各通道占总亮度的比例来判断颜色（更稳健，不易受环境光强度影响）

可以通过校准过程确定哪种方法在您的环境中表现更好。

### 颜色传感器使用提示

- 保持传感器与被测表面的距离一致（约1-2cm）
- 避免环境光直接照射传感器
- 不同光照条件下可能需要重新校准
- 若识别不稳定，可以尝试调整传感器增益和积分时间
- 颜色校准应在实际使用环境中进行，以获得最佳效果

## 传感器校准

1. 运行`TEST_CALIBRATION`测试
2. 按照串口输出的指示进行操作
3. 完成校准后，将输出的参数添加到相应的配置文件中

## 故障排除

### 编译错误

- **多重定义错误**: 确保一次只定义一个测试标志
- **找不到库**: 运行`pio lib install`安装所需库
- **端口未找到**: 检查USB连接或驱动安装

### 颜色传感器常见问题

- **无法识别颜色**: 检查I2C连接，确保电源稳定，尝试重新校准
- **识别结果不稳定**: 调整传感器位置，避免环境光干扰，增加采样次数
- **颜色混淆**: 重新校准，调整RGB比例判断阈值
- **传感器不响应**: 检查地址设置（默认0x29），重启Arduino

### 蓝牙连接问题

- **无法连接**: 检查电源和串口引脚连接，确认波特率设置正确
- **数据丢失**: 增大处理间隔时间，避免缓冲区溢出
- **命令无响应**: 调整电源供应，确保蓝牙模块稳定工作
- **初始化失败**: 检查AT命令响应，可能需要重置蓝牙模块

### 硬件连接

参考`src/Utils/Config.h`文件中的引脚定义，确保硬件连接正确。

## 最近更新

### 1. 蓝牙通信功能 (2024-05-XX)

- 添加`BluetoothSerial`类处理蓝牙数据传输和命令解析
- 更新`Logger`类支持蓝牙日志输出
- 创建蓝牙测试模块用于验证通信功能
- 添加结构化消息协议便于移动应用开发
- 配置文件中添加蓝牙模块引脚定义

### 2. 麦克纳姆轮支持 (2024-05-XX)

- 重构`MotionController`类支持麦克纳姆轮全向移动
- 添加电机补偿系数以获得更平稳运动
- 优化运动控制算法，提供更精确的方向控制
- 添加原地旋转和弧线运动功能
- 完善测试程序用于校准运动参数

### 3. 开发环境与文档改进 (2024-05-XX)

- 新增PlatformIO安装辅助脚本`get-platformio.py`
- 优化颜色传感器校准和测试功能
- 更新项目文档，添加详细安装和使用说明
- 完善测试程序文档，便于调试和功能验证

## 贡献

欢迎提交问题报告和改进建议！

## 许可

MIT License


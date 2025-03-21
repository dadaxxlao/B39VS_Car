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

修改后保存文件，然后执行编译和上传操作。

## 串口监视器

测试程序会通过串口输出调试信息。要查看这些信息：

1. 在VS Code中，点击底部状态栏的"Serial Monitor"按钮
2. 或者使用外部串口工具，波特率设置为9600

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

### 硬件连接

参考`src/Utils/Config.h`文件中的引脚定义，确保硬件连接正确。

## 贡献

欢迎提交问题报告和改进建议！

## 许可

[添加适当的许可信息]


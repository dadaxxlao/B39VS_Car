# 传感器测试程序集

这个目录包含了用于测试智能小车各种传感器的测试程序。这些测试程序可以帮助您验证传感器的工作状态、调试传感器问题以及校准传感器参数。

## 可用的测试程序

### 1. 超声波传感器测试 (TestUltrasonic.cpp)

测试超声波测距传感器的功能，包括距离测量和障碍物检测。

使用方法：
- 上传程序后打开串口监视器
- 程序会持续显示测量的距离值和障碍物检测状态
- 可用于验证超声波传感器的准确性和响应速度

### 2. 红外线阵列传感器测试 (TestInfrared.cpp)

测试红外线阵列传感器的巡线功能，显示传感器读数和线位置。

使用方法：
- 上传程序后打开串口监视器
- 将小车放在黑线上，观察传感器数值变化
- 程序会显示各传感器的原始值和计算出的线位置

### 3. 颜色传感器测试 (TestColorSensor.cpp)

测试TCS34725颜色传感器的颜色识别功能。

使用方法：
- 上传程序后打开串口监视器
- 将传感器对准不同颜色的物体
- 程序会显示检测到的颜色以及RGB原始值

### 4. 传感器管理器综合测试 (TestSensorManager.cpp)

测试传感器管理器的功能，同时检测所有传感器的工作状态。

使用方法：
- 上传程序后打开串口监视器
- 程序会顺序显示所有传感器的数据
- 可用于验证多个传感器的协同工作情况

### 5. 传感器校准程序 (TestCalibration.cpp)

交互式校准工具，用于校准红外线传感器。

使用方法：
- 上传程序后打开串口监视器
- 按照程序提示进行红外传感器的校准流程
- 会经过传感器稳定等待、I2C校准命令发送、状态监控等步骤
- 校准后可实时监控传感器状态，验证校准效果

### 6. 巡线功能测试程序 (TestLineFollowing.cpp)

测试和调优小车的PID巡线控制功能。

使用方法：
- 上传程序后打开串口监视器，输入"help"查看可用命令
- 提供交互式命令接口，可以实时调整PID参数
- 支持的命令：
  - start - 开始巡线测试
  - stop - 停止巡线测试
  - kp[value] - 设置比例系数 (例如: kp0.5)
  - kd[value] - 设置微分系数 (例如: kd0.1)
  - speed[value] - 设置速度 (例如: speed180)
  - mode0 - 切换到PID调参模式
  - mode1 - 切换到完整巡线测试模式
- 实时输出传感器值和控制参数，方便调整和优化
- 通过该测试程序可以找到最佳的PID参数，提高巡线稳定性

### 7. 颜色传感器校准程序 (TestColorCalibration.cpp)

专用于颜色传感器的交互式校准工具。

使用方法：
- 上传程序后打开串口监视器
- 按照程序提示依次对不同颜色进行采样和校准
- 程序会记录每种颜色的RGB值范围和HSV值范围
- 完成校准后，程序会显示校准的阈值，可复制到代码中使用

### 8. HSV颜色识别测试 (TestColorHSV.cpp)

测试基于HSV颜色空间的颜色识别算法。

使用方法：
- 上传程序后打开串口监视器
- 将传感器对准不同颜色的物体
- 程序会实时显示RGB值和转换后的HSV值
- 可用于调整和验证HSV颜色识别参数
- 颜色检测模式下会同时显示RGB和HSV算法的识别结果对比

### 9. 路口检测测试 (TestMultipleJunctionDetection.cpp)

测试使用红外阵列传感器检测各种类型的路口。

使用方法：
- 上传程序后打开串口监视器
- 将小车放在不同路口类型（T型、十字、左右转等）上
- 程序会显示传感器状态、检测到的路口类型和路口计数
- 可用于开发和调试路口识别算法

### 10. 麦克纳姆轮运动测试 (TestMecanumMotion.cpp)

测试麦克纳姆轮小车的各种运动模式，包括前进、转向、平移和掉头等功能。

使用方法：
- 在编译前通过修改文件顶部的宏定义选择测试模式：
  ```cpp
  // 取消注释其中一个宏定义，注释其他的
  // #define TEST_FORWARD_MOTION    // 测试前进
  // #define TEST_LEFT_TURN         // 测试左转
  // #define TEST_RIGHT_TURN        // 测试右转
  // #define TEST_U_TURN            // 测试原地掉头
  // #define TEST_LATERAL_LEFT      // 测试左平移
  // #define TEST_LATERAL_RIGHT     // 测试右平移
  #define TEST_AUTO_SEQUENCE      // 测试自动序列（默认）
  
  // 转向角度校准测试
  // #define TEST_LEFT_TURN_90      // 测试左转90度校准
  // #define TEST_RIGHT_TURN_90     // 测试右转90度校准
  // #define TEST_U_TURN_180        // 测试原地掉头180度校准
  ```
- 编译上传后，小车将根据选择的模式执行测试
- 对于自动测试序列和单一动作测试，小车将独立运行而无需连接电脑
- 对于角度校准测试，需要连接串口监视器（波特率：9600）进行交互式校准

#### 自动测试序列和单一动作测试

- 小车会等待约3秒后开始执行测试，给您足够时间放置小车
- 通过内置LED的闪烁指示测试状态：
  - 前进：LED闪烁1次
  - 左转：LED闪烁2次
  - 右转：LED闪烁3次
  - 原地掉头：LED闪烁4次
  - 左平移：LED闪烁5次
  - 右平移：LED闪烁6次
  - 停止状态：LED常亮
  - 测试完成：LED慢闪
- 每个测试动作会自动执行一段时间后停止
- 测试完成后小车会自动停止

#### 转向角度校准测试

这些测试用于精确校准小车转向90度或180度所需的时间参数：

- **左转90度校准**（TEST_LEFT_TURN_90）：校准小车左转90度所需的精确时间
- **右转90度校准**（TEST_RIGHT_TURN_90）：校准小车右转90度所需的精确时间
- **原地掉头180度校准**（TEST_U_TURN_180）：校准小车原地掉头180度所需的精确时间

校准步骤：
1. 取消注释对应的校准测试宏定义（如 `TEST_LEFT_TURN_90`）
2. 编译并上传程序到小车
3. 打开串口监视器（波特率：9600）
4. 按照串口监视器中的提示操作：
   - 按下回车键开始测试
   - 观察小车转向后的角度
   - 使用 `+` 增加转向时间，使用 `-` 减少转向时间
   - 使用 `r` 重新测试当前参数
   - 当小车正好转到目标角度时，使用 `s` 保存该参数
5. 记录下最终校准的参数值，更新到代码中对应的常量中

注意事项：
- 校准时小车应放置在平整的表面上
- 确保小车电池电量充足，以保证测试结果的一致性
- 校准过程中保持小车速度不变，只调整时间参数
- 每次调整时间参数时建议以小增量（如50毫秒）进行
- 完成校准后，将得到的时间参数更新到代码中，使程序能够执行精确的角度转向

### 11. 路口巡线测试程序 (TestJunctionFollowing.cpp)

测试小车的路口检测和导航功能，包括路口识别、路口动作决策和路口历史记录。

使用方法：
- 上传程序后打开串口监视器，输入"help"查看可用命令
- 提供交互式命令接口，可以控制路口测试和查看历史记录
- 支持的命令：
  - start - 开始路口巡线测试
  - stop - 停止路口巡线测试
  - uturn - 手动执行掉头动作
  - kp[value] - 设置比例系数
  - kd[value] - 设置微分系数
  - reset - 重置路口计数
  - history - 显示最近10个路口的历史记录
- 实时输出检测到的路口类型和执行的动作
- 包含掉头逻辑，当检测到线路终点时自动掉头
- 通过设置冷却时间避免重复检测同一个路口

## 如何运行测试

1. 在PlatformIO中，修改platformio.ini文件中的`build_flags`参数，选择要测试的程序
   ```
   build_flags = -D TEST_COLOR_CALIBRATION 
   ```
共有：TEST_CALIBRATION、TEST_COLOR_CALIBRATION、TEST_COLOR_HSV、TEST_COLOR_SENSOR、TEST_INFRARED、TEST_JUNCTION_FOLLOWING、TEST_LINE_FOLLOWING、TEST_MECANUM_MOTION、TEST_MULTIPLE_JUNCTION_DETECTION、TEST_SENSOR_MANAGER、TEST_ULTRASONIC
2. 或者使用Arduino IDE时，打开相应测试文件并上传到开发板

3. 对于大多数测试，上传完成后打开串口监视器（波特率：9600）观察输出

4. 对于麦克纳姆轮测试，编辑文件中的宏定义选择测试模式，然后上传程序并为小车提供电源即可自动执行测试

## 注意事项

- 进行测试前，请确保相应的传感器已正确连接，并检查引脚定义是否与Config.h中的定义一致
- 颜色传感器校准时需要在稳定的光照条件下进行
- 红外线传感器校准需要使用实际将要使用的黑线和背景表面
- HSV色彩空间识别算法相比RGB算法，对光照条件变化更具鲁棒性
- 路口检测时，需要合适的路口样本以确保准确识别
- 校准完成后，应将获得的阈值参数更新到相应的传感器源代码中
- 麦克纳姆轮测试前，确保小车放置在平坦空旷的区域，有足够的空间进行测试动作
- 运行麦克纳姆轮测试时，上电后小车会等待约3秒再开始动作，此时请迅速将小车放在合适位置
- 测试时请确保小车放置在合适的位置，某些测试程序会立即启动电机
- 校准程序会修改传感器内部参数，请谨慎使用
- 测试中记录的参数可以用于调整主程序中的相应配置值
- 如需在主项目中禁用测试代码，请确保所有测试宏都被注释掉 
# B39VS_Car 配置文件使用指南

本文档提供了B39VS_Car项目的配置文件(`Config.h`)使用说明。配置文件定义了所有硬件连接、参数设置和系统状态，是开发和维护项目的重要参考。

## 引脚定义

### 麦克纳姆轮电机

| 电机位置 | PWM引脚 | 输入引脚1 | 输入引脚2 |
|----------|---------|-----------|-----------|
| 左前 (FL) | `MOTOR_FL_PWM` (2) | `MOTOR_FL_IN1` (3) | `MOTOR_FL_IN2` (4) |
| 右前 (FR) | `MOTOR_FR_PWM` (10) | `MOTOR_FR_IN1` (5) | `MOTOR_FR_IN2` (6) |
| 左后 (RL) | `MOTOR_RL_PWM` (11) | `MOTOR_RL_IN1` (7) | `MOTOR_RL_IN2` (8) |
| 右后 (RR) | `MOTOR_RR_PWM` (13) | `MOTOR_RR_IN1` (14) | `MOTOR_RR_IN2` (15) |

### 其他硬件

| 组件 | 引脚/地址 | 说明 |
|------|-----------|------|
| RGB LED灯环 | `RGB_LED_PIN` (6) | 连接RGB灯环的数据引脚 |
| LED灯环数量 | `RGB_LED_COUNT` (16) | LED灯珠数量 |
| 机械臂舵机 | `ARM_SERVO_PIN` (9) | 控制机械臂上下移动 |
| 抓取舵机 | `GRIPPER_SERVO_PIN` (10) | 控制夹爪开合 |
| 超声波传感器触发 | `ULTRASONIC_TRIG_PIN` (11) | 超声波发射引脚 |
| 超声波传感器回声 | `ULTRASONIC_ECHO_PIN` (12) | 超声波接收引脚 |

### 通信接口

| 组件 | RX引脚 | TX引脚 | 波特率 |
|------|--------|--------|-------|
| 蓝牙HM-10 | `BT_RX_PIN` (16) | `BT_TX_PIN` (17) | `BT_BAUD_RATE` (9600) |
| ESP32 | `ESP_RX_PIN` (18) | `ESP_TX_PIN` (19) | `ESP_BAUD_RATE` (115200) |

### I2C设备地址

| 设备 | 地址 |
|------|------|
| 红外阵列传感器 | `INFRARED_ARRAY_ADDR` (0x12) |
| 颜色传感器 | `COLOR_SENSOR_ADDR` (0x29) |

## 运动参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `MAX_SPEED` | 255 | 电机最大速度 |
| `TURN_SPEED` | 100 | 转向速度 |
| `FOLLOW_SPEED` | 100 | 巡线速度 |
| `SHARP_TURN_SPEED` | 200 | 急转弯速度 |
| `DEFAULT_SPEED` | 100 | 默认移动速度 |

## 阈值参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `NO_OBJECT_THRESHOLD` | 50 | 超声波无障碍物阈值(cm) |
| `GRAB_DISTANCE` | 10 | 可抓取距离(cm) |
| `LINE_THRESHOLD` | 500 | 红外线检测阈值 |

## 机械臂参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `ARM_UP_ANGLE` | 90 | 机械臂抬起角度 |
| `ARM_DOWN_ANGLE` | 0 | 机械臂放下角度 |
| `GRIPPER_OPEN_ANGLE` | 180 | 夹爪打开角度 |
| `GRIPPER_CLOSE_ANGLE` | 90 | 夹爪闭合角度 |
| `SERVO_DELAY` | 15 | 舵机移动延迟(ms) |

## 枚举类型

### 路口类型 (JunctionType)

| 类型 | 说明 |
|------|------|
| `NO_JUNCTION` | 无路口 |
| `T_LEFT` | 左旋T字路口 |
| `T_RIGHT` | 右旋T字路口 |
| `T_FORWARD` | 正T字路口（倒T） |
| `CROSS` | 十字路口 |
| `LEFT_TURN` | 左转弯 |
| `RIGHT_TURN` | 右转弯 |
| `END_OF_LINE` | 线路终点 |

### 颜色编码 (ColorCode)

| 编码 | 颜色 |
|------|------|
| `COLOR_UNKNOWN` | 未知颜色 |
| `COLOR_RED` (1) | 红色 |
| `COLOR_BLUE` (2) | 蓝色 |
| `COLOR_YELLOW` (3) | 黄色 |
| `COLOR_WHITE` (4) | 白色 |
| `COLOR_BLACK` (5) | 黑色 |

### 系统状态 (SystemState)

| 状态 | 说明 |
|------|------|
| `INITIALIZED` | 初始化状态 |
| `OBJECT_FIND` | 寻找物块状态 |
| `OBJECT_GRAB` | 抓取物块状态 |
| `OBJECT_LOCATE` | 定位放置区域状态 |
| `OBJECT_PLACING` | 放置物块状态 |
| `RETURN_BASE` | 返回基地状态 |
| `END` | 任务结束状态 |
| `ERROR_STATE` | 错误状态 |

### 定位子状态 (LocateSubState)

| 子状态 | 说明 |
|--------|------|
| `TURN_AROUND` | 掉头 |
| `COUNT_JUNCTIONS` | 统计经过的路口 |
| `SELECT_TARGET` | 选择目标路口 |

## 通信配置

| 配置 | 值 | 说明 |
|------|-----|------|
| `ENABLE_BLUETOOTH` | 0 | 蓝牙功能（0=禁用，1=启用） |
| `ENABLE_ESP` | 1 | ESP32通信功能（0=禁用，1=启用） |

## 使用示例

在项目中包含Config.h文件：

```cpp
#include "Utils/Config.h"

void setup() {
  // 设置电机引脚
  pinMode(MOTOR_FL_PWM, OUTPUT);
  pinMode(MOTOR_FL_IN1, OUTPUT);
  pinMode(MOTOR_FL_IN2, OUTPUT);
  
  // 使用速度参数
  analogWrite(MOTOR_FL_PWM, DEFAULT_SPEED);
  
  // 根据系统状态执行不同操作
  SystemState currentState = INITIALIZED;
  if (currentState == OBJECT_FIND) {
    // 寻找物块...
  }
}
```

## 修改配置

如需修改配置，请编辑`Config.h`文件中的相应定义，然后重新编译上传程序。修改配置时请注意：

1. 更改引脚定义需确保硬件正确连接
2. 修改运动参数需测试适合的速度范围
3. 调整阈值参数需根据实际传感器响应情况
4. 更改机械臂参数需校准舵机角度

**警告**：不当的配置修改可能导致硬件损坏或系统异常，请谨慎操作。 
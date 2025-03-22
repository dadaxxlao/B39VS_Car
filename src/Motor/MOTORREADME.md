# 如何使用MotionController控制麦克纳姆轮小车

麦克纳姆轮小车的`MotionController`类提供了完整的运动控制功能。以下是使用方法的详细讲解：

## 1. 初始化

首先需要创建一个`MotionController`对象并进行初始化：

```cpp
#include "Motor/MotionController.h"

MotionController motionCtrl;

void setup() {
  // 初始化运动控制器
  motionCtrl.init();
}
```

初始化过程会自动配置四个电机的引脚并将所有电机设置为停止状态。

## 2. 基本运动控制

`MotionController`提供了多种基本运动方法：

### 2.1 直线移动

```cpp
// 前进 (默认速度200)
motionCtrl.moveForward();
// 前进 (指定速度150)
motionCtrl.moveForward(150);

// 后退
motionCtrl.moveBackward();
// 后退 (指定速度150)
motionCtrl.moveBackward(150);
```

### 2.2 横向平移

麦克纳姆轮的独特优势是可以实现横向移动：

```cpp
// 左平移
motionCtrl.lateralLeft();
// 左平移 (指定速度180)
motionCtrl.lateralLeft(180);

// 右平移
motionCtrl.lateralRight();
// 右平移 (指定速度180)
motionCtrl.lateralRight(180);
```

### 2.3 转向控制

```cpp
// 左转 (前进+左转)
motionCtrl.turnLeft();
// 左转 (指定速度150)
motionCtrl.turnLeft(150);

// 右转 (前进+右转)
motionCtrl.turnRight();
// 右转 (指定速度150)
motionCtrl.turnRight(150);
```

### 2.4 原地旋转

```cpp
// 原地左转
motionCtrl.spinLeft();
// 原地左转 (指定速度170)
motionCtrl.spinLeft(170);

// 原地右转
motionCtrl.spinRight();
// 原地右转 (指定速度170)
motionCtrl.spinRight(170);
```

### 2.5 特殊动作

```cpp
// 原地掉头 (旋转180度)
motionCtrl.uTurn();
// 原地掉头 (指定速度200)
motionCtrl.uTurn(200);

// 紧急停止所有电机
motionCtrl.emergencyStop();
```

## 3. 高级控制

### 3.1 全向移动核心算法

如果需要更精确的控制，可以直接使用麦克纳姆轮运动学核心算法：

```cpp
// mecanumDrive(vx, vy, omega)
// vx: X方向速度分量 (-1.0 到 1.0)
// vy: Y方向速度分量 (-1.0 到 1.0)
// omega: 旋转分量 (-1.0 到 1.0)

// 对角线移动 (右前方45度)
motionCtrl.mecanumDrive(-0.7, 0.7, 0);

// 圆弧移动 (向前移动+缓慢右转)
motionCtrl.mecanumDrive(0, 1.0, -0.3);

// 旋转+平移组合动作
motionCtrl.mecanumDrive(0.5, 0.5, 0.5);
```

### 3.2 调整电机参数

```cpp
// 设置全局速度系数 (0-255)
motionCtrl.setSpeedFactor(180);

// 设置各电机补偿系数，用于平衡各电机差异
// 参数顺序：左前(FL), 右前(FR), 左后(RL), 右后(RR)
motionCtrl.setMotorCompensation(0.85, 1.0, 1.0, 0.8);
```

## 4. 实际应用示例

### 4.1 基本移动序列

```cpp
void performMovementSequence() {
  // 前进2秒
  motionCtrl.moveForward(150);
  delay(2000);
  
  // 右转90度
  motionCtrl.spinRight(180);
  delay(800);  // 延时需根据实际测试调整
  
  // 前进1秒
  motionCtrl.moveForward(150);
  delay(1000);
  
  // 停止
  motionCtrl.emergencyStop();
}
```

### 4.2 复杂轨迹移动

```cpp
void performComplexTrajectory() {
  // 沿矩形路径移动
  
  // 前进
  motionCtrl.moveForward(150);
  delay(1000);
  
  // 右平移
  motionCtrl.lateralRight(150);
  delay(1000);
  
  // 后退
  motionCtrl.moveBackward(150);
  delay(1000);
  
  // 左平移
  motionCtrl.lateralLeft(150);
  delay(1000);
  
  // 停止
  motionCtrl.emergencyStop();
}
```

### 4.3 原地掉头实例

```cpp
void navigateWithUTurn() {
  // 前进到目标点
  motionCtrl.moveForward(180);
  delay(2000);
  
  // 原地掉头
  motionCtrl.uTurn(200);
  
  // 回到起点
  motionCtrl.moveForward(180);
  delay(2000);
  
  // 停止
  motionCtrl.emergencyStop();
}
```

## 注意事项

1. 所有运动命令都是非阻塞的，需要自行添加延时或条件控制运动时间
2. 速度参数范围为0-255，对应PWM输出
3. 电机补偿系数用于调整各电机的实际输出功率，以弥补电机差异
4. 在使用麦克纳姆轮时，平衡的电机输出对精确移动至关重要

通过这些API，您可以完全控制麦克纳姆轮小车的各种运动方式，包括常规的前进、后退、转向以及全向移动特有的横向平移和自由角度移动。

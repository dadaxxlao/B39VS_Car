#ifndef ROBOTIC_ARM_H
#define ROBOTIC_ARM_H

#include <Arduino.h>
#include <Servo.h>
#include "../Utils/Config.h"
#include "../Sensor/Ultrasonic.h"

class RoboticArm {
private:
    Servo myservos[3];     // 三个舵机：底部、中间和夹爪
    UltrasonicSensor ultrasonic; // 超声波传感器对象
    
    // 舵机引脚
    static const byte SERVO_NUM = 3;
    byte servo_pin[SERVO_NUM] = {44, 45, 46}; 
    
    // 舵机初始位置（微秒）
    int servo_initial_pos[SERVO_NUM] = {1500, 900, 600};
    
    // 当前位置
    int currentPositions[SERVO_NUM];
    
    bool isCalibrated;  // 是否已校准
    
    // 舵机控制函数
    void servoControl(int angle_base, int angle_arm, int angle_claw);
    
public:
    RoboticArm();
    
    // 初始化机械臂
    void init();
    
    // 校准机械臂
    void calibrate();
    
    // 夹取物体
    bool grab();
    
    // 释放物体
    void release();
    
    // 机械臂归位
    void reset();
    
    // 以下是对应TestStateMachineComplete.cpp中使用的接口
    // 打开夹爪
    void openGripper();
    
    // 关闭夹爪
    void closeGripper();
    
    // 移动机械臂向上
    void moveUp();
    
    // 移动机械臂向下
    void moveDown();
    
    // 机械臂放置到物料盒位置
    void moveToBox();
    
    // 机械臂调整角度
    void adjustArm(int baseAngle, int armAngle, int clawAngle);
    
    // 判断机械臂是否处于运动中
    bool isMoving() const;

    // 检查是否满足抓取条件
    bool checkGrabCondition();
};

#endif // ROBOTIC_ARM_H 
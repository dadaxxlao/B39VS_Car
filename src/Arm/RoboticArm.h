#ifndef ROBOTIC_ARM_H
#define ROBOTIC_ARM_H

#include <Arduino.h>
#include <Servo.h>
#include "../Utils/Config.h"

class RoboticArm {
private:
    Servo armServo;     // 控制机械臂升降的舵机
    Servo gripperServo; // 控制机械爪开合的舵机
    
    int armPosition;    // 当前机械臂角度
    int gripperPosition; // 当前机械爪角度
    
    bool isCalibrated;  // 是否已校准
    
public:
    RoboticArm();
    
    // 初始化机械臂
    void init();
    
    // 校准机械臂
    void calibrate();
    
    // 设置机械臂角度
    void setPosition(float angle);
    
    // 抓取物体
    bool grab();
    
    // 释放物体
    void release();
    
    // 机械臂上升
    void raise();
    
    // 机械臂下降
    void lower();
    
    // 获取当前机械臂角度
    int getArmPosition() const;
    
    // 获取当前机械爪角度
    int getGripperPosition() const;
    
    // 判断机械臂是否处于运动中
    bool isMoving() const;
};

#endif // ROBOTIC_ARM_H 
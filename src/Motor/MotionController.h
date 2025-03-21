#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include "MotorDriver.h"
#include "../Utils/Config.h"

class MotionController {
private:
    MotorDriver leftMotor;   // 左电机
    MotorDriver rightMotor;  // 右电机
    
    // PID控制参数
    float kP, kI, kD;
    float integral;
    float lastError;
    
public:
    MotionController();
    
    // 初始化运动控制器
    void init();
    
    // 设置PID参数
    void setPID(float p, float i, float d);
    
    // 前进
    void moveForward(int speed = FOLLOW_SPEED);
    
    // 后退
    void moveBackward(int speed = FOLLOW_SPEED);
    
    // 左转
    void turnLeft(int speed = TURN_SPEED);
    
    // 右转
    void turnRight(int speed = TURN_SPEED);
    
    // 原地左转
    void spinLeft(int speed = TURN_SPEED);
    
    // 原地右转
    void spinRight(int speed = TURN_SPEED);
    
    // 停止
    void stop();
    
    // 巡线控制
    void followLine(int position);
    
    // U型转弯
    void uTurn();
};

#endif // MOTION_CONTROLLER_H 
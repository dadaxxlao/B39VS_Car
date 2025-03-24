#ifndef MOTION_CONTROLLER_H
#define MOTION_CONTROLLER_H

#include "MotorDriver.h"
#include "../Utils/Config.h"

class MotionController {
private:
    MotorDriver motorFL;   // 左前轮 (FL)
    MotorDriver motorFR;   // 右前轮 (FR)
    MotorDriver motorRL;   // 左后轮 (RL)
    MotorDriver motorRR;   // 右后轮 (RR)
    
    // 速度系数 (0-255)
    int speedFactor;
    
    // 电机补偿系数
    float motorCompensation[4];
    
    // 设置单个电机状态
    void setMotorState(MotorDriver &motor, float ratio, int motorIndex);
    
public:
    MotionController();
    
    // 初始化运动控制器
    void init();
    
    // 麦克纳姆轮全向移动核心算法
    void mecanumDrive(float vx, float vy, float omega);
    
    // 前进
    void moveForward(int speed = DEFAULT_SPEED);
    
    // 后退
    void moveBackward(int speed = DEFAULT_SPEED);
    
    // 左平移
    void lateralLeft(int speed = DEFAULT_SPEED);
    
    // 右平移
    void lateralRight(int speed = DEFAULT_SPEED);
    
    // 左转
    void turnLeft(int speed = DEFAULT_SPEED);
    
    // 右转
    void turnRight(int speed = DEFAULT_SPEED);
    
    // 原地左转
    void spinLeft(int speed = TURN_SPEED);
    
    // 原地右转
    void spinRight(int speed = TURN_SPEED);
    
    // 原地掉头（旋转180度）
    void uTurn(int speed = TURN_SPEED);
    
    // 紧急停止
    void emergencyStop();
    
    // 设置速度系数
    void setSpeedFactor(int speed);
    
    // 设置电机补偿系数
    void setMotorCompensation(float fl, float fr, float rl, float rr);
};

#endif // MOTION_CONTROLLER_H 
#include "MotionController.h"
#include "../Utils/Logger.h"

MotionController::MotionController() : kP(0.5), kI(0.0), kD(0.1), integral(0.0), lastError(0.0) {
}

void MotionController::init() {
    // 初始化左右电机
    leftMotor.init(LEFT_MOTOR_PIN1, LEFT_MOTOR_PIN2);
    rightMotor.init(RIGHT_MOTOR_PIN1, RIGHT_MOTOR_PIN2);
    
    // 停止电机
    stop();
    
    Logger::info("运动控制器初始化完成");
}

void MotionController::setPID(float p, float i, float d) {
    kP = p;
    kI = i;
    kD = d;
    
    // 重置PID状态
    integral = 0.0;
    lastError = 0.0;
    
    Logger::info("设置PID参数: P=%.2f, I=%.2f, D=%.2f", p, i, d);
}

void MotionController::moveForward(int speed) {
    leftMotor.forward(speed);
    rightMotor.forward(speed);
}

void MotionController::moveBackward(int speed) {
    leftMotor.backward(speed);
    rightMotor.backward(speed);
}

void MotionController::turnLeft(int speed) {
    // 左转弯（左电机减速，右电机保持）
    leftMotor.forward(speed / 2);
    rightMotor.forward(speed);
}

void MotionController::turnRight(int speed) {
    // 右转弯（右电机减速，左电机保持）
    leftMotor.forward(speed);
    rightMotor.forward(speed / 2);
}

void MotionController::spinLeft(int speed) {
    // 原地左转（左电机后退，右电机前进）
    leftMotor.backward(speed);
    rightMotor.forward(speed);
}

void MotionController::spinRight(int speed) {
    // 原地右转（右电机后退，左电机前进）
    leftMotor.forward(speed);
    rightMotor.backward(speed);
}

void MotionController::stop() {
    leftMotor.stop();
    rightMotor.stop();
}

void MotionController::followLine(int position) {
    // PID巡线控制算法
    
    // 计算误差（position范围为-100到100，0为中心）
    float error = (float)position;
    
    // 计算积分项
    integral += error;
    
    // 限制积分项，防止积分饱和
    if (integral > 100.0) integral = 100.0;
    if (integral < -100.0) integral = -100.0;
    
    // 计算微分项
    float derivative = error - lastError;
    lastError = error;
    
    // 计算PID输出
    float output = kP * error + kI * integral + kD * derivative;
    
    // 限制输出范围
    int speedDiff = (int)constrain(output, -MAX_SPEED, MAX_SPEED);
    
    // 设置电机速度
    int baseSpeed = FOLLOW_SPEED;
    int leftSpeed = baseSpeed - speedDiff;
    int rightSpeed = baseSpeed + speedDiff;
    
    // 限制速度范围
    leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
    rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);
    
    // 更新电机速度
    leftMotor.setSpeed(leftSpeed);
    rightMotor.setSpeed(rightSpeed);
}

void MotionController::uTurn() {
    // 执行U型转弯
    
    // 先停止
    stop();
    delay(100);
    
    // 原地旋转180度
    spinLeft(SHARP_TURN_SPEED);
    
    // 等待足够时间完成旋转（可以使用陀螺仪或编码器进行更精确的控制）
    delay(1000);
    
    // 停止
    stop();
} 
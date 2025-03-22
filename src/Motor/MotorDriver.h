#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

class MotorDriver {
private:
    uint8_t pwmPin;   // PWM控制引脚
    uint8_t in1Pin;   // 方向控制引脚1
    uint8_t in2Pin;   // 方向控制引脚2
    int currentSpeed; // 当前速度(-255到255)
    
public:
    MotorDriver();
    
    // 初始化电机引脚
    void init(uint8_t pwm, uint8_t in1, uint8_t in2);
    
    // 设置电机速度和方向
    void setMotor(int speed, bool direction);
    
    // 获取当前速度
    int getSpeed() const;
    
    // 停止电机
    void stopMotor();
    
    // 设置电机速度(-255到255)
    // 正值表示正向旋转，负值表示反向旋转
    void setSpeed(int speed);
    
    // 正向旋转
    void forward(int speed);
    
    // 反向旋转
    void backward(int speed);
};

#endif // MOTOR_DRIVER_H 
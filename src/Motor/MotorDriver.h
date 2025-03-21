#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

class MotorDriver {
private:
    uint8_t pin1;    // 电机控制引脚1
    uint8_t pin2;    // 电机控制引脚2
    int currentSpeed; // 当前速度(-255到255)
    
public:
    MotorDriver();
    
    // 初始化电机引脚
    void init(uint8_t pin1, uint8_t pin2);
    
    // 设置电机速度和方向(-255到255)
    // 正值表示正向旋转，负值表示反向旋转，0表示停止
    void setSpeed(int speed);
    
    // 获取当前速度
    int getSpeed() const;
    
    // 停止电机
    void stop();
    
    // 正向旋转
    void forward(int speed);
    
    // 反向旋转
    void backward(int speed);
};

#endif // MOTOR_DRIVER_H 
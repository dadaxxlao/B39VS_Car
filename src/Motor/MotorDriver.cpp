#include "MotorDriver.h"
#include "../Utils/Logger.h"

MotorDriver::MotorDriver() : pin1(0), pin2(0), currentSpeed(0) {
}

void MotorDriver::init(uint8_t p1, uint8_t p2) {
    pin1 = p1;
    pin2 = p2;
    
    // 设置引脚为输出模式
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    
    // 初始化为停止状态
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
    
    Logger::debug("电机初始化 - 引脚 %d 和 %d", pin1, pin2);
}

void MotorDriver::setSpeed(int speed) {
    // 限制速度范围为-255到255
    speed = constrain(speed, -255, 255);
    currentSpeed = speed;
    
    if (speed > 0) {
        // 正向
        analogWrite(pin1, speed);
        analogWrite(pin2, 0);
    } else if (speed < 0) {
        // 反向
        analogWrite(pin1, 0);
        analogWrite(pin2, -speed);
    } else {
        // 停止
        analogWrite(pin1, 0);
        analogWrite(pin2, 0);
    }
}

int MotorDriver::getSpeed() const {
    return currentSpeed;
}

void MotorDriver::stop() {
    setSpeed(0);
}

void MotorDriver::forward(int speed) {
    setSpeed(abs(speed));
}

void MotorDriver::backward(int speed) {
    setSpeed(-abs(speed));
} 
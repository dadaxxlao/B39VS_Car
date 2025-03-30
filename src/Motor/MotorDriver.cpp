#include "MotorDriver.h"
#include "../Utils/Logger.h"

MotorDriver::MotorDriver() : pwmPin(0), in1Pin(0), in2Pin(0), currentSpeed(0) {
}

void MotorDriver::init(uint8_t pwm, uint8_t in1, uint8_t in2) {
    pwmPin = pwm;
    in1Pin = in1;
    in2Pin = in2;
    
    // 设置引脚为输出模式
    pinMode(pwmPin, OUTPUT);
    pinMode(in1Pin, OUTPUT);
    pinMode(in2Pin, OUTPUT);
    
    // 初始化为停止状态
    analogWrite(pwmPin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    
    Logger::debug("MotorDriver", "电机初始化 - 引脚 PWM:%d, IN1:%d, IN2:%d", pwmPin, in1Pin, in2Pin);
}

void MotorDriver::setMotor(int speed, bool direction) {
    // 限制速度范围为0-255
    speed = constrain(abs(speed), 0, 255);
    currentSpeed = direction ? speed : -speed;
    
    // 设置PWM和方向
    analogWrite(pwmPin, speed);
    digitalWrite(in1Pin, direction ? HIGH : LOW);
    digitalWrite(in2Pin, direction ? LOW : HIGH);
}

int MotorDriver::getSpeed() const {
    return currentSpeed;
}

void MotorDriver::stopMotor() {
    analogWrite(pwmPin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    currentSpeed = 0;
}

void MotorDriver::setSpeed(int speed) {
    // 限制速度范围为-255到255
    speed = constrain(speed, -255, 255);
    currentSpeed = speed;
    
    if (speed > 0) {
        // 正向
        setMotor(speed, true);
    } else if (speed < 0) {
        // 反向
        setMotor(-speed, false);
    } else {
        // 停止
        stopMotor();
    }
}

void MotorDriver::forward(int speed) {
    setSpeed(abs(speed));
}

void MotorDriver::backward(int speed) {
    setSpeed(-abs(speed));
} 
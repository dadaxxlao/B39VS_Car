#include "MotionController.h"
#include "../Utils/Logger.h"

MotionController::MotionController() : speedFactor(DEFAULT_SPEED) {
    // 设置默认的电机补偿系数
    motorCompensation[0] = 1.0;  // FL
    motorCompensation[1] = 1.0;  // FR
    motorCompensation[2] = 1.0;  // RL
    motorCompensation[3] = 1.0;  // RR
}

void MotionController::init() {
    // 初始化四个电机
    motorFL.init(MOTOR_FL_PWM, MOTOR_FL_IN1, MOTOR_FL_IN2);
    motorFR.init(MOTOR_FR_PWM, MOTOR_FR_IN1, MOTOR_FR_IN2);
    motorRL.init(MOTOR_RL_PWM, MOTOR_RL_IN1, MOTOR_RL_IN2);
    motorRR.init(MOTOR_RR_PWM, MOTOR_RR_IN1, MOTOR_RR_IN2);
    
    // 停止所有电机
    emergencyStop();
    
    Logger::info("MotionCtrl", "麦克纳姆轮运动控制器初始化完成");
}

void MotionController::setMotorState(MotorDriver &motor, float ratio, int motorIndex) {
    int actualSpeed = abs(ratio) * speedFactor * motorCompensation[motorIndex];
    bool direction = ratio > 0 ? true : false;
    motor.setMotor(actualSpeed, direction);
}

void MotionController::mecanumDrive(float vx, float vy, float omega) {
    // 运动学模型计算
    float fl = -vx - vy - omega;
    float fr = -vx + vy - omega;
    float rl = +vx - vy - omega;
    float rr = +vx + vy - omega;

    // 归一化处理
    float maxVal = max(max(abs(fl), abs(fr)), max(abs(rl), abs(rr)));
    if(maxVal > 1) {
        fl /= maxVal;
        fr /= maxVal;
        rl /= maxVal;
        rr /= maxVal;
    }

    // 设置电机状态 - 与示例代码保持一致的引脚映射
    setMotorState(motorFL, fl, 0);  // FL
    setMotorState(motorFR, fr, 1);  // FR
    setMotorState(motorRL, rl, 2);  // RL
    setMotorState(motorRR, rr, 3);  // RR
}

void MotionController::moveForward(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 1.0, 0);  // +Y方向移动
    speedFactor = originalSpeed;
}

void MotionController::moveBackward(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, -1.0, 0);  // -Y方向移动
    speedFactor = originalSpeed;
}

void MotionController::lateralLeft(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(1.0, 0, 0);  // +X方向平移
    speedFactor = originalSpeed;
}

void MotionController::lateralRight(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(-1.0, 0, 0); // -X方向平移
    speedFactor = originalSpeed;
}

void MotionController::turnLeft(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 0.7, - 0.3);  // 左前移动+左旋转
    speedFactor = originalSpeed;
}

void MotionController::turnRight(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 0.7, 0.3);  // 右前移动+右旋转
    speedFactor = originalSpeed;
}

void MotionController::spinLeft(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 0, -1.0);  // 原地左旋转
    speedFactor = originalSpeed;
}

void MotionController::spinRight(int speed) {
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 0, 1.0);  // 原地右旋转
    speedFactor = originalSpeed;
}

void MotionController::uTurn(int speed) {
    // 执行原地掉头（旋转180度）
    
    // 先停止
    emergencyStop();
    delay(100);
    
    // 原地旋转180度
    int originalSpeed = speedFactor;
    speedFactor = speed;
    mecanumDrive(0, 0, 1.0);  // 原地左旋转
    delay(1000);  // 等待足够时间完成旋转（可根据实际情况调整）
    emergencyStop();
    speedFactor = originalSpeed;
}

void MotionController::emergencyStop() {
    motorFL.stopMotor();
    motorFR.stopMotor();
    motorRL.stopMotor();
    motorRR.stopMotor();
}

void MotionController::setSpeedFactor(int speed) {
    speedFactor = constrain(speed, 0, 255);
}

void MotionController::setMotorCompensation(float fl, float fr, float rl, float rr) {
    motorCompensation[0] = fl;
    motorCompensation[1] = fr;
    motorCompensation[2] = rl;
    motorCompensation[3] = rr;
    
    Logger::info("MotionCtrl", "设置电机补偿系数: FL=%.2f, FR=%.2f, RL=%.2f, RR=%.2f", fl, fr, rl, rr);
} 
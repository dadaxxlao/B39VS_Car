#include "RoboticArm.h"
#include "../Utils/Logger.h"

RoboticArm::RoboticArm() : armPosition(0), gripperPosition(0), isCalibrated(false) {
}

void RoboticArm::init() {
    // 初始化舵机
    armServo.attach(ARM_SERVO_PIN);
    gripperServo.attach(GRIPPER_SERVO_PIN);
    
    Logger::info("RoboticArm", "机械臂初始化完成");
}

void RoboticArm::calibrate() {
    // 机械臂校准程序
    Logger::info("RoboticArm", "开始校准机械臂");
    
    // 先将机械臂移动到安全位置
    setPosition(ARM_UP_ANGLE);
    delay(500);
    
    // 打开机械爪
    gripperServo.write(GRIPPER_OPEN_ANGLE);
    gripperPosition = GRIPPER_OPEN_ANGLE;
    delay(500);
    
    // 标记为已校准
    isCalibrated = true;
    
    Logger::info("RoboticArm", "机械臂校准完成");
}

void RoboticArm::setPosition(float angle) {
    // 检查角度范围
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    
    // 平滑移动到目标位置
    int targetPos = (int)angle;
    int startPos = armPosition;
    int steps = abs(targetPos - startPos);
    
    for (int i = 0; i <= steps; i++) {
        int pos = startPos + (targetPos - startPos) * i / steps;
        armServo.write(pos);
        armPosition = pos;
        delay(SERVO_DELAY);
    }
    
    Logger::debug("RoboticArm", "机械臂移动到位置: %d", targetPos);
}

bool RoboticArm::grab() {
    if (!isCalibrated) {
        Logger::warning("RoboticArm", "机械臂未校准，无法执行抓取");
        return false;
    }
    
    // 执行抓取动作序列
    Logger::info("RoboticArm", "执行抓取动作");
    
    // 1. 先确保机械爪打开
    gripperServo.write(GRIPPER_OPEN_ANGLE);
    gripperPosition = GRIPPER_OPEN_ANGLE;
    delay(300);
    
    // 2. 放下机械臂
    setPosition(ARM_DOWN_ANGLE);
    delay(500);
    
    // 3. 闭合机械爪
    gripperServo.write(GRIPPER_CLOSE_ANGLE);
    gripperPosition = GRIPPER_CLOSE_ANGLE;
    delay(500);
    
    // 4. 抬起机械臂
    setPosition(ARM_UP_ANGLE);
    
    // 返回抓取成功
    return true;
}

void RoboticArm::release() {
    if (!isCalibrated) {
        Logger::warning("RoboticArm", "机械臂未校准，无法执行释放");
        return;
    }
    
    // 执行释放动作序列
    Logger::info("RoboticArm", "执行释放动作");
    
    // 1. 放下机械臂
    setPosition(ARM_DOWN_ANGLE);
    delay(500);
    
    // 2. 打开机械爪
    gripperServo.write(GRIPPER_OPEN_ANGLE);
    gripperPosition = GRIPPER_OPEN_ANGLE;
    delay(500);
    
    // 3. 抬起机械臂
    setPosition(ARM_UP_ANGLE);
}

void RoboticArm::raise() {
    setPosition(ARM_UP_ANGLE);
}

void RoboticArm::lower() {
    setPosition(ARM_DOWN_ANGLE);
}

int RoboticArm::getArmPosition() const {
    return armPosition;
}

int RoboticArm::getGripperPosition() const {
    return gripperPosition;
}

bool RoboticArm::isMoving() const {
    // 在实际应用中，可以根据舵机的反馈来判断机械臂是否在运动
    // 这里简化处理，直接返回false
    return false;
} 
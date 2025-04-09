#include "RoboticArm.h"
#include "../Utils/Logger.h"

RoboticArm::RoboticArm() : isCalibrated(false) {
    // 初始化当前位置为初始位置
    for (int i = 0; i < SERVO_NUM; i++) {
        currentPositions[i] = servo_initial_pos[i];
    }
    // 修改爪子初始角度
    servo_initial_pos[2] = 600;
}

void RoboticArm::init() {
    // 初始化舵机
    for (byte i = 0; i < SERVO_NUM; i++) {
        myservos[i].attach(servo_pin[i]);
        myservos[i].writeMicroseconds(servo_initial_pos[i]);
        currentPositions[i] = servo_initial_pos[i];
    }
    
    // 初始化超声波传感器
    ultrasonic.init();
    
    Logger::info("RoboticArm", "机械臂初始化完成");
}

void RoboticArm::calibrate() {
    // 机械臂校准程序
    Logger::info("RoboticArm", "开始校准机械臂");
    
    // 回到初始位置，爪子角度为600
    servoControl(1500, 900, 600);
    delay(1000);
    
    // 标记为已校准
    isCalibrated = true;
    
    Logger::info("RoboticArm", "机械臂校准完成");
}

void RoboticArm::servoControl(int angle_base, int angle_arm, int angle_claw) {
    char cmd_return[64];
    sprintf(cmd_return, "#000P%04dT2000!#001P%04dT2000!#002P%04dT2000!",
            angle_base, angle_arm, angle_claw);
    Serial.println((char *)cmd_return);

    myservos[0].writeMicroseconds(angle_base);
    myservos[1].writeMicroseconds(angle_arm);
    myservos[2].writeMicroseconds(angle_claw);
    
    // 更新当前位置
    currentPositions[0] = angle_base;
    currentPositions[1] = angle_arm;
    currentPositions[2] = angle_claw;
    
    // 记录日志
    Logger::debug("RoboticArm", "舵机位置: 底部=%d, 中间=%d, 夹爪=%d", 
                 angle_base, angle_arm, angle_claw);
}

bool RoboticArm::checkGrabCondition() {
    float distance = ultrasonic.getDistance();
    // 实时显示距离值
    Serial2.print("超声波距离: ");
    Serial2.print(distance);
    Serial2.println(" cm");
    
    // 当距离在12-13.5cm范围内时返回true
    return (distance >= 12.5f && distance <= 13.5f);
}

bool RoboticArm::grab() {
    if (!isCalibrated) {
        Logger::warning("RoboticArm", "机械臂未校准，无法执行抓取");
        return false;
    }
    
    // 检查是否满足抓取条件
    if (!checkGrabCondition()) {
        Logger::warning("RoboticArm", "不满足抓取条件，距离不在12-13.5cm范围内");
        return false;
    }
    
    // 执行抓取动作序列
    Logger::info("RoboticArm", "执行抓取动作");
    
    // 旋转到抓取位置并打开夹爪
    servoControl(2150, 450, 150);
    delay(2000);
    
    // 闭合夹爪抓取物体 - 使用900作为闭合值
    servoControl(2150, 450, 900);
    delay(2000);
    
    // 抬起物体 - 保持夹爪闭合
    servoControl(1500, 1600, 900);
    delay(2000);
    
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
    
    // 放下机械臂到放置区
    servoControl(2150, 450, 900); // 确保夹爪仍然闭合
    delay(2000);
    
    // 打开夹爪释放物体
    servoControl(2150, 450, 150);
    delay(2000);
    
    // 回到安全位置
    moveUp();
}

void RoboticArm::openGripper() {
    Logger::info("RoboticArm", "打开夹爪");
    servoControl(currentPositions[0], currentPositions[1], 150);
    delay(1000);
}

void RoboticArm::closeGripper() {
    Logger::info("RoboticArm", "关闭夹爪");
    servoControl(currentPositions[0], currentPositions[1], 900);
    delay(1000);
}

void RoboticArm::moveUp() {
    Logger::info("RoboticArm", "机械臂上升到安全位置");
    servoControl(1500, 1600, currentPositions[2]);
    delay(2000);
}

void RoboticArm::moveDown() {
    Logger::info("RoboticArm", "机械臂下降到抓取位置");
    servoControl(2150, 450, currentPositions[2]);
    delay(2000);
}

void RoboticArm::moveToBox() {
    Logger::info("RoboticArm", "机械臂移动到物料盒位置");
    servoControl(1500, 1600, currentPositions[2]);
    delay(2000);
}

void RoboticArm::reset() {
    Logger::info("RoboticArm", "机械臂复位");
    
    // 复位到默认位置，爪子角度为600
    servoControl(1500, 900, 600);
    delay(2000);
    
    // 重新校准
    calibrate();
}

void RoboticArm::adjustArm(int baseAngle, int armAngle, int clawAngle) {
    Logger::info("RoboticArm", "调整机械臂位置");
    servoControl(baseAngle, armAngle, clawAngle);
    delay(2000);
}

bool RoboticArm::isMoving() const {
    // 在实际应用中，可以根据舵机的反馈来判断机械臂是否在运动
    // 这里简化处理，直接返回false
    return false;
} 
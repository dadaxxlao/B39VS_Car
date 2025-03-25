#include "LineFollower.h"

// 构造函数
LineFollower::LineFollower(InfraredArray& infraredSensor, MotionController& motionController)
    : m_infraredSensor(infraredSensor)
    , m_motionController(motionController)
    , m_Kp(1.0)
    , m_Ki(0.0)
    , m_Kd(1.0)
    , m_lastError(0)
    , m_integral(0)
    , m_lineLastDetectedTime(0)
    , m_maxLineLostTime(2000)
    , m_lastForwardSpeed(0.3)
    , m_lastTurnAmount(0.0)
    , m_baseSpeed(FOLLOW_SPEED)
    , m_leftTurnGain(1.0)  // 新增左转增益参数，默认1.0倍
    , m_rightTurnGain(1.0) // 新增右转增益参数，默认1.0倍
{
}

// 初始化函数
void LineFollower::init() {
    // 重置所有状态
    reset();
    Logger::info("巡线控制器已初始化");
}

// 设置PID参数
void LineFollower::setPIDParams(float Kp, float Ki, float Kd) {
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    // 重置PID状态，防止突变
    m_integral = 0;
    m_lastError = 0;
    Logger::debug("已设置PID参数: Kp=%.2f, Ki=%.2f, Kd=%.2f", m_Kp, m_Ki, m_Kd);
}

// 设置丢线处理参数
void LineFollower::setLineLostParams(unsigned long maxLineLostTime) {
    m_maxLineLostTime = maxLineLostTime;
    Logger::debug("已设置最长允许丢线时间: %lu ms", m_maxLineLostTime);
}

// 设置基础速度
void LineFollower::setBaseSpeed(int speed) {
    m_baseSpeed = speed;
    Logger::debug("已设置基础速度: %d", m_baseSpeed);
}

// 设置转向增益
void LineFollower::setTurnGain(float leftGain, float rightGain) {
    m_leftTurnGain = leftGain;
    m_rightTurnGain = rightGain;
    Logger::debug("已设置转向增益: 左转=%.2f, 右转=%.2f", m_leftTurnGain, m_rightTurnGain);
}

// 重置状态
void LineFollower::reset() {
    m_lastError = 0;
    m_integral = 0;
    m_lineLastDetectedTime = 0;
    m_lastForwardSpeed = 0.3;
    m_lastTurnAmount = 0.0;
}

// 巡线函数 - 更新机器人移动
void LineFollower::update() {
    // 更新传感器数据
    m_infraredSensor.update();
    
    // 获取线位置 (-100 到 100)
    int position = m_infraredSensor.getLinePosition();
    
    // 检测是否有线
    bool lineDetected = m_infraredSensor.isLineDetected();
    
    // 当前时间
    unsigned long currentTime = millis();
    
    // 打印传感器数值便于调试
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    Logger::debug("传感器值: [%d,%d,%d,%d,%d,%d,%d,%d], 位置: %d", 
                 sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3],
                 sensorValues[4], sensorValues[5], sensorValues[6], sensorValues[7],
                 position);
    
    if (!lineDetected) {
        // 如果没有检测到线
        if (m_lineLastDetectedTime == 0) {
            // 首次丢失线，记录时间
            m_lineLastDetectedTime = currentTime;
            
            // 使用最后的控制量继续行驶
            Logger::info("暂时未检测到线，继续按最后方向行驶");
            // 使用高级运动控制函数，这些函数内部会设置speedFactor
            if (abs(m_lastTurnAmount) > 0.2) {
                // 根据转向量的符号决定转向方向，修正逻辑
                if (m_lastTurnAmount > 0) {
                    m_motionController.spinRight(m_baseSpeed);  // 上次偏右，所以右转向线方向
                } else {
                    m_motionController.spinLeft(m_baseSpeed); // 上次偏左，所以左转向线方向
                }
            } else {
                m_motionController.moveForward(m_baseSpeed);
            }
            return;
        } else if (currentTime - m_lineLastDetectedTime > m_maxLineLostTime) {
            // 超过最长允许丢线时间，停车
            m_motionController.emergencyStop();
            Logger::info("长时间(%lu毫秒)未检测到线，已停止", m_maxLineLostTime);
            return;
        } else {
            // 在允许的丢线时间内，继续按最后方向行驶
            // 使用高级运动控制函数，这些函数内部会设置speedFactor
            if (abs(m_lastTurnAmount) > 0.2) {
                // 根据转向量的符号决定转向方向，修正逻辑
                if (m_lastTurnAmount > 0) {
                    m_motionController.spinRight(m_baseSpeed);  // 上次偏右，所以右转向线方向
                } else {
                    m_motionController.spinLeft(m_baseSpeed); // 上次偏左，所以左转向线方向
                }
            } else {
                m_motionController.moveForward(m_baseSpeed);
            }
            return;
        }
    } else {
        // 检测到线，重置丢线时间
        m_lineLastDetectedTime = 0;
    }
    
    // 计算误差 (position就是误差，因为目标位置是0)
    int error = position;
    m_integral = m_integral + error;
    m_integral = constrain(m_integral, -100, 100);  // 防止积分饱和
    int errorChange = error - m_lastError;
    m_lastError = error;
    
    // PID计算转向量
    float turnAmount = (m_Kp * error + m_Ki * m_integral + m_Kd * errorChange) / 100.0;
    
    // 限制转向量范围
    turnAmount = constrain(turnAmount, -0.8, 0.8);
    
    // 记录当前控制量，以便在丢线时使用
    m_lastTurnAmount = turnAmount;
    m_lastForwardSpeed = 0.3;  // 前进速度分量，值较小以确保安全
    
    // 根据转向量的大小选择不同的控制方式
    if (abs(turnAmount) < 0.2) {
        // 小转向量，直接前进
        m_motionController.moveForward(m_baseSpeed);
        Logger::debug("线位置: %d, 误差较小，直行", position);
    } else if (turnAmount > 0) {
        // 线偏右，需要右转修正（转向黑线方向）
        // 应用右转增益
        float adjustedTurn = turnAmount * m_rightTurnGain;
        int turnSpeed = map(abs(adjustedTurn * 100), 20, 80, m_baseSpeed/2, m_baseSpeed);
        // 确保转向速度在合理范围内
        turnSpeed = constrain(turnSpeed, m_baseSpeed/2, m_baseSpeed);
        m_motionController.spinRight(turnSpeed);
        Logger::debug("线位置: %d (偏右), 右转修正, 原始转向: %.2f, 调整后: %.2f", 
                     position, turnAmount, adjustedTurn);
    } else {
        // 线偏左，需要左转修正（转向黑线方向）
        // 应用左转增益
        float adjustedTurn = abs(turnAmount) * m_leftTurnGain;
        int turnSpeed = map(adjustedTurn * 100, 20, 80, m_baseSpeed/2, m_baseSpeed);
        // 确保转向速度在合理范围内
        turnSpeed = constrain(turnSpeed, m_baseSpeed/2, m_baseSpeed);
        m_motionController.spinLeft(turnSpeed);
        Logger::debug("线位置: %d (偏左), 左转修正, 原始转向: %.2f, 调整后: %.2f", 
                     position, turnAmount, adjustedTurn);
    }
} 
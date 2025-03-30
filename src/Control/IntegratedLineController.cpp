#include "IntegratedLineController.h"

IntegratedLineController::IntegratedLineController(InfraredArray& infrared, MotionController& motion)
    : m_infraredSensor(infrared)
    , m_motionController(motion)
    , m_state(NORMAL_LINE_FOLLOWING)
    , m_stateStartTime(0)
    , m_lineLastDetectedTime(0)
    , m_maxLineLostTime(2000)      // 默认2秒丢线超时
    , m_junctionConfirmDelay(1000) // 默认1秒路口确认延迟
    , m_baseSpeed(100)             // 默认基础速度100
    , m_lastTurnAmount(0.0)
    , m_lastLeftDetected(false)
    , m_lastCenterDetected(false)
    , m_lastRightDetected(false)
    , m_currentJunction(NO_JUNCTION)
{
}

void IntegratedLineController::init() {
    reset();
    Logger::info("集成巡线控制器已初始化");
}

void IntegratedLineController::reset() {
    m_state = NORMAL_LINE_FOLLOWING;
    m_stateStartTime = millis();
    m_lineLastDetectedTime = 0;
    m_lastTurnAmount = 0.0;
    m_currentJunction = NO_JUNCTION;
}

void IntegratedLineController::update() {
    // 根据当前状态执行相应处理
    switch (m_state) {
        case NORMAL_LINE_FOLLOWING:
            handleNormalLineFollowing();
            break;
        case POTENTIAL_JUNCTION:
            handlePotentialJunction();
            break;
        case JUNCTION_CONFIRMING:
            handleJunctionConfirming();
            break;
        case LINE_LOST:
            handleLineLost();
            break;
    }
}

void IntegratedLineController::handleNormalLineFollowing() {
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    
    // 首先检查是否为倒T字路口（所有传感器都检测到黑线）
    bool allBlack = true;
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] != 0) {
            allBlack = false;
            break;
        }
    }
    
    if (allBlack) {
        // 直接前进1秒，然后确认为T_FORWARD
        m_motionController.moveForward(m_baseSpeed);
        delay(1000);
        m_currentJunction = T_FORWARD;
        Logger::info("检测到倒T字路口");
        return;
    }
    
    // 检查是否进入其他路口模式
    if (isJunctionPattern()) {
        updateSensorStatus();
        m_state = POTENTIAL_JUNCTION;
        m_stateStartTime = millis();
        Logger::info("检测到潜在路口");
        return;
    }
    
    // 检查是否丢线
    if (!isLineDetected()) {
        if (m_lineLastDetectedTime == 0) {
            m_lineLastDetectedTime = millis();
        }
        m_state = LINE_LOST;
        return;
    }
    
    // 简化的巡线逻辑
    if (sensorValues[3] == 0 || sensorValues[4] == 0) {
        m_motionController.moveForward(m_baseSpeed);
        m_lastTurnAmount = 0;
    }
    else if (sensorValues[2] == 0) {
        m_motionController.spinLeft(25);
        m_lastTurnAmount = -0.25;
    }
    else if (sensorValues[5] == 0) {
        m_motionController.spinRight(25);
        m_lastTurnAmount = 0.25;
    }
    else if (sensorValues[1] == 0) {
        m_motionController.spinLeft(50);
        m_lastTurnAmount = -0.5;
    }
    else if (sensorValues[6] == 0) {
        m_motionController.spinRight(50);
        m_lastTurnAmount = 0.5;
    }
    else if (sensorValues[0] == 0) {
        m_motionController.spinLeft(75);
        m_lastTurnAmount = -0.75;
    }
    else if (sensorValues[7] == 0) {
        m_motionController.spinRight(75);
        m_lastTurnAmount = 0.75;
    }
}

void IntegratedLineController::handlePotentialJunction() {
    // 在潜在路口状态下继续直行一段时间
    if (millis() - m_stateStartTime < m_junctionConfirmDelay) {
        m_motionController.moveForward(m_baseSpeed);
        return;
    }
    
    // 到达确认时间，停车并进入确认状态
    m_motionController.emergencyStop();
    m_state = JUNCTION_CONFIRMING;
    m_stateStartTime = millis();
    updateSensorStatus();
    Logger::info("进入路口确认状态，更新传感器状态");
}

void IntegratedLineController::handleJunctionConfirming() {
    // 等待一小段时间让车完全停稳
    if (millis() - m_stateStartTime < 200) {
        return;
    }
    
    updateSensorStatus();
    // 确定路口类型
    m_currentJunction = determineJunctionType();
    
    // 重新进入巡线状态
    m_state = NORMAL_LINE_FOLLOWING;
    Logger::info("路口类型确认: %d", m_currentJunction);
}

void IntegratedLineController::handleLineLost() {
    unsigned long currentTime = millis();
    
    // 检查是否重新找到线
    if (isLineDetected()) {
        m_state = NORMAL_LINE_FOLLOWING;
        m_lineLastDetectedTime = 0;
        return;
    }
    
    // 检查是否超时
    if (currentTime - m_lineLastDetectedTime > m_maxLineLostTime) {
        m_motionController.emergencyStop();
        Logger::warning("丢线超时，已停止");
        return;
    }
    
    // 使用最后的转向量继续行驶，保持与巡线逻辑一致的转向速度
    if (abs(m_lastTurnAmount) > 0.6) {
        // 对应最外侧传感器的转向（原75速度）
        if (m_lastTurnAmount > 0) {
            m_motionController.spinRight(75);
        } else {
            m_motionController.spinLeft(75);
        }
    } else if (abs(m_lastTurnAmount) > 0.3) {
        // 对应外侧传感器的转向（原50速度）
        if (m_lastTurnAmount > 0) {
            m_motionController.spinRight(50);
        } else {
            m_motionController.spinLeft(50);
        }
    } else if (abs(m_lastTurnAmount) > 0.1) {
        // 对应内侧传感器的转向（原25速度）
        if (m_lastTurnAmount > 0) {
            m_motionController.spinRight(25);
        } else {
            m_motionController.spinLeft(25);
        }
    } else {
        // 基本是直线情况
        m_motionController.moveForward(m_baseSpeed);
    }
}

JunctionType IntegratedLineController::determineJunctionType() {
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    bool allWhite = true;
    
    // 检查是否所有传感器都检测到白线
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] == 0) {
            allWhite = false;
            break;
        }
    }
    
    // 记录当前的传感器状态用于调试
    Logger::debug("路口确认时传感器状态 - 左:%d 中:%d 右:%d 全白:%d", 
                 m_lastLeftDetected, m_lastCenterDetected, m_lastRightDetected, allWhite);
    
    // 根据之前保存的状态和当前状态判断路口类型
    if (allWhite) {
        // 全白说明是转弯路口
        if (m_lastLeftDetected && !m_lastRightDetected) {
            Logger::info("确认为左转弯路口");
            return LEFT_TURN;
        } else if (!m_lastLeftDetected && m_lastRightDetected) {
            Logger::info("确认为右转弯路口");
            return RIGHT_TURN;
        }
    } else {
        // 还能检测到线说明是T字路口
        if (m_lastLeftDetected && m_lastCenterDetected && !m_lastRightDetected) {
            Logger::info("确认为左T字路口");
            return T_LEFT;
        } else if (!m_lastLeftDetected && m_lastCenterDetected && m_lastRightDetected) {
            Logger::info("确认为右T字路口");
            return T_RIGHT;
        } 
    }
    
    Logger::warning("无法确认路口类型");
    return NO_JUNCTION;
}

bool IntegratedLineController::isJunctionPattern() const {
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    int detectedCount = 0;
    
    // 计算检测到黑线的传感器数量
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] == 0) {
            detectedCount++;
        }
    }
    
    // 当3个或更多传感器检测到黑线时，认为可能是路口
    return detectedCount >= 3;
}

void IntegratedLineController::updateSensorStatus() {
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    
    // 更新传感器状态
    m_lastLeftDetected = (sensorValues[0] == 0 || sensorValues[1] == 0);
    m_lastCenterDetected = (sensorValues[3] == 0 || sensorValues[4] == 0);
    m_lastRightDetected = (sensorValues[6] == 0 || sensorValues[7] == 0);
}

bool IntegratedLineController::isLineDetected() const {
    const uint16_t* sensorValues = m_infraredSensor.getAllSensorValues();
    
    // 检查是否有任何传感器检测到线
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] == 0) {
            return true;
        }
    }
    return false;
}

// Getter方法
JunctionType IntegratedLineController::getCurrentJunction() const {
    return m_currentJunction;
}

bool IntegratedLineController::isAtJunction() const {
    return m_currentJunction != NO_JUNCTION;
}

// Setter方法
void IntegratedLineController::setBaseSpeed(int speed) {
    m_baseSpeed = speed;
    Logger::debug("设置基础速度: %d", speed);
}

void IntegratedLineController::setLineLostTimeout(unsigned long timeout) {
    m_maxLineLostTime = timeout;
    Logger::debug("设置丢线超时: %lu ms", timeout);
}

void IntegratedLineController::setJunctionConfirmDelay(unsigned long delay) {
    m_junctionConfirmDelay = delay;
    Logger::debug("设置路口确认延迟: %lu ms", delay);
} 
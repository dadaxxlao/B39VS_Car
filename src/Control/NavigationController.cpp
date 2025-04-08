#include "NavigationController.h"

// 传感器数组格式化辅助函数
static void formatSensorArray(const uint16_t values[8], char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "[%d%d%d%d%d%d%d%d]",
             values[0], values[1], values[2], values[3],
             values[4], values[5], values[6], values[7]);
}

// 构造函数
NavigationController::NavigationController(SensorManager& sm, MotionController& mc, LineFollower& lf)
    : m_sensorManager(sm)
    , m_motionController(mc)
    , m_lineFollower(lf)
    , m_currentState(NAV_STOPPED)
    , m_detectedJunctionType(NO_JUNCTION)
    , m_triggerType(LineFollower::TRIGGER_NONE)
    , m_actionStartTime(0)
    , m_obstacleAvoidanceStartTime(0)
    , m_obstacleThreshold(7.0)      // 示例值: 7cm (来自ObstacleAvoidance.h)
    , m_avoidSpeed(100)             // 示例值: 100 (来自ObstacleAvoidance.h)
    , m_avoidRightDuration(1300)    // 示例值: 1500ms (来自ObstacleAvoidance.h)
    , m_avoidForwardDuration(2500)  // 示例值: 3000ms (来自ObstacleAvoidance.h)
    , m_avoidLeftDuration(1700)     // 示例值: 1700ms (来自ObstacleAvoidance.h)
    , m_lineLostStartTime(0)
    , m_maxLineLostTime(2000)  // 默认2000毫秒，与原LineFollower保持一致
    , m_isLineLost(false)
    , m_lastKnownTurnAmount(0.0)
    , m_sensorErrorCount(0)
    , m_needsVerification(false)
    , m_verificationStartTime(0)
{
    // 构造函数初始化完成
    Logger::info("NavCtrl", "Obstacle Avoidance parameters initialized: Threshold=%.1fcm, Speed=%d, Durations(R/F/L)=%lu/%lu/%lu ms",
                m_obstacleThreshold, m_avoidSpeed, m_avoidRightDuration, m_avoidForwardDuration, m_avoidLeftDuration);
}

// PID控制封装方法
void NavigationController::applyPIDControl(float turnAmount, int baseSpeed) {
    // 根据转向量的大小选择不同的控制方式
    int calculatedTurnSpeed = 0;
    const char* actionStr = "";
    
    if (abs(turnAmount) < 0.2) {
        // 小转向量，直接前进
        actionStr = "Forward";
        m_motionController.moveForward(baseSpeed);
        calculatedTurnSpeed = baseSpeed;
    } else if (turnAmount > 0) {
        // 线偏右，需要右转修正
        actionStr = "TurnRight";
        calculatedTurnSpeed = map(abs(turnAmount * 100), 20, 80, baseSpeed/2, baseSpeed);
        // 确保转向速度在合理范围内
        calculatedTurnSpeed = constrain(calculatedTurnSpeed, baseSpeed/2, baseSpeed);
        m_motionController.turnRight(calculatedTurnSpeed);
    } else {
        // 线偏左，需要左转修正
        actionStr = "TurnLeft";
        calculatedTurnSpeed = map(abs(turnAmount * 100), 20, 80, baseSpeed/2, baseSpeed);
        // 确保转向速度在合理范围内
        calculatedTurnSpeed = constrain(calculatedTurnSpeed, baseSpeed/2, baseSpeed);
        m_motionController.turnLeft(calculatedTurnSpeed);
    }
    
    Logger::debug("NavCtrl", "PID应用: 转向量=%f -> 动作=%s, 速度=%d", 
                 turnAmount, actionStr, calculatedTurnSpeed);
}

// 检查障碍物
bool NavigationController::checkForObstacle() {
    float distance;
    bool success = false;
        
    // 测量新的距离
    unsigned long duration = m_sensorManager.measurePulseDuration();
    if (duration > 0 && duration < ULTRASONIC_PULSE_TIMEOUT) {
        distance = m_sensorManager.getDistanceCmFromDuration(duration);
        success = true;
    }

    if (!success) {
        // 传感器读取失败，可能需要增加错误计数或特定处理
        Logger::warning("NavCtrl", "[CheckObstacle] 超声波传感器读取失败");
        // 返回false可能导致不避障，如果超声波不可靠，可能需要调整策略
        return false; 
    }

    // 检查距离是否在有效范围内并小于阈值
    if (distance < m_obstacleThreshold && distance > 3.0) { // 添加 distance > 0 过滤无效读数
        Logger::info("NavCtrl", "[CheckObstacle] 检测到障碍物，距离: %.2f cm (阈值: %.1f cm)", distance, m_obstacleThreshold);
        return true;
    }

    // 可以在调试时取消注释下一行
    // Logger::debug("NavCtrl", "[CheckObstacle] 未检测到障碍物，距离: %.2f cm", distance);
    return false;
}

// 初始化函数
void NavigationController::init() {
    // 初始化LineFollower
    m_lineFollower.init();
    
    // 设置初始状态为巡线
    m_currentState = NAV_FOLLOWING_LINE;
    
    Logger::info("NavCtrl", "State -> FOLLOWING_LINE (Initialized)");
    Logger::info("NavCtrl", "NavigationController初始化完成");
}

// 核心状态更新函数
void NavigationController::update() {
    // 根据当前状态执行不同的逻辑
    switch (m_currentState) {
        case NAV_FOLLOWING_LINE: {
            // 保留原有的传感器读取（用于T字型检测等其他功能）
            uint16_t sensorValues[8];
            bool sensorValuesReadSuccess = m_sensorManager.getInfraredSensorValues(sensorValues);
            
            // 传感器读取错误处理
            if (!sensorValuesReadSuccess) {
                m_sensorErrorCount++;
                Logger::warning("NavCtrl", "[State:FOLLOWING] 传感器读取失败 (%d 次连续失败)", m_sensorErrorCount);
                
                if (m_sensorErrorCount >= MAX_CONSECUTIVE_ERRORS) {
                    // 连续多次失败，紧急停止
                    m_motionController.emergencyStop();
                    Logger::error("NavCtrl", "State -> ERROR; 原因: 传感器最大错误次数 (%d) 达到！", MAX_CONSECUTIVE_ERRORS);
                    m_currentState = NAV_ERROR;
                    return;
                } else {
                    // 尝试使用上次的控制量继续移动（丢线逻辑）
                    Logger::info("NavCtrl", "传感器读取失败，尝试按最后方向行驶");
                    if (abs(m_lastKnownTurnAmount) > 0.2) {
                        if (m_lastKnownTurnAmount > 0)
                            m_motionController.turnRight(m_lineFollower.getBaseSpeed());
                        else
                            m_motionController.turnLeft(m_lineFollower.getBaseSpeed());
                    } else {
                        m_motionController.moveForward(m_lineFollower.getBaseSpeed());
                    }
                    return;
                }
            } else {
                // 读取成功，重置错误计数器
                m_sensorErrorCount = 0;
            }
            
            // >>> 开始添加障碍物检测 (在成功读取红外之后，巡线逻辑之前) <<<
            if (checkForObstacle()) {
                // 检测到障碍物，开始避障流程
                m_motionController.emergencyStop();
                Logger::info("NavCtrl", "检测到障碍物! State -> NAV_AVOIDING_RIGHT");

                // 切换到向右平移状态
                m_currentState = NAV_AVOIDING_RIGHT;
                m_obstacleAvoidanceStartTime = millis();
                m_motionController.lateralRight(m_avoidSpeed);
                Logger::debug("NavCtrl", "开始向右平移避障，速度: %d", m_avoidSpeed);
                return; // 立即返回，避免执行后续巡线逻辑
            }
            // >>> 结束添加障碍物检测 <<<

            // 检测是否有线
            bool lineDetected = m_sensorManager.isLineDetected();
            
            // 当前时间
            unsigned long currentTime = millis();
            
            // 调用LineFollower更新，获取触发类型和转向量
            LineFollower::TriggerType trigger = m_lineFollower.update();
            float turnAmount = m_lineFollower.getLastTurnAmount();
            
            // 丢线处理
            if (!lineDetected) {
                // 如果没有检测到线
                if (!m_isLineLost) {
                    // 首次丢失线，记录时间
                    m_isLineLost = true;
                    m_lineLostStartTime = currentTime;
                    
                    // 使用最后的控制量继续行驶
                    Logger::info("NavCtrl", "线路丢失! 开始恢复 (最后转向量: %f)", m_lastKnownTurnAmount);
                    if (abs(m_lastKnownTurnAmount) > 0.2) {
                        // 根据转向量的符号决定转向方向
                        if (m_lastKnownTurnAmount > 0) {
                            m_motionController.turnRight(m_lineFollower.getBaseSpeed());  // 上次偏右，所以右转向线方向
                        } else {
                            m_motionController.turnLeft(m_lineFollower.getBaseSpeed()); // 上次偏左，所以左转向线方向
                        }
                    } else {
                        m_motionController.moveForward(m_lineFollower.getBaseSpeed());
                    }
                    return;
                } else if (currentTime - m_lineLostStartTime > m_maxLineLostTime) {
                    // 超过最长允许丢线时间，停车
                    m_motionController.emergencyStop();
                    Logger::warning("NavCtrl", "线路丢失超时 (%lu 毫秒)! 停止中.", m_maxLineLostTime);
                    Logger::error("NavCtrl", "State -> ERROR; 原因: 线路丢失超时");
                    m_currentState = NAV_ERROR;
                    return;
                } else {
                    // 在允许的丢线时间内，继续按最后方向行驶
                    if (abs(m_lastKnownTurnAmount) > 0.2) {
                        // 根据转向量的符号决定转向方向
                        if (m_lastKnownTurnAmount > 0) {
                            m_motionController.turnRight(m_lineFollower.getBaseSpeed());
                        } else {
                            m_motionController.turnLeft(m_lineFollower.getBaseSpeed());
                        }
                    } else {
                        m_motionController.moveForward(m_lineFollower.getBaseSpeed());
                    }
                    return;
                }
            } else {
                // 检测到线，重置丢线状态
                if (m_isLineLost) {
                    Logger::info("NavCtrl", "线路重新获得!");
                    m_isLineLost = false;
                }
                m_lineLostStartTime = 0;
                
                // 更新最后已知的有效转向量
                m_lastKnownTurnAmount = turnAmount;
            }
            
            // 检查是否为T_FORWARD（全黑模式）
            if (m_lineDetector.isForwardTee(sensorValues)) {
                char sensorStr[40];
                formatSensorArray(sensorValues, sensorStr, sizeof(sensorStr));
                Logger::info("NavCtrl", "检测到T_FORWARD! 传感器: %s", sensorStr);
                m_motionController.emergencyStop();
                Logger::info("NavCtrl", "State -> AT_JUNCTION (Type: T_FORWARD)");
                m_detectedJunctionType = T_FORWARD;
                m_currentState = NAV_AT_JUNCTION;
                m_motionController.moveForward();
                delay(NAV_CHECK_FORWARD_DURATION);
                return;
            }
            
            // 检查是否有边缘触发
            if (trigger == LineFollower::TRIGGER_LEFT_EDGE || 
                trigger == LineFollower::TRIGGER_RIGHT_EDGE) {
                char sensorStr[40];
                formatSensorArray(sensorValues, sensorStr, sizeof(sensorStr));
                Logger::info("NavCtrl", "检测到边缘触发! 类型: %d, 传感器: %s", 
                           trigger, sensorStr);
                
                m_triggerType = trigger;
                
                // 开始短距前进
                Logger::info("NavCtrl", "State -> MOVING_TO_STOP (Trigger: %d)", m_triggerType);
                m_motionController.moveForward(NAV_CHECK_FORWARD_SPEED);
                m_actionStartTime = millis();
                m_currentState = NAV_MOVING_TO_STOP;
                
                if (trigger == LineFollower::TRIGGER_LEFT_EDGE) {
                    Logger::info("NavCtrl", "检测到左边缘触发，开始移动检查");
                } else {
                    Logger::info("NavCtrl", "检测到右边缘触发，开始移动检查");
                }
                return;
            }
            
            // 正常巡线 - 应用PID控制
            applyPIDControl(turnAmount, m_lineFollower.getBaseSpeed());
            break;
        }
        
        case NAV_MOVING_TO_STOP: {
            // 检查短距前进是否完成
            if (millis() - m_actionStartTime >= NAV_CHECK_FORWARD_DURATION) {
                // 短距前进完成，停车
                m_motionController.emergencyStop();
                m_actionStartTime = millis(); // 记录停止时间，用于稳定延迟
                Logger::info("NavCtrl", "State -> STOPPED_FOR_CHECK");
                m_currentState = NAV_STOPPED_FOR_CHECK;
                Logger::debug("NavCtrl", "停车检查: 等待 %.1f 秒稳定", NAV_CHECK_STABILIZE_DELAY / 1000.0);
                Logger::info("NavCtrl", "短距移动完成，开始停车检查");
            }
            break;
        }
        
        case NAV_STOPPED_FOR_CHECK: {
            // 检查稳定时间是否到
            if (millis() - m_actionStartTime >= NAV_CHECK_STABILIZE_DELAY) {
                // 稳定时间到，获取静态传感器值
                uint16_t sensorValues[8];
                m_sensorManager.getInfraredSensorValues(sensorValues);
                
                char sensorStr[40];
                formatSensorArray(sensorValues, sensorStr, sizeof(sensorStr));
                Logger::debug("NavCtrl", "停车检查: 读取静态传感器值: %s", sensorStr);
                
                // 分类判断路口类型
                m_detectedJunctionType = m_lineDetector.classifyStoppedJunction(sensorValues, m_triggerType);
                
                // 检查是否可能是全白模式误判
                bool isAllWhite = true;
                for (int i = 0; i < 8; i++) {
                    if (sensorValues[i] == 0) {
                        isAllWhite = false;
                        break;
                    }
                }
                
                if (isAllWhite && (m_detectedJunctionType == LEFT_TURN || m_detectedJunctionType == RIGHT_TURN)) {
                    // 可能是全白误判，需要进行微调验证
                    m_needsVerification = true;
                    m_verificationStartTime = millis();
                    
                    // 根据触发类型决定微调方向
                    if (m_triggerType == LineFollower::TRIGGER_LEFT_EDGE) {
                        // 向右微调，尝试找回可能丢失的左侧线
                        m_motionController.spinRight(50);
                        Logger::info("NavCtrl", "State -> VERIFYING_ALL_WHITE (向右微调)");
                    } else {
                        // 向左微调，尝试找回可能丢失的右侧线
                        m_motionController.spinLeft(50);
                        Logger::info("NavCtrl", "State -> VERIFYING_ALL_WHITE (向左微调)");
                    }
                    
                    m_currentState = NAV_VERIFYING_ALL_WHITE;
                    Logger::info("NavCtrl", "全白模式检测到，开始微调验证");
                } else {
                    // 不需要验证，直接进入路口状态
                    Logger::info("NavCtrl", "State -> AT_JUNCTION (Type: %d)", m_detectedJunctionType);
                    m_currentState = NAV_AT_JUNCTION;
                    Logger::info("NavCtrl", "静态检查完成，路口类型: %d", m_detectedJunctionType);
                }
            }
            break;
        }
        
        case NAV_VERIFYING_ALL_WHITE: {
            // 检查微调时间是否到
            if (millis() - m_verificationStartTime >= VERIFICATION_TURN_DURATION) {
                // 微调完成，停车并重新检测
                m_motionController.emergencyStop();
                
                // 短暂延时让传感器稳定
                delay(50);
                
                // 重新获取传感器值
                uint16_t sensorValues[8];
                m_sensorManager.getInfraredSensorValues(sensorValues);
                
                char sensorStr[40];
                formatSensorArray(sensorValues, sensorStr, sizeof(sensorStr));
                Logger::debug("NavCtrl", "微调后检查: 读取静态传感器值: %s", sensorStr);
                
                // 检查是否检测到线（不再是全白）
                bool stillAllWhite = true;
                for (int i = 0; i < 8; i++) {
                    if (sensorValues[i] == 0) {
                        stillAllWhite = false;
                        break;
                    }
                }
                
                if (!stillAllWhite) {
                    // 微调后检测到线，重新分类
                    JunctionType newType = m_lineDetector.classifyStoppedJunction(sensorValues, m_triggerType);
                    Logger::info("NavCtrl", "微调后检测到线! 重新分类为: %d", newType);
                    
                    // 更新路口类型
                    if (newType != NO_JUNCTION) {
                        m_detectedJunctionType = newType;
                    }
                } else {
                    // 仍然是全白，保持原来的分类
                    Logger::info("NavCtrl", "微调后仍为全白，保持原分类: %d", m_detectedJunctionType);
                }
                
                // 进入路口状态
                Logger::info("NavCtrl", "State -> AT_JUNCTION (Type: %d, 验证后)", m_detectedJunctionType);
                m_currentState = NAV_AT_JUNCTION;
            }
            break;
        }
        
        case NAV_AT_JUNCTION:
            // 此状态下不执行任何操作，等待StateMachine查询状态并调用resumeFollowing()
            break;
            
        case NAV_STOPPED:
        case NAV_ERROR:
            // 停止状态或错误状态，不执行任何操作
            break;
            
        case NAV_AVOIDING_RIGHT: {
            unsigned long currentTime = millis();
            Logger::debug("NavCtrl", "[State:AVOIDING_RIGHT] Time elapsed: %lu ms", currentTime - m_obstacleAvoidanceStartTime);
            if (currentTime - m_obstacleAvoidanceStartTime >= m_avoidRightDuration) {
                // 右平移时间到，切换到向前行驶
                Logger::info("NavCtrl", "右平移完成. State -> NAV_AVOIDING_FORWARD");
                m_currentState = NAV_AVOIDING_FORWARD;
                m_obstacleAvoidanceStartTime = currentTime;
                m_motionController.moveForward(m_avoidSpeed);
                Logger::debug("NavCtrl", "开始向前行驶避障，速度: %d", m_avoidSpeed);
            }
            // 否则，保持向右平移 (MotionController应保持上一个命令)
            break;
        }

        case NAV_AVOIDING_FORWARD: {
            unsigned long currentTime = millis();
            Logger::debug("NavCtrl", "[State:AVOIDING_FORWARD] Time elapsed: %lu ms", currentTime - m_obstacleAvoidanceStartTime);
            if (currentTime - m_obstacleAvoidanceStartTime >= m_avoidForwardDuration) {
                // 向前行驶时间到，切换到向左平移
                Logger::info("NavCtrl", "向前行驶完成. State -> NAV_AVOIDING_LEFT");
                m_currentState = NAV_AVOIDING_LEFT;
                m_obstacleAvoidanceStartTime = currentTime;
                m_motionController.lateralLeft(m_avoidSpeed);
                Logger::debug("NavCtrl", "开始向左平移寻找线，速度: %d", m_avoidSpeed);
            }
            // 否则，保持向前行驶
            break;
        }

        case NAV_AVOIDING_LEFT: {
            unsigned long currentTime = millis();
            Logger::debug("NavCtrl", "[State:AVOIDING_LEFT] Time elapsed: %lu ms", currentTime - m_obstacleAvoidanceStartTime);
            // 检查是否找到线
            uint16_t sensorValues[8];
            bool success = m_sensorManager.getInfraredSensorValues(sensorValues);

            if (success) {
                // 检查中间两个传感器(3和4)是否检测到黑线 (值为0)
                // 注意: 传感器索引和黑线值(0代表黑线)需要根据实际硬件确认
                if (sensorValues[3] == 0 || sensorValues[4] == 0) {
                    m_motionController.emergencyStop();
                    char sensorStr[40];
                    formatSensorArray(sensorValues, sensorStr, sizeof(sensorStr));
                    Logger::info("NavCtrl", "在左平移时找到线! 传感器: %s. State -> NAV_FOLLOWING_LINE", sensorStr);
                    delay(500);
                    // 可以在这里重置一些巡线相关状态，例如 m_isLineLost = false;
                    m_isLineLost = false; 
                    m_lineLostStartTime = 0;
                    m_currentState = NAV_FOLLOWING_LINE;
                    return; // 完成避障，返回巡线
                }
            } else {
                 Logger::warning("NavCtrl", "[State:AVOIDING_LEFT] 红外传感器读取失败");
                 // 这里可以考虑是否也停止，或者继续尝试直到超时
            }

            // 检查是否超时
            if (currentTime - m_obstacleAvoidanceStartTime >= m_avoidLeftDuration) {
                m_motionController.emergencyStop();
                Logger::warning("NavCtrl", "左平移超时 (%lu ms)! 强制返回巡线. State -> NAV_FOLLOWING_LINE", m_avoidLeftDuration);
                m_currentState = NAV_FOLLOWING_LINE; // 超时也尝试返回巡线状态
                // 可以在这里重置一些巡线相关状态
                m_isLineLost = false; 
                m_lineLostStartTime = 0;
                // 可能需要一个错误状态或特殊处理？暂时返回巡线
            }
            // 否则，保持向左平移
            break;
        }
        
        default:
            // 未知状态，记录错误
            Logger::error("NavCtrl", "未知的导航状态: %d", m_currentState);
            m_currentState = NAV_ERROR;
            break;
    }
}

// 获取当前导航状态
NavigationState NavigationController::getCurrentNavigationState() const {
    return m_currentState;
}

// 获取检测到的路口类型
JunctionType NavigationController::getDetectedJunctionType() const {
    return m_detectedJunctionType;
}

// 恢复巡线状态
void NavigationController::resumeFollowing() {
    // 重置路口类型
    m_detectedJunctionType = NO_JUNCTION;
    // 重置丢线状态，以防在避障完成时恰好处于丢线恢复中
    m_isLineLost = false;
    m_lineLostStartTime = 0;
    // 恢复巡线状态
    Logger::info("NavCtrl", "State -> FOLLOWING_LINE (Resumed)");
    m_currentState = NAV_FOLLOWING_LINE;
    Logger::info("NavCtrl", "恢复巡线");
}

// 强制停止导航
void NavigationController::stop() {
    m_motionController.emergencyStop();
    Logger::info("NavCtrl", "State -> STOPPED");
    m_currentState = NAV_STOPPED;
    Logger::info("NavCtrl", "导航停止");
} 
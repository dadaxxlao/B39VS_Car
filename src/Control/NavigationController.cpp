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
    , m_lineLostStartTime(0)
    , m_maxLineLostTime(2000)  // 默认2000毫秒，与原LineFollower保持一致
    , m_isLineLost(false)
    , m_lastKnownTurnAmount(0.0)
    , m_sensorErrorCount(0)
{
    // 构造函数初始化完成
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
        actionStr = "SpinRight";
        calculatedTurnSpeed = map(abs(turnAmount * 100), 20, 80, baseSpeed/2, baseSpeed);
        // 确保转向速度在合理范围内
        calculatedTurnSpeed = constrain(calculatedTurnSpeed, baseSpeed/2, baseSpeed);
        m_motionController.spinRight(calculatedTurnSpeed);
    } else {
        // 线偏左，需要左转修正
        actionStr = "SpinLeft";
        calculatedTurnSpeed = map(abs(turnAmount * 100), 20, 80, baseSpeed/2, baseSpeed);
        // 确保转向速度在合理范围内
        calculatedTurnSpeed = constrain(calculatedTurnSpeed, baseSpeed/2, baseSpeed);
        m_motionController.spinLeft(calculatedTurnSpeed);
    }
    
    Logger::debug("NavCtrl", "PID应用: 转向量=%f -> 动作=%s, 速度=%d", 
                 turnAmount, actionStr, calculatedTurnSpeed);
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
                            m_motionController.spinRight(m_lineFollower.getBaseSpeed());
                        else
                            m_motionController.spinLeft(m_lineFollower.getBaseSpeed());
                    } else {
                        m_motionController.moveForward(m_lineFollower.getBaseSpeed());
                    }
                    return;
                }
            } else {
                // 读取成功，重置错误计数器
                m_sensorErrorCount = 0;
            }
            
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
                            m_motionController.spinRight(m_lineFollower.getBaseSpeed());  // 上次偏右，所以右转向线方向
                        } else {
                            m_motionController.spinLeft(m_lineFollower.getBaseSpeed()); // 上次偏左，所以左转向线方向
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
                            m_motionController.spinRight(m_lineFollower.getBaseSpeed());
                        } else {
                            m_motionController.spinLeft(m_lineFollower.getBaseSpeed());
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
                Logger::info("NavCtrl", "State -> AT_JUNCTION (Type: %d)", m_detectedJunctionType);
                m_currentState = NAV_AT_JUNCTION;
                
                Logger::info("NavCtrl", "静态检查完成，路口类型: %d", m_detectedJunctionType);
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
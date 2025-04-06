#include "ObstacleAvoidance.h"

// 构造函数 - 不使用LineFollower的版本
ObstacleAvoidance::ObstacleAvoidance(SensorManager& sensorManager, MotionController& motionController)
    : m_sensorManager(sensorManager)
    , m_motionController(motionController)
    , m_lineFollower(nullptr)
    , m_useLineFollower(false)
    , m_currentState(OBS_INACTIVE)
    , m_actionStartTime(0)
{
    // 构造函数初始化
}

// 构造函数 - 使用LineFollower的版本
ObstacleAvoidance::ObstacleAvoidance(SensorManager& sensorManager, MotionController& motionController, LineFollower& lineFollower)
    : m_sensorManager(sensorManager)
    , m_motionController(motionController)
    , m_lineFollower(&lineFollower)
    , m_useLineFollower(true)
    , m_currentState(OBS_INACTIVE)
    , m_actionStartTime(0)
{
    // 构造函数初始化
    Logger::info("ObsAvoid", "已配置LineFollower支持");
}

// 初始化
void ObstacleAvoidance::init() {
    // 设置初始状态
    m_currentState = OBS_INACTIVE;
    Logger::info("ObsAvoid", "避障模块初始化完成");
}

// 启动避障检测
void ObstacleAvoidance::startDetecting() {
    if (m_currentState == OBS_INACTIVE) {
        m_currentState = OBS_DETECTING;
        Logger::info("ObsAvoid", "开始避障检测");
    }
}

// 停止避障检测
void ObstacleAvoidance::stopDetecting() {
    if (m_currentState != OBS_INACTIVE) {
        m_currentState = OBS_INACTIVE;
        Logger::info("ObsAvoid", "停止避障检测");
    }
}

// 检查障碍物
bool ObstacleAvoidance::checkForObstacle() {
    // 获取超声波距离
    float distance = 0;
    bool success = m_sensorManager.getDistanceCm(distance);
    
    if (!success) {
        Logger::warning("ObsAvoid", "超声波传感器读取失败");
        return false;
    }
    
    // 检查是否有障碍物
    if (distance < OBSTACLE_THRESHOLD) {
        Logger::info("ObsAvoid", "检测到障碍物，距离: %.2f cm", distance);
        return true;
    }
    
    return false;
}

// 应用PID控制
void ObstacleAvoidance::applyPIDControl(float turnAmount, int baseSpeed) {
    // 根据转向量设置小车移动
    if (turnAmount > 0) {  // 需要右转
        // 使用MotionController的turnRight方法
        m_motionController.turnRight(baseSpeed);
    } else if (turnAmount < 0) {  // 需要左转
        // 使用MotionController的turnLeft方法
        m_motionController.turnLeft(baseSpeed);
    } else {  // 直行
        // 使用MotionController的moveForward方法
        m_motionController.moveForward(baseSpeed);
    }
    
    // 记录当前控制
    Logger::debug("ObsAvoid", "PID控制: 转向量=%.2f, 速度=%d", turnAmount, baseSpeed);
}

// 更新状态
void ObstacleAvoidance::update() {
    unsigned long currentTime = millis();
    
    switch (m_currentState) {
        case OBS_INACTIVE:
            // 非活动状态，不执行任何操作
            break;
            
        case OBS_DETECTING:
            // 检测障碍物并同时巡线
            if (m_useLineFollower && m_lineFollower != nullptr) {
                // 使用LineFollower进行巡线
                LineFollower::TriggerType trigger = m_lineFollower->update();
                float turnAmount = m_lineFollower->getLastTurnAmount();
                int baseSpeed = m_lineFollower->getBaseSpeed();
                
                // 应用PID控制
                applyPIDControl(turnAmount, baseSpeed);
                
                // 记录巡线状态
                if (trigger != LineFollower::TRIGGER_NONE) {
                    Logger::debug("ObsAvoid", "巡线触发事件: %d", trigger);
                }
            }
            
            // 检测障碍物
            if (checkForObstacle()) {
                // 检测到障碍物，开始避障过程
                m_motionController.emergencyStop();
                Logger::info("ObsAvoid", "检测到障碍物，开始避障");
                
                // 开始向右平移避障
                m_motionController.lateralRight(AVOID_SPEED);
                m_actionStartTime = currentTime;
                m_currentState = OBS_AVOIDING_RIGHT;
                Logger::info("ObsAvoid", "状态: 向右平移避障");
            }
            break;
            
        case OBS_AVOIDING_RIGHT:
            // 向右平移阶段
            if (currentTime - m_actionStartTime >= RIGHT_MOVE_DURATION) {
                // 向右平移结束，开始向前行驶
                m_motionController.moveForward(AVOID_SPEED);
                m_actionStartTime = currentTime;
                m_currentState = OBS_AVOIDING_FORWARD;
                Logger::info("ObsAvoid", "状态: 向前行驶避障");
            }
            break;
            
        case OBS_AVOIDING_FORWARD:
            // 向前行驶阶段
            if (currentTime - m_actionStartTime >= FORWARD_MOVE_DURATION) {
                // 向前行驶结束，开始向左平移
                m_motionController.lateralLeft(AVOID_SPEED);
                m_actionStartTime = currentTime;
                m_currentState = OBS_AVOIDING_LEFT;
                Logger::info("ObsAvoid", "状态: 向左平移避障");
            }
            break;
            
        case OBS_AVOIDING_LEFT:
            // 向左平移阶段
            {
                // 获取红外传感器值
                uint16_t sensorValues[8];
                bool success = m_sensorManager.getInfraredSensorValues(sensorValues);
                
                if (success) {
                    // 检查中间两个传感器(3和4)是否有任意一个检测到黑线
                    if (sensorValues[3] == 0 || sensorValues[4] == 0) {
                        // 中间传感器检测到黑线，避障完成
                        m_motionController.emergencyStop();
                        m_currentState = OBS_COMPLETED;
                        Logger::info("ObsAvoid", "中间传感器检测到黑线，避障完成");
                        break;
                    }
                }
                
                // 超时检查（安全机制）
                if (currentTime - m_actionStartTime >= LEFT_MOVE_DURATION) {
                    // 向左平移超时，避障完成
                    m_motionController.emergencyStop();
                    m_currentState = OBS_COMPLETED;
                    Logger::warning("ObsAvoid", "向左平移超时，避障完成");
                }
            }
            break;
            
        case OBS_COMPLETED:
            // 避障完成状态，不执行任何操作
            break;
            
        default:
            // 未知状态，重置为非活动状态
            Logger::error("ObsAvoid", "未知状态: %d", m_currentState);
            m_currentState = OBS_INACTIVE;
            break;
    }
}

// 获取当前状态
ObstacleAvoidanceState ObstacleAvoidance::getCurrentState() const {
    return m_currentState;
}

// 检查避障是否已完成
bool ObstacleAvoidance::isAvoidanceCompleted() const {
    return m_currentState == OBS_COMPLETED;
}

// 重置状态
void ObstacleAvoidance::reset() {
    m_currentState = OBS_INACTIVE;
    Logger::info("ObsAvoid", "避障状态已重置");
}

// 设置障碍物检测阈值
void ObstacleAvoidance::setObstacleThreshold(float threshold) {
    if (threshold > 0) {
        const_cast<float&>(OBSTACLE_THRESHOLD) = threshold;
        Logger::info("ObsAvoid", "障碍物检测阈值已设置为 %.2f cm", threshold);
    }
} 
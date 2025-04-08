#include "LineFollower.h"

// 构造函数 - 更新为不接收 MotionController
LineFollower::LineFollower(SensorManager& sensorManager)
    : m_sensorManager(sensorManager) // 初始化 SensorManager
    , m_Kp(1.0)
    , m_Ki(0.0)
    , m_Kd(0.0)
    , m_lastError(0)
    , m_integral(0)
    // 移除丢线处理参数，由NavigationController接管
    // , m_lineLastDetectedTime(0)
    // , m_maxLineLostTime(2000)
    , m_lastTurnAmount(0.0)
    , m_baseSpeed(FOLLOW_SPEED)
{
}

// 初始化函数
void LineFollower::init() {
    // 重置所有状态
    reset();
    Logger::info("LineFollower", "巡线控制器已初始化");
}

// 设置PID参数
void LineFollower::setPIDParams(float Kp, float Ki, float Kd) {
    m_Kp = Kp;
    m_Ki = Ki;
    m_Kd = Kd;
    // 重置PID状态，防止突变
    m_integral = 0;
    m_lastError = 0;
    Logger::debug("LineFollower", "已设置PID参数: Kp=%.2f, Ki=%.2f, Kd=%.2f", m_Kp, m_Ki, m_Kd);
}

// 设置丢线处理参数 - 注释掉，由NavigationController接管
/*
void LineFollower::setLineLostParams(unsigned long maxLineLostTime) {
    m_maxLineLostTime = maxLineLostTime;
    Logger::debug("LineFollower", "已设置最长允许丢线时间: %lu ms", m_maxLineLostTime);
}
*/

// 设置基础速度
void LineFollower::setBaseSpeed(int speed) {
    m_baseSpeed = speed;
    Logger::debug("LineFollower", "已设置基础速度: %d", m_baseSpeed);
}

// 重置状态
void LineFollower::reset() {
    m_lastError = 0;
    m_integral = 0;
    // m_lineLastDetectedTime = 0;  // 移除，由NavigationController接管
    m_lastTurnAmount = 0.0;
}

// 巡线函数 - 更新机器人移动，现在返回TriggerType
LineFollower::TriggerType LineFollower::update() {
    // 获取线位置
    int position = 0;
    m_sensorManager.getLinePosition(position);

    // 获取传感器原始值，用于检测边缘触发
    uint16_t sensorValues[8];
    m_sensorManager.getInfraredSensorValues(sensorValues);


    // 检查边缘触发模式
    if (sensorValues[0] == 0 && sensorValues[1] == 0) {
        Logger::debug("LineFoll", "边缘触发: 左边缘");
        return TRIGGER_LEFT_EDGE;
    } else if (sensorValues[6] == 0 && sensorValues[7] == 0) {
        Logger::debug("LineFoll", "边缘触发: 右边缘");
        return TRIGGER_RIGHT_EDGE;
    }
    
    // 正常巡线逻辑 - 计算PID调整量，但不直接控制电机
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
    
    // 记录当前控制量，以便在NavigationController中使用
    m_lastTurnAmount = turnAmount;
    
    // PID计算日志
    Logger::debug("LineFoll", "PID计算: 位置=%d, 误差=%d, 积分=%d, 微分=%d, 转向量=%f",
                 position, error, m_integral, errorChange, m_lastTurnAmount);
    
    // 返回TRIGGER_NONE表示没有触发特殊条件
    return TRIGGER_NONE;
} 
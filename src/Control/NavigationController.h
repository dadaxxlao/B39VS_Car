#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "LineFollower.h"
#include "LineDetector.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 导航状态枚举
enum NavigationState {

    NAV_STOPPED,             // 初始或错误停止状态
    NAV_FOLLOWING_LINE,      // 正常巡线
    NAV_POTENTIAL_JUNCTION,  // 可选，检测到边缘触发
    NAV_MOVING_TO_STOP,      // 正在执行短距前进
    NAV_STOPPED_FOR_CHECK,   // 已停止，准备静态检测
    NAV_AT_JUNCTION,         // 已完成静态检测，等待StateMachine决策
    NAV_AVOIDING_RIGHT,      // 向右平移避障
    NAV_AVOIDING_FORWARD,    // 向前行驶绕过障碍物
    NAV_AVOIDING_LEFT,       // 向左平移寻找线
    NAV_ERROR                // 导航错误状态
};

class NavigationController {
private:
    // 引用其他组件
    SensorManager& m_sensorManager;
    MotionController& m_motionController;
    LineFollower& m_lineFollower;
    LineDetector m_lineDetector;
    
    // 当前导航状态
    NavigationState m_currentState;
    
    // 检测到的路口类型
    JunctionType m_detectedJunctionType;
    
    // 触发类型
    LineFollower::TriggerType m_triggerType;  // 存储触发检查的模式

    unsigned long m_actionStartTime;          // 用于计时短距前进等
    unsigned long m_obstacleAvoidanceStartTime; // 用于计时避障动作
    
    // 避障参数 (可以考虑从Config加载或设为常量)
    float m_obstacleThreshold;          // 障碍物检测阈值 (cm)
    int m_avoidSpeed;                   // 避障速度
    unsigned long m_avoidRightDuration; // 向右平移时间 (ms)
    unsigned long m_avoidForwardDuration; // 向前行驶时间 (ms)
    unsigned long m_avoidLeftDuration;  // 向左平移最大时间 (ms)

    
    // 时间控制
    unsigned long m_actionStartTime;  // 动作开始时间
    unsigned long m_lineLostStartTime; // 丢线起始时间
    unsigned long m_maxLineLostTime;   // 最大允许丢线时间
    
    // 丢线恢复相关
    bool m_isLineLost;            // 是否丢线
    float m_lastKnownTurnAmount; // 最后已知的转向量
    
    // 传感器错误计数
    int m_sensorErrorCount;
    const int MAX_CONSECUTIVE_ERRORS = 5; // 最大连续错误次数
    
    // 新增：全白验证相关
    bool m_needsVerification; // 是否需要验证
    unsigned long m_verificationStartTime; // 验证开始时间
    const unsigned long VERIFICATION_TURN_DURATION = 50; // 微转向时间(0.05秒)
    
    // PID控制封装方法
    void applyPIDControl(float turnAmount, int baseSpeed);

    bool checkForObstacle();

public:
    // 构造函数
    NavigationController(SensorManager& sm, MotionController& mc, LineFollower& lf);
    
    // 初始化
    void init();
    
    // 更新状态
    void update();
    
    // 获取当前导航状态
    NavigationState getCurrentNavigationState() const;
    
    // 获取检测到的路口类型
    JunctionType getDetectedJunctionType() const;
    
    // 恢复巡线状态
    void resumeFollowing();
    
    // 强制停止导航
    void stop();
    
    // 常量
 
};

#endif // NAVIGATION_CONTROLLER_H 
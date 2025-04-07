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
    // 引用成员
    SensorManager& m_sensorManager;
    MotionController& m_motionController;
    LineFollower& m_lineFollower;
    LineDetector m_lineDetector;
    
    // 状态变量
    NavigationState m_currentState;
    JunctionType m_detectedJunctionType;
    LineFollower::TriggerType m_triggerType;  // 存储触发检查的模式
    unsigned long m_actionStartTime;          // 用于计时短距前进等
    unsigned long m_obstacleAvoidanceStartTime; // 用于计时避障动作
    
    // 避障参数 (可以考虑从Config加载或设为常量)
    float m_obstacleThreshold;          // 障碍物检测阈值 (cm)
    int m_avoidSpeed;                   // 避障速度
    unsigned long m_avoidRightDuration; // 向右平移时间 (ms)
    unsigned long m_avoidForwardDuration; // 向前行驶时间 (ms)
    unsigned long m_avoidLeftDuration;  // 向左平移最大时间 (ms)
    
    // 丢线处理变量
    unsigned long m_lineLostStartTime;        // 丢线开始时间
    unsigned long m_maxLineLostTime;          // 最大丢线容忍时间
    bool m_isLineLost;                        // 是否处于丢线状态
    float m_lastKnownTurnAmount;              // 最后一次有效的转向量
    
    // 传感器错误处理
    int m_sensorErrorCount;                   // 传感器读取错误计数
    static const int MAX_CONSECUTIVE_ERRORS = 5; // 最大连续错误次数
    
    // PID控制封装方法
    void applyPIDControl(float turnAmount, int baseSpeed);
    bool checkForObstacle();

public:
    // 构造函数
    NavigationController(SensorManager& sm, MotionController& mc, LineFollower& lf);
    
    // 初始化函数
    void init();
    
    // 核心状态更新函数
    void update();
    
    // 获取当前导航状态
    NavigationState getCurrentNavigationState() const;
    
    // 获取检测到的路口类型（仅在NAV_AT_JUNCTION状态有效）
    JunctionType getDetectedJunctionType() const;
    
    // 恢复巡线状态（由StateMachine调用）
    void resumeFollowing();
    
    // 强制停止导航
    void stop();
};

#endif // NAVIGATION_CONTROLLER_H 
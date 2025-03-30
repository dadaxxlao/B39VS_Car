/*
    集成了巡线和路口判断
    作为原来PID控制方案的简化和整合
    将路口判断进行集合进巡线红外过程中
*/
#ifndef INTEGRATED_LINE_CONTROLLER_H
#define INTEGRATED_LINE_CONTROLLER_H

#include "../Motor/MotionController.h"
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

class IntegratedLineController {
private:
    // 控制状态枚举
    enum ControlState {
        NORMAL_LINE_FOLLOWING,    // 普通巡线状态
        POTENTIAL_JUNCTION,       // 潜在路口状态
        JUNCTION_CONFIRMING,      // 路口确认状态
        LINE_LOST                 // 丢线状态
    };

    // 组件引用
    InfraredArray& m_infraredSensor;
    MotionController& m_motionController;

    // 状态变量
    ControlState m_state;
    unsigned long m_stateStartTime;
    unsigned long m_lineLastDetectedTime;
    
    // 配置参数
    unsigned long m_maxLineLostTime;      // 最大允许丢线时间
    unsigned long m_junctionConfirmDelay; // 路口确认延迟时间
    int m_baseSpeed;                      // 基础速度
    float m_lastTurnAmount;              // 上次转向量
    
    // 路口检测变量
    bool m_lastLeftDetected;
    bool m_lastCenterDetected;
    bool m_lastRightDetected;
    JunctionType m_currentJunction;

    // 私有辅助方法
    void handleNormalLineFollowing();
    void handlePotentialJunction();
    void handleJunctionConfirming();
    void handleLineLost();
    JunctionType determineJunctionType();
    
    // 传感器状态分析
    bool isJunctionPattern() const;
    void updateSensorStatus();
    bool isLineDetected() const;

public:
    // 构造函数
    IntegratedLineController(InfraredArray& infrared, MotionController& motion);
    
    // 初始化
    void init();
    
    // 主更新函数
    void update();
    
    // 获取当前状态信息
    JunctionType getCurrentJunction() const;
    bool isAtJunction() const;
    
    // 配置方法
    void setBaseSpeed(int speed);
    void setLineLostTimeout(unsigned long timeout);
    void setJunctionConfirmDelay(unsigned long delay);
    
    // 状态重置
    void reset();
};

#endif // INTEGRATED_LINE_CONTROLLER_H 
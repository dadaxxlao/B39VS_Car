#ifndef SIMPLE_STATE_MACHINE_H
#define SIMPLE_STATE_MACHINE_H

#include <Arduino.h>
#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Arm/RoboticArm.h"
#include "../Control/NavigationController.h"
#include "../Control/AccurateTurn.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

// 前向声明
class SensorManager;
class MotionController;
class RoboticArm;
class NavigationController;
class AccurateTurn;

/**
 * 简化版状态机，使用if-else逻辑替代switch-case
 * 负责控制小车在不同任务阶段的行为
 * 与NavigationController协作，在路口处做出决策
 */
class SimpleStateMachine {
public:
    // 构造函数
    SimpleStateMachine(SensorManager& sm, MotionController& mc, 
                     RoboticArm& arm, NavigationController& nc);
    
    // 初始化函数
    void init();
    
    // 主循环更新函数
    void update();
    
    // 状态转换函数
    void transitionTo(SystemState newState);
    
    // 获取当前状态
    SystemState getCurrentState() const;
    
    // 获取计数器值
    int getJunctionCounter() const;
    
    // 获取检测到的颜色
    ColorCode getDetectedColor() const;
    
    // 处理上位机命令
    void handleCommand(const char* command);
    
private:
    // 组件引用
    SensorManager& m_sensorManager;
    MotionController& m_motionController;
    RoboticArm& m_roboticArm;
    NavigationController& m_navigationController;
    AccurateTurn m_accurateTurn;
    
    // 状态相关变量
    SystemState m_currentState;
    LocateSubState m_locateSubState;
    uint8_t m_zoneCounter;
    uint8_t m_colorCounter;
    uint8_t m_blockCounter; // 物块计数器
    ColorCode m_detectedColorCode;
    unsigned long m_actionStartTime;
    
    // 使用位域节省内存
    struct {
        uint8_t m_isActionComplete : 1;
        uint8_t m_isTurning : 1;
    } m_flags;
    
    // 辅助函数
    void executeStateTransition(SystemState newState);
    void logStateTransition(SystemState oldState, SystemState newState);
    const char* systemStateToString(SystemState state);
    const char* junctionTypeToString(JunctionType type) const;
};

#endif // SIMPLE_STATE_MACHINE_H 
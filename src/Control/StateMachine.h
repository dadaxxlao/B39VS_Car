#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Arm/RoboticArm.h"
#include "../Control/NavigationController.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

// 前向声明
class SensorManager;
class MotionController;
class RoboticArm;
class NavigationController;

/**
 * 高层状态机，负责控制小车在不同任务阶段的行为
 * 与NavigationController协作，在路口处做出决策
 * 状态转换逻辑基于Design.md中的规范
 */
class StateMachine {
public:
    // 构造函数
    StateMachine(SensorManager& sm, MotionController& mc, 
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
    
    // 测试相关 - 声明友元，使TestStateMachine.cpp可以访问私有方法
    #ifdef TEST_STATE_MACHINE
    friend class TestableStateMachine;
    #endif
    
private:
    // 组件引用
    SensorManager& m_sensorManager;
    MotionController& m_motionController;
    RoboticArm& m_roboticArm;
    NavigationController& m_navigationController;
    
    // 状态相关变量
    SystemState m_currentState;      // 当前系统状态
    LocateSubState m_locateSubState; // 定位子状态
    int m_zoneCounter;               // 区域计数器
    int m_colorCounter;              // 颜色计数器
    ColorCode m_detectedColorCode;   // 检测到的颜色代码
    unsigned long m_actionStartTime; // 动作开始时间
    bool m_isActionComplete;         // 动作是否完成
    
    // 状态处理函数
    void handleInitialized();
    void handleObjectFind();
    void handleUltrasonicDetect();
    void handleZoneJudge();
    void handleZoneToBase();
    void handleObjectGrab();
    void handleObjectPlacing();
    void handleCountIntersection();
    void handleObjectRelease();
    void handleErgodicJudge();
    void handleBackObjectFind();
    void handleReturnBase();
    void handleBaseArrive();
    void handleEnd();
    void handleErrorState();
    
    // 辅助函数
    void checkUltrasonicDetection(); // 超声波检测辅助函数
    void logStateTransition(SystemState oldState, SystemState newState);
    const char* systemStateToString(SystemState state) const;
    
    // 新增：路口类型转字符串函数作为类的私有方法
    const char* junctionTypeToString(JunctionType type) const;
};

#endif // STATE_MACHINE_H 
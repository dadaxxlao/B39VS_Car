#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "../Utils/Config.h"
#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Arm/RoboticArm.h"
#include "LineDetector.h"

class StateMachine {
private:
    SystemState currentState;
    LocateSubState locateSubState;  // OBJECT_LOCATE的子状态
    
    SensorManager* sensorManager;
    MotionController* motionController;
    RoboticArm* roboticArm;
    LineDetector lineDetector;
    
    // 路口计数器
    int junctionCounter;
    
    // 检测到的颜色
    ColorCode detectedColor;
    
    // 各状态处理函数
    void handleInitialized();
    void handleObjectFind();
    void handleObjectGrab();
    void handleObjectLocate();
    void handleObjectPlacing();
    void handleReturnBase();
    void handleEnd();
    void handleErrorState();
    
    // 检测路口类型
    JunctionType detectJunction();
    
    // 执行路口动作
    void executeJunctionAction(int action);
    
    // 掉头动作
    void performUTurn();
    
    // 遇到错误的处理
    void handleError(const char* errorMsg);
    
public:
    StateMachine();
    
    // 初始化状态机，注入依赖
    void init(SensorManager* sensors, MotionController* motion, RoboticArm* arm);
    
    // 获取当前状态
    SystemState getCurrentState() const;
    
    // 获取当前子状态
    LocateSubState getLocateSubState() const;
    
    // 状态转换
    void transitionTo(SystemState newState);
    
    // 子状态转换
    void transitionLocateSubState(LocateSubState newSubState);
    
    // 主循环调用的状态处理函数
    void handleState();
    
    // 重置路口计数器
    void resetJunctionCounter();
    
    // 获取路口计数
    int getJunctionCounter() const;
    
    // 增加路口计数
    void incrementJunctionCounter();
};

#endif // STATE_MACHINE_H 
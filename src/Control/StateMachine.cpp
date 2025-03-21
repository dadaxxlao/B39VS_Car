#include "StateMachine.h"
#include "../Utils/Logger.h"

StateMachine::StateMachine() : 
    currentState(INITIALIZED), 
    locateSubState(TURN_AROUND),
    sensorManager(nullptr), 
    motionController(nullptr), 
    roboticArm(nullptr),
    junctionCounter(0),
    detectedColor(COLOR_UNKNOWN) {
}

void StateMachine::init(SensorManager* sensors, MotionController* motion, RoboticArm* arm) {
    sensorManager = sensors;
    motionController = motion;
    roboticArm = arm;
    Logger::info("状态机初始化完成");
}

SystemState StateMachine::getCurrentState() const {
    return currentState;
}

LocateSubState StateMachine::getLocateSubState() const {
    return locateSubState;
}

void StateMachine::transitionTo(SystemState newState) {
    if (newState == currentState) {
        return;
    }
    
    // 记录状态转换
    Logger::info("状态转换: %d -> %d", currentState, newState);
    
    // 根据需要在状态转换时执行特定操作
    if (currentState == OBJECT_FIND && newState == OBJECT_GRAB) {
        // 进入抓取状态前，停止运动
        motionController->stop();
    } else if (newState == OBJECT_LOCATE) {
        // 进入定位状态时，初始化子状态和路口计数器
        transitionLocateSubState(TURN_AROUND);
        resetJunctionCounter();
    }
    
    currentState = newState;
}

void StateMachine::transitionLocateSubState(LocateSubState newSubState) {
    if (newSubState == locateSubState) {
        return;
    }
    
    Logger::info("子状态转换: %d -> %d", locateSubState, newSubState);
    locateSubState = newSubState;
}

void StateMachine::handleState() {
    // 根据当前状态调用相应的处理函数
    switch (currentState) {
        case INITIALIZED:
            handleInitialized();
            break;
        case OBJECT_FIND:
            handleObjectFind();
            break;
        case OBJECT_GRAB:
            handleObjectGrab();
            break;
        case OBJECT_LOCATE:
            handleObjectLocate();
            break;
        case OBJECT_PLACING:
            handleObjectPlacing();
            break;
        case RETURN_BASE:
            handleReturnBase();
            break;
        case END:
            handleEnd();
            break;
        case ERROR_STATE:
            handleErrorState();
            break;
        default:
            handleError("未知状态");
            break;
    }
}

JunctionType StateMachine::detectJunction() {
    // 使用LineDetector分析传感器数据
    const uint16_t* sensorValues = sensorManager->getInfraredSensorValues();
    return lineDetector.detectJunction(sensorValues);
}

void StateMachine::executeJunctionAction(int action) {
    // 根据动作码执行相应操作
    switch (action) {
        case ACTION_MOVE_FORWARD:
            motionController->moveForward(FOLLOW_SPEED);
            break;
        case ACTION_TURN_LEFT:
            motionController->turnLeft(TURN_SPEED);
            break;
        case ACTION_TURN_RIGHT:
            motionController->turnRight(TURN_SPEED);
            break;
        case ACTION_U_TURN:
            performUTurn();
            break;
        case ACTION_STOP:
            motionController->stop();
            break;
        case ACTION_GRAB:
            roboticArm->grab();
            break;
        case ACTION_PLACE:
            roboticArm->release();
            break;
        default:
            Logger::warning("未知动作码: %d", action);
            break;
    }
}

void StateMachine::performUTurn() {
    Logger::info("执行掉头动作");
    motionController->stop();
    delay(200);
    
    // 执行掉头动作
    motionController->turnLeft(SHARP_TURN_SPEED);
    delay(1000); // 根据实际电机速度调整
    
    // 等待中央红外检测到线
    while (!sensorManager->isLineDetected()) {
        delay(10);
    }
    
    motionController->stop();
    delay(200);
}

void StateMachine::resetJunctionCounter() {
    junctionCounter = 0;
}

int StateMachine::getJunctionCounter() const {
    return junctionCounter;
}

void StateMachine::incrementJunctionCounter() {
    junctionCounter++;
    Logger::debug("路口计数增加: %d", junctionCounter);
}

void StateMachine::handleError(const char* errorMsg) {
    Logger::error("状态机错误: %s", errorMsg);
    transitionTo(ERROR_STATE);
}

// 状态处理函数实现
void StateMachine::handleInitialized() {
    // 初始化状态的处理
    if (!sensorManager || !motionController || !roboticArm) {
        handleError("依赖未注入");
        return;
    }
    
    // 这里可以等待上位机信号或某个按钮按下
    // 简单起见，这里直接转到OBJECT_FIND状态
    // 实际情况可能需要等待某个事件或信号
    
    // 初始化完成，转到寻找物块状态
    transitionTo(OBJECT_FIND);
}

void StateMachine::handleObjectFind() {
    // 寻找物块状态
    // 1. 检测路口类型
    JunctionType junction = detectJunction();
    
    // 2. 根据状态和路口类型获取动作决策
    int action = lineDetector.getActionForJunction(junction, OBJECT_FIND);
    
    // 3. 执行动作
    executeJunctionAction(action);
    
    // 4. 如果是左T字路口，需要检测物块
    if (junction == T_LEFT) {
        // 在左转之后，检测是否有物块
        delay(500); // 等待转弯完成
        
        float distance = sensorManager->getUltrasonicDistance();
        if (distance <= NO_OBJECT_THRESHOLD) {
            // 检测到物块，转到抓取状态
            Logger::info("检测到物块，距离: %.2f cm", distance);
            transitionTo(OBJECT_GRAB);
        } else {
            // 未检测到物块，掉头
            Logger::info("未检测到物块，执行掉头");
            performUTurn();
        }
    }
    
    // 5. 如果是正常巡线，按照PID控制线位置
    if (junction == NO_JUNCTION) {
        int position = sensorManager->getLinePosition();
        motionController->followLine(position);
    }
}

void StateMachine::handleObjectGrab() {
    // 抓取物块状态
    motionController->stop();
    
    // 获取超声波距离
    float distance = sensorManager->getUltrasonicDistance();
    
    // 根据距离调整位置
    if (distance > GRAB_DISTANCE + 2.0) {
        // 距离太远，缓慢接近
        motionController->moveForward(FOLLOW_SPEED / 2);
    } else if (distance < GRAB_DISTANCE - 2.0) {
        // 距离太近，略微后退
        motionController->moveBackward(FOLLOW_SPEED / 2);
    } else {
        // 距离合适，停止并抓取
        motionController->stop();
        
        // 执行抓取动作
        roboticArm->grab();
        
        // 读取物块颜色
        detectedColor = sensorManager->getColor();
        Logger::info("物块颜色: %d", detectedColor);
        
        // 转到定位状态
        transitionTo(OBJECT_LOCATE);
    }
}

void StateMachine::handleObjectLocate() {
    // 定位放置区域状态
    
    switch (locateSubState) {
        case TURN_AROUND:
            // 执行掉头动作
            performUTurn();
            // 掉头完成后，进入路口计数子状态
            transitionLocateSubState(COUNT_JUNCTIONS);
            break;
            
        case COUNT_JUNCTIONS:
            // 检测路口并计数
            {
                JunctionType junction = detectJunction();
                
                // 如果检测到路口，增加计数并处理
                static JunctionType lastJunction = NO_JUNCTION;
                
                if (junction != NO_JUNCTION && junction != lastJunction) {
                    // 新的路口，增加计数
                    incrementJunctionCounter();
                    
                    // 根据路口计数决定动作
                    if (junctionCounter == 1 && junction == T_FORWARD) {
                        // 第一个倒T字路口右转
                        motionController->turnRight(TURN_SPEED);
                    } else if ((junctionCounter == 2 || junctionCounter == 3) && junction == T_LEFT) {
                        // 第二、三个左T字路口左转
                        motionController->turnLeft(TURN_SPEED);
                    } else if (junction == T_RIGHT) {
                        // 检查是否到达目标路口
                        int targetJunction = lineDetector.getTargetJunctionCount(detectedColor);
                        if (junctionCounter >= targetJunction + 3) { // +3是因为前面已经计数了3个路口
                            transitionLocateSubState(SELECT_TARGET);
                        } else {
                            // 未到达目标路口，继续直行
                            motionController->moveForward(FOLLOW_SPEED);
                        }
                    } else {
                        // 其他路口，继续前进
                        motionController->moveForward(FOLLOW_SPEED);
                    }
                    
                    // 更新上一个路口类型
                    lastJunction = junction;
                } else if (junction == NO_JUNCTION) {
                    // 正常巡线
                    int position = sensorManager->getLinePosition();
                    motionController->followLine(position);
                    lastJunction = NO_JUNCTION;
                }
            }
            break;
            
        case SELECT_TARGET:
            // 选择目标路口
            {
                // 执行右转进入放置区
                motionController->turnRight(TURN_SPEED);
                delay(500); // 等待转弯完成
                
                // 前进一段距离
                motionController->moveForward(FOLLOW_SPEED);
                delay(1000); // 根据实际情况调整
                
                // 转到放置状态
                transitionTo(OBJECT_PLACING);
            }
            break;
    }
}

void StateMachine::handleObjectPlacing() {
    // 放置物块状态
    motionController->stop();
    
    // 放置物块
    roboticArm->release();
    
    // 稍微后退
    motionController->moveBackward(FOLLOW_SPEED);
    delay(500);
    
    // 转到返回基地状态
    transitionTo(RETURN_BASE);
}

void StateMachine::handleReturnBase() {
    // 返回基地状态
    
    // 先执行掉头
    static bool hasTurnedAround = false;
    
    if (!hasTurnedAround) {
        performUTurn();
        hasTurnedAround = true;
        return;
    }
    
    // 检测路口
    JunctionType junction = detectJunction();
    
    // 判断是否所有传感器都检测到线（到达基地）
    const uint16_t* sensors = sensorManager->getInfraredSensorValues();
    bool allTriggered = true;
    for (int i = 0; i < 8; i++) {
        if (sensors[i] == 0) {
            allTriggered = false;
            break;
        }
    }
    
    if (allTriggered) {
        // 所有传感器都检测到线，说明到达基地
        motionController->stop();
        transitionTo(END);
        return;
    }
    
    // 根据路口类型决定动作
    int action = lineDetector.getActionForJunction(junction, RETURN_BASE);
    executeJunctionAction(action);
}

void StateMachine::handleEnd() {
    // 任务结束状态
    motionController->stop();
    Logger::info("任务完成");
    
    // 可以添加一些结束动作，如LED闪烁等
    
    // 这里可以选择是否重新开始任务
    // transitionTo(INITIALIZED);
}

void StateMachine::handleErrorState() {
    // 错误状态处理
    motionController->stop();
    
    // 这里可以添加一些错误指示，如LED闪烁等
    
    // 等待一段时间后尝试恢复
    delay(1000);
    transitionTo(INITIALIZED);
} 
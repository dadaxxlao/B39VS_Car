#include "LineDetector.h"
#include "../Utils/Logger.h"

LineDetector::LineDetector() 
    : currentState(NO_JUNCTION),
      lastDetectionTime(0),
      lostLineStartTime(0),
      junctionConfirmTime(300) { // 300ms确认时间
}

JunctionType LineDetector::detectJunction(const uint16_t* sensorValues) {
    // 传感器布局（从左到右）:
    // [0][1][2][3][4][5][6][7]
    // 数字量传感器: 0 = 检测到黑线, 1 = 检测到白线
    
    // 当前时间
    unsigned long currentTime = millis();
    
    // 分析当前传感器状态
    bool leftDetected = (sensorValues[0] == 0 || sensorValues[1] == 0);
    bool centerDetected = (sensorValues[3] == 0 || sensorValues[4] == 0);
    bool rightDetected = (sensorValues[6] == 0 || sensorValues[7] == 0);
    bool allWhite = true;
    for(int i = 0; i < 8; i++) {
        if(sensorValues[i] == 0) {
            allWhite = false;
            break;
        }
    }
    
    // 状态机处理
    switch(currentState) {
        case NO_JUNCTION:
            if(leftDetected && centerDetected && !rightDetected) {
                // 可能遇到左转弯或左T字路口
                potentialJunction = LEFT_TURN;
                currentState = POTENTIAL_LEFT;
                lastDetectionTime = currentTime;
                Logger::info("检测到潜在左转/左T字路口");
            } else if(!leftDetected && centerDetected && rightDetected) {
                // 可能遇到右转弯或右T字路口
                potentialJunction = RIGHT_TURN;
                currentState = POTENTIAL_RIGHT;
                lastDetectionTime = currentTime;
                Logger::info("检测到潜在右转/右T字路口");
            }
            break;
            
        case POTENTIAL_LEFT:
            if(allWhite) {
                // 出现完全丢线，可能是左转弯
                lostLineStartTime = currentTime;
                currentState = LOST_LINE_LEFT;
                Logger::info("左转特征：完全丢线");
            } else if(currentTime - lastDetectionTime > junctionConfirmTime) {
                // 持续检测到中心线，可能是左T字路口
                currentState = CONFIRMED_T_LEFT;
                Logger::info("确认为左T字路口");
                return T_LEFT;
            }
            break;
            
        case POTENTIAL_RIGHT:
            if(allWhite) {
                // 出现完全丢线，可能是右转弯
                lostLineStartTime = currentTime;
                currentState = LOST_LINE_RIGHT;
                Logger::info("右转特征：完全丢线");
            } else if(currentTime - lastDetectionTime > junctionConfirmTime) {
                // 持续检测到中心线，可能是右T字路口
                currentState = CONFIRMED_T_RIGHT;
                Logger::info("确认为右T字路口");
                return T_RIGHT;
            }
            break;
            
        case LOST_LINE_LEFT:
            if(centerDetected && !leftDetected) {
                // 重新检测到中心线，完成左转弯
                currentState = NO_JUNCTION;
                Logger::info("左转弯完成");
                return LEFT_TURN;
            } else if(currentTime - lostLineStartTime > 1000) {
                // 超时处理
                currentState = NO_JUNCTION;
                Logger::warning("左转弯检测超时");
            }
            break;
            
        case LOST_LINE_RIGHT:
            if(centerDetected && !rightDetected) {
                // 重新检测到中心线，完成右转弯
                currentState = NO_JUNCTION;
                Logger::info("右转弯完成");
                return RIGHT_TURN;
            } else if(currentTime - lostLineStartTime > 1000) {
                // 超时处理
                currentState = NO_JUNCTION;
                Logger::warning("右转弯检测超时");
            }
            break;
            
        case CONFIRMED_T_LEFT:
        case CONFIRMED_T_RIGHT:
            // 已确认的T字路口，重置状态
            currentState = NO_JUNCTION;
            break;
    }
    
    // 超时重置状态
    if(currentState != NO_JUNCTION && currentTime - lastDetectionTime > 2000) {
        currentState = NO_JUNCTION;
        Logger::warning("路口检测超时，重置状态");
    }
    
    return ::NO_JUNCTION;  // 使用全局作用域的NO_JUNCTION
}

JunctionType LineDetector::analyzeJunction(const uint16_t* sensorValues) {
    // 这个方法现在只被detectJunction调用
    return detectJunction(sensorValues);
}

int LineDetector::getActionForJunction(JunctionType junction, SystemState state, ColorCode color) {
    // 基于当前状态和路口类型决定行动策略
    
    switch (state) {
        case OBJECT_FIND:
            // 寻找物体状态 - 左转优先策略
            switch (junction) {
                case T_LEFT:
                    return ACTION_TURN_LEFT;  // 左T字路口左转
                
                case T_RIGHT:
                    return ACTION_MOVE_FORWARD; // 右T字路口直行
                
                case T_FORWARD:
                    return ACTION_TURN_LEFT;  // 倒T字路口左转
                
                case CROSS:
                    return ACTION_TURN_LEFT;  // 十字路口左转
                
                case LEFT_TURN:
                    return ACTION_TURN_LEFT;  // 左转弯路口左转
                
                case RIGHT_TURN:
                    return ACTION_TURN_RIGHT; // 右转弯路口右转
                
                case END_OF_LINE:
                    return ACTION_U_TURN;     // 线路终点掉头
                
                default:
                    return ACTION_MOVE_FORWARD; // 无路口直行
            }
            break;
            
        case OBJECT_LOCATE:
            // 定位放置区域状态 - 根据路口计数和颜色决定
            // 这部分逻辑需要与StateMachine的junctionCounter配合
            switch (junction) {
                case T_LEFT:
                    return ACTION_TURN_LEFT;  // 通常左T字路口左转
                
                case T_RIGHT:
                    // 可能需要根据颜色和计数判断是否需要右转进入放置区
                    return ACTION_MOVE_FORWARD; // 默认直行
                
                case T_FORWARD:
                    return ACTION_TURN_RIGHT; // 第一个倒T字路口右转
                
                default:
                    return ACTION_MOVE_FORWARD;
            }
            break;
            
        case RETURN_BASE:
            // 返回基地状态
            switch (junction) {
                case T_LEFT:
                    return ACTION_TURN_LEFT;  // 第一个路口左转
                
                case T_RIGHT:
                case T_FORWARD:
                case CROSS:
                    return ACTION_MOVE_FORWARD; // 其他路口直行
                
                case END_OF_LINE:
                    return ACTION_STOP;       // 到达基地停止
                
                default:
                    return ACTION_MOVE_FORWARD;
            }
            break;
            
        default:
            // 其他状态默认动作
            if (junction == END_OF_LINE) {
                return ACTION_STOP;  // 线路终点停止
            } else if (junction == NO_JUNCTION) {
                return ACTION_MOVE_FORWARD; // 无路口直行
            } else {
                return ACTION_MOVE_FORWARD; // 默认直行
            }
    }
}

int LineDetector::getTargetJunctionCount(ColorCode color) {
    // 根据颜色返回目标路口计数
    // 红色=1，蓝色=2，黄色=3，白色=4，黑色=5
    switch (color) {
        case COLOR_RED:    return 1;
        case COLOR_BLUE:   return 2;
        case COLOR_YELLOW: return 3;
        case COLOR_WHITE:  return 4;
        case COLOR_BLACK:  return 5;
        default:           return 0;
    }
} 
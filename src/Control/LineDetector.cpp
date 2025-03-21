#include "LineDetector.h"
#include "../Utils/Logger.h"

LineDetector::LineDetector() {
}

JunctionType LineDetector::detectJunction(const uint16_t* sensorValues) {
    // 分析传感器数据判断路口类型
    return analyzeJunction(sensorValues);
}

JunctionType LineDetector::analyzeJunction(const uint16_t* sensorValues) {
    // 传感器布局（从左到右）:
    // [0][1][2][3][4][5][6][7]
    
    // 计算检测到线的传感器数量
    int lineCount = 0;
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] == 1) {
            lineCount++;
        }
    }
    
    // 简单的路口判断逻辑
    // 注意：这里的逻辑需要根据实际传感器安装和小车运动调整
    
    // 没有检测到线
    if (lineCount == 0) {
        return END_OF_LINE;
    }
    
    // 正常巡线（只有中间传感器检测到线）
    if (lineCount <= 3 && sensorValues[3] == 1 && sensorValues[4] == 1) {
        return NO_JUNCTION;
    }
    
    // 左T字路口 - 左侧和前侧有线
    if (sensorValues[0] == 1 && sensorValues[1] == 1 && 
        sensorValues[3] == 1 && sensorValues[4] == 1 && 
        sensorValues[6] == 0 && sensorValues[7] == 0) {
        return T_LEFT;
    }
    
    // 右T字路口 - 右侧和前侧有线
    if (sensorValues[0] == 0 && sensorValues[1] == 0 && 
        sensorValues[3] == 1 && sensorValues[4] == 1 && 
        sensorValues[6] == 1 && sensorValues[7] == 1) {
        return T_RIGHT;
    }
    
    // 倒T字路口（正T） - 前侧有线，两侧没有线
    if (sensorValues[0] == 0 && sensorValues[1] == 0 &&
        sensorValues[3] == 1 && sensorValues[4] == 1 &&
        sensorValues[6] == 0 && sensorValues[7] == 0) {
        return T_FORWARD;
    }
    
    // 左转弯路口
    if (sensorValues[0] == 1 && sensorValues[1] == 1 && 
        sensorValues[3] == 1 && sensorValues[4] == 0) {
        return LEFT_TURN;
    }
    
    // 右转弯路口
    if (sensorValues[3] == 0 && sensorValues[4] == 1 && 
        sensorValues[6] == 1 && sensorValues[7] == 1) {
        return RIGHT_TURN;
    }
    
    // 十字路口 - 左侧、前侧和右侧都有线
    if (sensorValues[0] == 1 && sensorValues[1] == 1 && 
        sensorValues[3] == 1 && sensorValues[4] == 1 && 
        sensorValues[6] == 1 && sensorValues[7] == 1) {
        return CROSS;
    }
    
    // 默认情况
    return NO_JUNCTION;
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
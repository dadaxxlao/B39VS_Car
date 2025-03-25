#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#include "../Utils/Config.h"

class LineDetector {
private:
    // 路口检测状态机状态
    enum DetectionState {
        NO_JUNCTION,        // 无路口
        POTENTIAL_LEFT,     // 潜在左转/左T字
        POTENTIAL_RIGHT,    // 潜在右转/右T字
        LOST_LINE_LEFT,     // 左转丢线状态
        LOST_LINE_RIGHT,    // 右转丢线状态
        CONFIRMED_T_LEFT,   // 确认左T字
        CONFIRMED_T_RIGHT   // 确认右T字
    };
    
    // 状态机变量
    DetectionState currentState;
    JunctionType potentialJunction;
    unsigned long lastDetectionTime;
    unsigned long lostLineStartTime;
    const unsigned long junctionConfirmTime;  // 路口确认时间阈值
    
    // 分析传感器数据确定路口类型
    JunctionType analyzeJunction(const uint16_t* sensorValues);
    
public:
    LineDetector();
    
    // 检测路口类型
    JunctionType detectJunction(const uint16_t* sensorValues);
    
    // 根据当前系统状态获取应该采取的动作决策
    // 不同状态下对同一路口类型可能有不同处理策略
    int getActionForJunction(JunctionType junction, SystemState state, ColorCode color = COLOR_UNKNOWN);
    
    // 根据颜色代码获取目标区域的路口数
    int getTargetJunctionCount(ColorCode color);
};

// 路口动作类型
#define ACTION_MOVE_FORWARD    0
#define ACTION_TURN_LEFT       1
#define ACTION_TURN_RIGHT      2
#define ACTION_U_TURN          3
#define ACTION_STOP            4
#define ACTION_GRAB            5
#define ACTION_PLACE           6

#endif // LINE_DETECTOR_H 
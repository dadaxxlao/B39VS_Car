#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#include "../Utils/Config.h"
#include "LineFollower.h"

class LineDetector {
private:
    // 路口检测状态机状态 - 这部分将被NavigationController替代
    /*
    enum DetectionState {
        NO_JUNCTION,        // 无路口
        POTENTIAL_LEFT,     // 潜在左转/左T字
        POTENTIAL_RIGHT,    // 潜在右转/右T字
        LOST_LINE_LEFT,     // 左转丢线状态
        LOST_LINE_RIGHT,    // 右转丢线状态
        CONFIRMED_T_LEFT,   // 确认左T字
        CONFIRMED_T_RIGHT   // 确认右T字
    };
    */
    
    // 状态机变量 - 这部分将被NavigationController替代
    /*
    DetectionState currentState;
    JunctionType potentialJunction;
    unsigned long lastDetectionTime;
    unsigned long lostLineStartTime;
    const unsigned long junctionConfirmTime;  // 路口确认时间阈值
    */
    
    // 分析传感器数据确定路口类型 - 内部方法，更高层应用中不再需要
    // JunctionType analyzeJunction(const uint16_t* sensorValues);
    
public:
    LineDetector();
    
    // 检测路口类型 - 这部分将被NavigationController替代
    // JunctionType detectJunction(const uint16_t* sensorValues);
    
    // 新增：静态分类方法，用于停车后的路口类型判断
    JunctionType classifyStoppedJunction(const uint16_t* staticSensorValues, LineFollower::TriggerType triggerType);
    
    // 新增：判断是否为T_FORWARD
    bool isForwardTee(const uint16_t* sensorValues);
    
};

#endif // LINE_DETECTOR_H 
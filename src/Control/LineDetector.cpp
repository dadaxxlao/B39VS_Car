#include "LineDetector.h"
#include "../Utils/Logger.h"

LineDetector::LineDetector() {
    // 简化构造函数，移除状态机相关变量
}

// 移除旧的动态路口检测方法
// JunctionType LineDetector::detectJunction(const uint16_t* sensorValues) {
//    ... 移除旧方法内容 ...
// }

// 移除旧的分析方法
// JunctionType LineDetector::analyzeJunction(const uint16_t* sensorValues) {
//    return detectJunction(sensorValues);
// }

// 新增：判断是否为T_FORWARD（全黑模式）
bool LineDetector::isForwardTee(const uint16_t* sensorValues) {
    // 检查是否所有传感器都是黑色（检测到黑线）
    bool allBlack = true;
    for (int i = 0; i < 8; i++) {
        if (sensorValues[i] != 0) {
            allBlack = false;
            break;
        }
    }
    
    char sensorStr[40];
    snprintf(sensorStr, sizeof(sensorStr), "[%d%d%d%d%d%d%d%d]",
             sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[3],
             sensorValues[4], sensorValues[5], sensorValues[6], sensorValues[7]);
    
    Logger::debug("LineDet", "isForwardTee检查: 传感器=%s -> 结果=%s", 
                 sensorStr, allBlack ? "是" : "否");
    
    if (allBlack) {
        Logger::info("LineDet", "检测到T_FORWARD（全黑线模式）");
        return true;
    }
    
    return false;
}

// 新增：静态路口分类方法
JunctionType LineDetector::classifyStoppedJunction(const uint16_t* staticSensorValues, LineFollower::TriggerType triggerType) {
    // 检查是否为中心模式 (xxx00xxx)
    bool centerMode = (staticSensorValues[2] == 0|| staticSensorValues[3] == 0 || staticSensorValues[4] == 0 || staticSensorValues[5] == 0);
    
    // 检查是否为全白模式 (11111111)
    bool allWhite = true;
    for (int i = 0; i < 8; i++) {
        if (staticSensorValues[i] == 0) {
            allWhite = false;
            break;
        }
    }
    
    // 格式化传感器数组为字符串用于日志
    char sensorStr[40];
    snprintf(sensorStr, sizeof(sensorStr), "[%d%d%d%d%d%d%d%d]",
             staticSensorValues[0], staticSensorValues[1], staticSensorValues[2], staticSensorValues[3],
             staticSensorValues[4], staticSensorValues[5], staticSensorValues[6], staticSensorValues[7]);
    
    // 记录分类开始的日志
    Logger::debug("LineDet", "静态分类: 传感器=%s, 触发类型=%d, 中心模式=%s, 全白=%s",
                 sensorStr, triggerType, centerMode ? "是" : "否", allWhite ? "是" : "否");
    
    JunctionType resultJunctionType = NO_JUNCTION;
    
    // 根据触发类型和传感器模式分类
    if (triggerType == LineFollower::TRIGGER_LEFT_EDGE) {
        if (centerMode) {
            resultJunctionType = T_LEFT;
            Logger::info("LineDet", "分类结果: T_LEFT（左边缘触发+中心线）");
        } else if (allWhite) {
            resultJunctionType = LEFT_TURN;
            Logger::info("LineDet", "分类结果: LEFT_TURN（左边缘触发+全白）");
        }
    } else if (triggerType == LineFollower::TRIGGER_RIGHT_EDGE) {
        if (centerMode) {
            resultJunctionType = T_RIGHT;
            Logger::info("LineDet", "分类结果: T_RIGHT（右边缘触发+中心线）");
        } else if (allWhite) {
            resultJunctionType = RIGHT_TURN;
            Logger::info("LineDet", "分类结果: RIGHT_TURN（右边缘触发+全白）");
        }
    }
    
    // 如果无法分类，记录警告
    if (resultJunctionType == NO_JUNCTION) {
        Logger::warning("LineDet", "无法分类路口类型，返回NO_JUNCTION");
    }
    
    // 记录最终分类结果的详细日志
    Logger::debug("LineDet", "静态分类结果: 传感器=%s, 触发=%d -> 路口类型=%d", 
                 sensorStr, triggerType, resultJunctionType);
                 
    return resultJunctionType;
}


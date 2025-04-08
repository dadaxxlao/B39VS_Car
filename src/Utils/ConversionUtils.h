#ifndef CONVERSION_UTILS_H
#define CONVERSION_UTILS_H

#include "Config.h"
#include "../Control/NavigationController.h"
#include "../Sensor/SensorManager.h"

// 状态转字符串函数 - 使用inline
inline const char* systemStateToString(SystemState state) {
  switch (state) {
    case INITIALIZED: return "初始化完成";
    case OBJECT_FIND: return "寻找物块";
    case ULTRASONIC_DETECT: return "超声波检测";
    case ZONE_JUDGE: return "区域判断";
    case ZONE_TO_BASE: return "返回基地";
    case OBJECT_GRAB: return "抓取物块";
    case OBJECT_PLACING: return "放置物块";
    case COUNT_INTERSECTION: return "计数路口";
    case OBJECT_RELEASE: return "释放物块";
    case ERGODIC_JUDGE: return "遍历判断";
    case BACK_OBJECT_FIND: return "返回寻找物块";
    case RETURN_BASE: return "返回基地";
    case BASE_ARRIVE: return "到达基地";
    case END: return "任务完成";
    case ERROR_STATE: return "错误状态";
    default: return "未知状态";
  }
}

inline const char* navigationStateToString(NavigationState state) {
  switch (state) {
    case NAV_FOLLOWING_LINE: return "循线中";
    case NAV_AT_JUNCTION: return "在路口";
    case NAV_MOVING_TO_STOP: return "移动到停止";
    case NAV_STOPPED_FOR_CHECK: return "停止检查";
    case NAV_STOPPED: return "已停止";
    case NAV_ERROR: return "导航错误";
    default: return "未知";
  }
}

inline const char* colorCodeToString(ColorCode color) {
  switch (color) {
    case COLOR_RED: return "红色";
    case COLOR_BLUE: return "蓝色";
    case COLOR_YELLOW: return "黄色";
    case COLOR_BLACK: return "黑色";
    case COLOR_WHITE: return "白色";
    case COLOR_UNKNOWN: return "未知颜色";
    default: return "未定义";
  }
}

inline const char* sensorStatusToString(SensorStatus status) {
  switch (status) {
    case SensorStatus::OK: return "正常";
    case SensorStatus::NOT_INITIALIZED: return "未初始化";
    case SensorStatus::UNKNOWN: return "未知";
    default: return "未定义";
  }
}

#endif // CONVERSION_UTILS_H 
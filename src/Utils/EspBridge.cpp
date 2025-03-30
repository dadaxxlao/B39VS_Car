#include "EspBridge.h"

// 命令处理函数前向声明
static bool handleStartCommand(const char* params);
static bool handleStopCommand(const char* params);
static bool handleResetCommand(const char* params);
static bool handleConfigCommand(const char* params);
static bool handleGetStatusCommand(const char* params);
static bool handleCustomMotionCommand(const char* params);

// 命令处理函数映射表
EspBridge::CommandMapping EspBridge::commandHandlers[] = {
    { CMD_START, "START", handleStartCommand },
    { CMD_STOP, "STOP", handleStopCommand },
    { CMD_RESET, "RESET", handleResetCommand },
    { CMD_CONFIG, "CONFIG", handleConfigCommand },
    { CMD_GET_STATUS, "GET_STATUS", handleGetStatusCommand },
    { CMD_CUSTOM_MOTION, "CUSTOM_MOTION", handleCustomMotionCommand }
};

// 命令处理函数数量
const int EspBridge::commandHandlersCount = sizeof(EspBridge::commandHandlers) / sizeof(EspBridge::CommandMapping);

// 全局ESP桥接实例
EspBridge EspComm;

// 系统当前状态（示例，实际项目中应根据实际状态更新）
static SystemStateCode currentState = STATE_IDLE;

// 示例：开始命令处理函数
static bool handleStartCommand(const char* params) {
    // 在此处理开始命令
    Logger::info("EspBridgeCmd", "收到开始命令");
    
    // 更新系统状态
    currentState = STATE_RUNNING;
    
    // 发送状态更新
    EspComm.sendState(currentState, "任务已开始");
    
    return true;
}

// 示例：停止命令处理函数
static bool handleStopCommand(const char* params) {
    // 在此处理停止命令
    Logger::info("EspBridgeCmd", "收到停止命令");
    
    // 更新系统状态
    currentState = STATE_IDLE;
    
    // 发送状态更新
    EspComm.sendState(currentState, "任务已停止");
    
    return true;
}

// 示例：重置命令处理函数
static bool handleResetCommand(const char* params) {
    // 在此处理重置命令
    Logger::info("EspBridgeCmd", "收到重置命令");
    
    // 更新系统状态
    currentState = STATE_IDLE;
    
    // 发送状态更新
    EspComm.sendState(currentState, "系统已重置");
    
    return true;
}

// 示例：配置命令处理函数
static bool handleConfigCommand(const char* params) {
    // 在此处理配置命令
    if (!params) {
        Logger::warning("EspBridgeCmd", "配置命令缺少参数");
        return false;
    }
    
    Logger::info("EspBridgeCmd", "收到配置命令: %s", params);
    
    // 解析配置参数
    // 示例: "speed=100,mode=1"
    
    // 发送状态更新
    EspComm.sendState(currentState, "配置已更新");
    
    return true;
}

// 示例：获取状态命令处理函数
static bool handleGetStatusCommand(const char* params) {
    // 在此处理获取状态命令
    Logger::info("EspBridgeCmd", "收到获取状态命令");
    
    // 发送当前状态
    EspComm.sendState(currentState, "正常运行中");
    
    // 发送一些示例数据
    char sensorData[64];
    sprintf(sensorData, "temp=25.5,humid=60,distance=150");
    EspComm.sendData(DATA_SENSORS, sensorData);
    
    return true;
}

// 示例：自定义运动命令处理函数
static bool handleCustomMotionCommand(const char* params) {
    // 在此处理自定义运动命令
    if (!params) {
        Logger::warning("EspBridgeCmd", "自定义运动命令缺少参数");
        return false;
    }
    
    Logger::info("EspBridgeCmd", "收到自定义运动命令: %s", params);
    
    // 解析运动参数
    // 示例: "direction=forward,speed=150,time=1000"
    
    // 发送状态更新
    EspComm.sendState(currentState, "执行自定义运动");
    
    return true;
} 
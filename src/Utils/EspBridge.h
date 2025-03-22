#ifndef ESP_BRIDGE_H
#define ESP_BRIDGE_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Config.h"
#include "Logger.h"

// 导入共享通信协议的定义
#include "../../shared/CommProtocol.h"

/**
 * ESP桥接类
 * 用于Arduino与ESP32之间的串口通信
 */
class EspBridge {
private:
    SoftwareSerial* espSerial;
    bool initialized;
    
    // 命令解析缓冲区
    static const int ESP_BUFFER_SIZE = 256;
    char buffer[ESP_BUFFER_SIZE];
    int bufferIndex;
    
    // 命令处理函数类型
    typedef bool (*CommandHandler)(const char* params);
    
    // 命令处理函数映射结构
    struct CommandMapping {
        CommandType cmdType;
        const char* cmdName;
        CommandHandler handler;
    };
    
    // 命令处理函数映射表
    static CommandMapping commandHandlers[];
    static const int commandHandlersCount;
    
public:
    EspBridge() : initialized(false), bufferIndex(0) {
        memset(buffer, 0, ESP_BUFFER_SIZE);
    }
    
    /**
     * 初始化ESP桥接
     * @param rxPin Arduino接收引脚
     * @param txPin Arduino发送引脚
     * @param baudRate 波特率
     * @return 是否初始化成功
     */
    bool begin(int rxPin, int txPin, long baudRate = 115200) {
        if (!ENABLE_ESP) {
            return false;
        }
        
        espSerial = new SoftwareSerial(rxPin, txPin);
        espSerial->begin(baudRate);
        
        // 简单测试ESP32是否就绪
        espSerial->println("$PING#");
        delay(500);
        
        initialized = true;
        
        // 设置Logger的ESP流
        setupLogger();
        
        return initialized;
    }
    
    /**
     * 设置Logger的ESP流
     */
    void setupLogger() {
        if (ENABLE_ESP && initialized && espSerial) {
            Logger::setStream(COMM_ESP, espSerial);
            Logger::enableComm(COMM_ESP, true);
        }
    }
    
    /**
     * 检查是否初始化
     * @return 是否初始化成功
     */
    bool isInitialized() const {
        return ENABLE_ESP && initialized;
    }
    
    /**
     * 检查是否有可用数据
     * @return 是否有可用数据
     */
    bool available() {
        return ENABLE_ESP && espSerial && espSerial->available();
    }
    
    /**
     * 发送系统状态
     * @param stateCode 状态码
     * @param params 附加参数，可为nullptr
     */
    void sendState(SystemStateCode stateCode, const char* params = nullptr) {
        if (!ENABLE_ESP || !espSerial) return;
        
        espSerial->print(PREFIX_STATE);
        espSerial->print((int)stateCode);
        
        if (params != nullptr) {
            espSerial->print(",");
            espSerial->print(params);
        }
        
        espSerial->print(MSG_TERMINATOR);
    }
    
    /**
     * 发送日志消息
     * @param level 日志级别
     * @param message 日志消息
     */
    void sendLog(LogLevel level, const char* message) {
        if (!ENABLE_ESP || !espSerial) return;
        
        const char* levelStr;
        switch (level) {
            case LOG_ERROR:
                levelStr = "ERROR";
                break;
            case LOG_WARNING:
                levelStr = "WARNING";
                break;
            case LOG_INFO:
                levelStr = "INFO";
                break;
            case LOG_DEBUG:
                levelStr = "DEBUG";
                break;
            default:
                levelStr = "UNKNOWN";
                break;
        }
        
        espSerial->print(PREFIX_LOG);
        espSerial->print(levelStr);
        espSerial->print(",");
        espSerial->print(message);
        espSerial->print(MSG_TERMINATOR);
    }
    
    /**
     * 发送传感器数据
     * @param dataType 数据类型
     * @param dataStr 数据字符串
     */
    void sendData(DataType dataType, const char* dataStr) {
        if (!ENABLE_ESP || !espSerial) return;
        
        const char* typeStr;
        switch (dataType) {
            case DATA_SENSORS:
                typeStr = "SENSORS";
                break;
            case DATA_POSITION:
                typeStr = "POSITION";
                break;
            case DATA_BATTERY:
                typeStr = "BATTERY";
                break;
            case DATA_CUSTOM:
                typeStr = "CUSTOM";
                break;
            default:
                typeStr = "UNKNOWN";
                break;
        }
        
        espSerial->print(PREFIX_DATA);
        espSerial->print(typeStr);
        espSerial->print(",");
        espSerial->print(dataStr);
        espSerial->print(MSG_TERMINATOR);
    }
    
    /**
     * 处理接收到的数据
     * @return 是否接收到完整命令
     */
    bool processReceivedData() {
        if (!ENABLE_ESP || !espSerial || !espSerial->available()) {
            return false;
        }
        
        // 读取数据到缓冲区
        while (espSerial->available() && bufferIndex < ESP_BUFFER_SIZE - 1) {
            char c = espSerial->read();
            
            // 检测到消息结束标记
            if (c == MSG_TERMINATOR) {
                if (bufferIndex > 0) {
                    buffer[bufferIndex] = '\0';
                    bool result = parseCommand(buffer);
                    bufferIndex = 0;
                    return result;
                }
            } else {
                buffer[bufferIndex++] = c;
            }
        }
        
        return false;
    }
    
    /**
     * 解析并处理命令
     * @param cmdStr 命令字符串
     * @return 命令是否处理成功
     */
    bool parseCommand(const char* cmdStr) {
        if (!ENABLE_ESP || !cmdStr || strlen(cmdStr) == 0) {
            return false;
        }
        
        // 检查是否是命令前缀
        if (strncmp(cmdStr, PREFIX_CMD, strlen(PREFIX_CMD)) != 0) {
            Logger::warning("收到非命令消息: %s", cmdStr);
            return false;
        }
        
        // 提取命令内容
        const char* cmdContent = cmdStr + strlen(PREFIX_CMD);
        char cmdName[32] = {0};
        const char* params = nullptr;
        
        // 分离命令名和参数
        const char* comma = strchr(cmdContent, ',');
        if (comma) {
            // 有参数的情况
            size_t nameLen = comma - cmdContent;
            if (nameLen >= sizeof(cmdName)) nameLen = sizeof(cmdName) - 1;
            strncpy(cmdName, cmdContent, nameLen);
            cmdName[nameLen] = '\0';
            params = comma + 1;
        } else {
            // 无参数的情况
            strncpy(cmdName, cmdContent, sizeof(cmdName) - 1);
        }
        
        // 转换为大写
        for (char* p = cmdName; *p; ++p) {
            *p = toupper(*p);
        }
        
        // 根据命令名查找处理函数
        for (int i = 0; i < commandHandlersCount; i++) {
            const CommandMapping& mapping = commandHandlers[i];
            if (strcmp(cmdName, mapping.cmdName) == 0) {
                Logger::info("执行命令: %s", cmdName);
                return mapping.handler(params);
            }
        }
        
        Logger::warning("未知命令: %s", cmdName);
        return false;
    }
    
    /**
     * 更新状态，应在主循环中调用
     */
    void update() {
        if (ENABLE_ESP) {
            processReceivedData();
        }
    }
    
    /**
     * 清空接收缓冲区
     */
    void clearBuffer() {
        if (!ENABLE_ESP || !espSerial) return;
        
        while (espSerial->available()) {
            espSerial->read();
        }
        bufferIndex = 0;
    }
};

// 全局ESP桥接实例
extern EspBridge EspComm;

#endif // ESP_BRIDGE_H 
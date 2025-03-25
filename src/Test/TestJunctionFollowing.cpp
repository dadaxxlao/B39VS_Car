#ifdef TEST_JUNCTION_FOLLOWING
#include <Arduino.h>
#include <Wire.h>
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Control/LineDetector.h"
#include "../Control/LineFollower.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

// 创建实例
MotionController motionController;
SensorManager sensorManager;
LineDetector lineDetector;
LineFollower* lineFollower;  // 使用指针，因为需要在setup中初始化

// 测试配置
bool testRunning = false;
bool newCommandReceived = false;
String receivedCommand = "";
unsigned long lastUpdateTime = 0;
unsigned long junctionLastDetectedTime = 0;
const unsigned long JUNCTION_DETECTION_COOLDOWN = 1500; // 路口检测冷却时间（毫秒）

// 路口计数
int junctionCounter = 0;
JunctionType lastJunctionType = NO_JUNCTION;

// 记录路口历史
#define MAX_JUNCTION_HISTORY 10
JunctionType junctionHistory[MAX_JUNCTION_HISTORY];
int junctionHistoryIndex = 0;

// 转换路口类型为字符串
const char* junctionTypeToString(JunctionType junction) {
    switch (junction) {
        case NO_JUNCTION: return "无路口";
        case T_LEFT: return "左T字路口";
        case T_RIGHT: return "右T字路口";
        case LEFT_TURN: return "左转弯";
        case RIGHT_TURN: return "右转弯";
        default: return "未知路口";
    }
}

// 执行路口动作
void executeJunctionAction(JunctionType junction) {
    switch (junction) {
        case T_LEFT:
            Logger::info("检测到左T字路口，继续直行");
            break;
            
        case T_RIGHT:
            Logger::info("检测到右T字路口，继续直行");
            break;
            
        case LEFT_TURN:
            Logger::info("检测到左转弯，继续直行");
            break;
            
        case RIGHT_TURN:
            Logger::info("检测到右转弯，继续直行");
            break;
            
        default:
            break;
    }
    
    // 记录路口历史
    junctionHistory[junctionHistoryIndex] = junction;
    junctionHistoryIndex = (junctionHistoryIndex + 1) % MAX_JUNCTION_HISTORY;
    junctionCounter++;
}

// 处理串口命令
void processCommand(String command) {
    command.trim();
    
    if (command.startsWith("start") || command == "s") {
        testRunning = true;
        junctionCounter = 0;
        junctionHistoryIndex = 0;
        lineFollower->reset();
        Logger::info("路口巡线测试启动");
        Logger::info("等待3秒后开始...");
        delay(3000); // 增加启动延时，给操作者更多准备时间
    } 
    else if (command.startsWith("stop") || command == "x") {
        testRunning = false;
        motionController.emergencyStop();
        Logger::info("路口巡线测试停止");
    }
    else if (command.startsWith("kp")) {
        float value = command.substring(3).toFloat();
        if (value > 0) {
            lineFollower->setPIDParams(value, lineFollower->getKi(), lineFollower->getKd());
            Logger::info("设置 Kp = %.2f", value);
        }
    }
    else if (command.startsWith("kd")) {
        float value = command.substring(3).toFloat();
        if (value >= 0) {
            lineFollower->setPIDParams(lineFollower->getKp(), lineFollower->getKi(), value);
            Logger::info("设置 Kd = %.2f", value);
        }
    }
    else if (command.startsWith("speed")) {
        int value = command.substring(6).toInt();
        if (value > 0) {
            lineFollower->setBaseSpeed(value);
            Logger::info("设置基础速度 = %d", value);
        }
    }
    else if (command.startsWith("reset") || command == "r") {
        junctionCounter = 0;
        junctionHistoryIndex = 0;
        lastJunctionType = NO_JUNCTION;
        lineFollower->reset();
        Logger::info("重置路口计数和巡线状态");
    }
    else if (command.startsWith("history") || command == "h") {
        Serial.println("\n路口历史记录:");
        for (int i = 0; i < MAX_JUNCTION_HISTORY; i++) {
            int idx = (junctionHistoryIndex - i - 1 + MAX_JUNCTION_HISTORY) % MAX_JUNCTION_HISTORY;
            if (i < junctionCounter) {
                Serial.print(i+1);
                Serial.print(": ");
                Serial.println(junctionTypeToString(junctionHistory[idx]));
            }
        }
    }
    else if (command.startsWith("help") || command == "?") {
        Serial.println("\n可用命令:");
        Serial.println("start 或 s - 开始路口巡线测试");
        Serial.println("stop 或 x - 停止路口巡线测试");
        Serial.println("reset 或 r - 重置路口计数和巡线状态");
        Serial.println("history 或 h - 显示路口历史记录");
        Serial.println("kp[value] - 设置Kp值 (例如: kp0.5)");
        Serial.println("kd[value] - 设置Kd值 (例如: kd0.1)");
        Serial.println("speed[value] - 设置基础速度 (例如: speed100)");
        Serial.println("help 或 ? - 显示帮助信息");
    }
}

void setup() {
    // 初始化串口
    Serial.begin(9600);
    
    // 设置日志级别
    Logger::setLogLevel(LOG_LEVEL_DEBUG);
    
    // 清空串口缓冲区
    while(Serial.available()) {
        Serial.read();
    }
    
    // 输出欢迎信息
    Serial.println("\n===== 路口巡线测试程序 =====");
    Serial.println("使用此程序测试小车的路口检测和巡线功能");
    Serial.println("输入'help'或'?'查看可用命令");
    Serial.println("提示：可使用's'快速启动，'x'快速停止");
    
    // 初始化I2C
    Wire.begin();
    
    // 初始化传感器
    sensorManager.initAllSensors();
    
    // 初始化运动控制器
    motionController.init();
    
    // 初始化巡线控制器
    lineFollower = new LineFollower(sensorManager.getInfraredArray(), motionController);
    lineFollower->init();
    lineFollower->setPIDParams(0.6, 0.0, 0.1); // 设置默认PID参数
    lineFollower->setBaseSpeed(FOLLOW_SPEED);
    
    // 初始化历史记录数组
    for (int i = 0; i < MAX_JUNCTION_HISTORY; i++) {
        junctionHistory[i] = NO_JUNCTION;
    }
    
    Logger::info("初始化完成，准备开始测试");
    delay(2000); // 增加初始化后的稳定时间
    
    // 再次清空串口缓冲区
    while(Serial.available()) {
        Serial.read();
    }
    receivedCommand = ""; // 确保命令缓冲为空
}

void loop() {
    // 处理串口命令
    while (Serial.available() > 0) {
        char c = Serial.read();
        // 忽略回车符和换行符
        if (c == '\n' || c == '\r') {
            if (receivedCommand.length() > 0) {
                newCommandReceived = true;
            }
        } else {
            receivedCommand += c;
        }
    }
    
    if (newCommandReceived) {
        processCommand(receivedCommand);
        receivedCommand = ""; // 清空命令缓冲
        newCommandReceived = false;
    }
    
    // 根据测试状态执行相应操作
    if (testRunning) {
        // 更新传感器数据
        sensorManager.update();
        
        // 检查是否检测到线
        if (!sensorManager.isLineDetected()) {
            motionController.emergencyStop();
            Logger::warning("未检测到线，停止运动");
            return;
        }
        
        // 检测路口
        JunctionType junction = lineDetector.detectJunction(sensorManager.getInfraredSensorValues());
        
        // 路口检测和处理
        unsigned long currentTime = millis();
        if (junction != NO_JUNCTION && junction != lastJunctionType && 
            (currentTime - junctionLastDetectedTime > JUNCTION_DETECTION_COOLDOWN)) {
            
            // 检测到新的路口
            Logger::info("检测到路口: %s", junctionTypeToString(junction));
            
            // 只记录路口，不执行转弯动作
            executeJunctionAction(junction);
            
            // 更新状态
            lastJunctionType = junction;
            junctionLastDetectedTime = currentTime;
        } 
        
        // 继续巡线
        lineFollower->update();
        
        // 每500ms输出一次状态信息
        if (currentTime - lastUpdateTime > 500) {
            lastUpdateTime = currentTime;
            
            // 获取并打印红外传感器原始值
            const uint16_t* irValues = sensorManager.getInfraredSensorValues();
            Serial.print("传感器值: [");
            for (int i = 0; i < 8; i++) {
                Serial.print(irValues[i]);
                if (i < 7) Serial.print(", ");
            }
            Serial.println("]");
            
            Serial.print("当前路口: ");
            Serial.println(junctionTypeToString(junction));
            
            Serial.print("路口计数: ");
            Serial.print(junctionCounter);
            Serial.print(", 上一个路口: ");
            Serial.println(junctionTypeToString(lastJunctionType));
        }
    }
    else {
        // 如果测试未运行，确保车辆停止
        motionController.emergencyStop();
        delay(50); // 减少延时，提高响应速度
    }
}

#endif 
#include <Arduino.h>
#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Control/LineFollower.h"
#include "../Control/ObstacleAvoidance.h"
#include "../Utils/Logger.h"
#include "../Utils/Config.h"

// 注意: 只有当定义了TEST_OBSTACLE_AVOIDANCE时，才会编译此文件
#ifdef TEST_OBSTACLE_AVOIDANCE

// 全局对象
SensorManager sensorManager;
MotionController motionController;
LineFollower lineFollower(sensorManager);
// 使用新的构造函数，传入LineFollower以启用巡线功能
ObstacleAvoidance obstacleAvoidance(sensorManager, motionController, lineFollower);

// 全局变量
bool g_isRunning = false;
float g_lineSpeed = FOLLOW_SPEED;  // 使用Config.h中定义的巡线速度
float g_obstacleThreshold = 20.0f;
unsigned long g_lastStatusTime = 0;
const int STATUS_INTERVAL_MS = 1000;

// 函数声明
void processSerialCommands();
void printStatus();
void printHelp();
const char* getObstacleStateString(ObstacleAvoidanceState state);

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(500);
    Logger::init();
    Logger::setGlobalLogLevel(LOG_LEVEL_DEBUG);
    Logger::info("Test", "巡线避障测试程序启动...");
    
    Serial.println("\n===== 巡线避障测试程序 =====");
    
    // 初始化传感器
    Logger::info("Test", "正在初始化传感器...");
    if (sensorManager.initAllSensors()) {
        Logger::info("Test", "传感器初始化成功");
        Serial.println("传感器初始化成功");
    } else {
        Logger::error("Test", "传感器初始化失败");
        Serial.println("传感器初始化失败，请检查硬件连接");
        while (1) {
            delay(1000);
        }
    }
    
    // 初始化电机控制器
    Logger::info("Test", "初始化电机控制器...");
    motionController.init();
    Logger::info("Test", "电机控制器初始化成功");
    
    // 初始化巡线模块
    Logger::info("Test", "初始化巡线模块...");
    lineFollower.init();
    lineFollower.setBaseSpeed(g_lineSpeed);
    // 设置优化的PID参数，与路口测试使用的参数一致
    lineFollower.setPIDParams(1.0, 0.0, 1.0);
    Logger::info("Test", "巡线模块初始化成功");
    
    // 初始化避障模块
    Logger::info("Test", "初始化避障模块...");
    obstacleAvoidance.init();
    obstacleAvoidance.setObstacleThreshold(g_obstacleThreshold);
    Logger::info("Test", "避障模块初始化成功");
    
    // 打印帮助信息
    Serial.println("初始化完成，等待指令...");
    printHelp();
}

void loop() {
    // 处理串口命令
    processSerialCommands();
    
    // 更新传感器数据
    sensorManager.updateAll();
    
    // 如果正在运行测试
    if (g_isRunning) {
        // 更新避障控制器 - 现在它会同时处理巡线和避障
        obstacleAvoidance.update();
        
        // 检查当前避障状态
        ObstacleAvoidanceState obsState = obstacleAvoidance.getCurrentState();
        
        // 如果避障刚完成，重新启动避障检测
        if (obsState == OBS_COMPLETED) {
            obstacleAvoidance.reset();
            obstacleAvoidance.startDetecting();
            Logger::info("Test", "避障完成，继续巡线并启动障碍物检测");
        }
    }
    
    // 定期打印状态信息
    unsigned long currentTime = millis();
    if (currentTime - g_lastStatusTime >= STATUS_INTERVAL_MS) {
        g_lastStatusTime = currentTime;
        if (g_isRunning) {
            printStatus();
        }
    }
    
    // 短暂延时
    delay(30);
}

// 处理串口命令
void processSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.equalsIgnoreCase("start") || command == "s") {
            g_isRunning = true;
            // 重置巡线状态
            lineFollower.reset();
            // 重置避障状态并启动检测
            obstacleAvoidance.reset();
            obstacleAvoidance.startDetecting();
            Serial.println("测试已启动 - 巡线和避障开始");
            
            // 延迟1秒后开始，让用户有时间放好小车
            delay(1000);
        }
        else if (command.equalsIgnoreCase("stop") || command == "x") {
            g_isRunning = false;
            motionController.emergencyStop();
            obstacleAvoidance.reset();
            Serial.println("测试已停止");
        }
        else if (command.startsWith("speed=")) {
            String speedStr = command.substring(6);
            int speed = speedStr.toInt();
            if (speed >= 0 && speed <= 255) {
                g_lineSpeed = speed;
                lineFollower.setBaseSpeed(speed);
                Serial.print("巡线速度已设置为: ");
                Serial.println(speed);
            } else {
                Serial.println("无效的速度值，应在0-255范围内");
            }
        }
        else if (command.startsWith("kp=")) {
            float value = command.substring(3).toFloat();
            if (value >= 0) {
                lineFollower.setPIDParams(value, lineFollower.getKi(), lineFollower.getKd());
                Serial.print("PID参数Kp设置为: ");
                Serial.println(value);
            }
        }
        else if (command.startsWith("kd=")) {
            float value = command.substring(3).toFloat();
            if (value >= 0) {
                lineFollower.setPIDParams(lineFollower.getKp(), lineFollower.getKi(), value);
                Serial.print("PID参数Kd设置为: ");
                Serial.println(value);
            }
        }
        else if (command.startsWith("threshold=")) {
            String thresholdStr = command.substring(10);
            float threshold = thresholdStr.toFloat();
            if (threshold > 0) {
                g_obstacleThreshold = threshold;
                obstacleAvoidance.setObstacleThreshold(threshold);
                Serial.print("障碍物检测阈值已设置为: ");
                Serial.print(threshold);
                Serial.println(" cm");
            } else {
                Serial.println("无效的阈值，应大于0");
            }
        }
        else if (command.equalsIgnoreCase("status")) {
            printStatus();
        }
        else if (command.equalsIgnoreCase("help") || command == "h") {
            printHelp();
        }
        else if (command == "f") {
            // 直接前进，不使用巡线
            g_isRunning = false;
            motionController.moveForward(DEFAULT_SPEED);
            Serial.println("前进中...");
        }
        else if (command == "r") {
            // 恢复传感器
            Serial.println("重新初始化传感器...");
            sensorManager.initAllSensors();
            Serial.println("传感器重新初始化完成");
        }
    }
}

// 打印当前状态
void printStatus() {
    float distance = 0;
    bool distSuccess = sensorManager.getDistanceCm(distance);
    ObstacleAvoidanceState obsState = obstacleAvoidance.getCurrentState();
    
    Serial.println("\n----- 当前状态 -----");
    Serial.print("运行状态: ");
    Serial.println(g_isRunning ? "运行中" : "已停止");
    
    Serial.print("避障状态: ");
    Serial.println(getObstacleStateString(obsState));
    
    Serial.print("超声波距离: ");
    if (distSuccess) {
        Serial.print(distance);
        Serial.println(" cm");
    } else {
        Serial.println("读取失败");
    }
    
    Serial.print("巡线速度: ");
    Serial.println(g_lineSpeed);
    
    Serial.print("PID参数: Kp=");
    Serial.print(lineFollower.getKp());
    Serial.print(", Ki=");
    Serial.print(lineFollower.getKi());
    Serial.print(", Kd=");
    Serial.println(lineFollower.getKd());
    
    Serial.print("障碍物阈值: ");
    Serial.print(g_obstacleThreshold);
    Serial.println(" cm");
    
    // 打印线位置，但不显示详细的传感器值
    int position;
    if (sensorManager.getLinePosition(position)) {
        Serial.print("线位置: ");
        Serial.print(position);
        
        // 显示位置指示
        Serial.print(" (");
        if (position < -40) {
            Serial.println("偏左)");
        } else if (position > 40) {
            Serial.println("偏右)");
        } else {
            Serial.println("居中)");
        }
    } else {
        Serial.println("线位置: 未检测到线");
    }
    
    Serial.println("-------------------");
}

// 打印帮助信息
void printHelp() {
    Serial.println("\n----- 指令列表 -----");
    Serial.println("start/s     - 开始巡线和避障测试");
    Serial.println("stop/x      - 停止测试");
    Serial.println("speed=xxx   - 设置巡线速度(0-255)");
    Serial.println("kp=xxx      - 设置PID比例参数");
    Serial.println("kd=xxx      - 设置PID微分参数");
    Serial.println("threshold=xxx - 设置障碍物检测阈值(cm)");
    Serial.println("status      - 显示当前状态");
    Serial.println("f           - 直接前进(不使用巡线)");
    Serial.println("r           - 重新初始化传感器");
    Serial.println("help/h      - 显示此帮助信息");
    Serial.println("-------------------");
}

// 避障状态转字符串
const char* getObstacleStateString(ObstacleAvoidanceState state) {
    switch(state) {
        case OBS_INACTIVE: return "未激活";
        case OBS_DETECTING: return "障碍物检测中";
        case OBS_AVOIDING_RIGHT: return "右侧避障中";
        case OBS_AVOIDING_FORWARD: return "前进避障中";
        case OBS_AVOIDING_LEFT: return "左侧避障中";
        case OBS_COMPLETED: return "避障完成";
        default: return "未知状态";
    }
}

#endif // TEST_OBSTACLE_AVOIDANCE 
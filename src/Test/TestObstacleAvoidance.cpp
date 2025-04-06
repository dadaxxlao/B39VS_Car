#include <Arduino.h>
#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Control/ObstacleAvoidance.h"
#include "../Utils/Logger.h"

// 注意: 只有当定义了TEST_OBSTACLE_AVOIDANCE时，才会编译此文件
#ifdef TEST_OBSTACLE_AVOIDANCE

// 全局对象
SensorManager sensorManager;
MotionController motionController;
ObstacleAvoidance obstacleAvoidance(sensorManager, motionController);

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(500);
    Logger::info("Test", "避障测试程序启动...");
    
    // 初始化传感器
    if (sensorManager.init()) {
        Logger::info("Test", "传感器管理器初始化成功");
    } else {
        Logger::error("Test", "传感器管理器初始化失败");
        while (1) {
            delay(1000);
        }
    }
    
    // 初始化电机控制器
    motionController.init();
    Logger::info("Test", "电机控制器初始化成功");
    
    // 初始化避障控制器
    obstacleAvoidance.init();
    
    // 启动避障检测
    obstacleAvoidance.startDetecting();
    Logger::info("Test", "避障检测已启动，开始前进");
    
    // 开始前进
    motionController.moveForward(150);
}

void loop() {
    // 更新避障控制器
    obstacleAvoidance.update();
    
    // 如果不在检测状态，则继续前进
    ObstacleAvoidanceState state = obstacleAvoidance.getCurrentState();
    if (state == OBS_INACTIVE || state == OBS_COMPLETED) {
        // 避障完成或未开始避障，继续前进
        motionController.moveForward(150);
        
        // 如果避障完成，重置避障状态并重新开始检测
        if (state == OBS_COMPLETED) {
            obstacleAvoidance.reset();
            obstacleAvoidance.startDetecting();
            Logger::info("Test", "避障完成，继续前进并重新开始检测");
        }
    }
    
    // 打印超声波距离（每500ms）
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 500) {
        float distance = 0;
        if (sensorManager.getDistanceCm(distance)) {
            Logger::debug("Test", "超声波距离: %.2f cm", distance);
        }
        lastPrintTime = millis();
    }
    
    // 简单延时
    delay(10);
}

#endif // TEST_OBSTACLE_AVOIDANCE 
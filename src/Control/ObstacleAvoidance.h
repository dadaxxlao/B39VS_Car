#ifndef OBSTACLE_AVOIDANCE_H
#define OBSTACLE_AVOIDANCE_H

#include "../Sensor/SensorManager.h"
#include "../Motor/MotionController.h"
#include "../Control/LineFollower.h"
#include "../Utils/Logger.h"

// 避障状态枚举
enum ObstacleAvoidanceState {
    OBS_INACTIVE,           // 未激活状态
    OBS_DETECTING,          // 检测状态
    OBS_AVOIDING_RIGHT,     // 向右平移避障
    OBS_AVOIDING_FORWARD,   // 向前行驶避障
    OBS_AVOIDING_LEFT,      // 向左平移避障，直到中间传感器检测到黑线或超时
    OBS_COMPLETED           // 避障完成
};

class ObstacleAvoidance {
private:
    // 引用传感器和电机控制器
    SensorManager& m_sensorManager;
    MotionController& m_motionController;
    LineFollower* m_lineFollower;  // 可选的LineFollower对象指针
    
    // 是否使用LineFollower进行巡线
    bool m_useLineFollower;
    
    // 避障状态
    ObstacleAvoidanceState m_currentState;
    
    // 操作开始时间
    unsigned long m_actionStartTime;
    
    // 避障参数
    const unsigned long RIGHT_MOVE_DURATION = 1500;    // 向右平移时间(5秒)
    const unsigned long FORWARD_MOVE_DURATION = 3000;  // 向前行驶时间(5秒)
    const unsigned long LEFT_MOVE_DURATION = 1700;     // 向左平移时间(5秒)
    const int AVOID_SPEED = 100;                       // 避障速度
    const float OBSTACLE_THRESHOLD = 5.0;             // 障碍物检测阈值(20cm)
    
    // 私有方法：检查障碍物
    bool checkForObstacle();
    
    // 私有方法：应用PID控制
    void applyPIDControl(float turnAmount, int baseSpeed);

public:
    // 构造函数 - 原始版本，不使用LineFollower
    ObstacleAvoidance(SensorManager& sensorManager, MotionController& motionController);
    
    // 构造函数 - 支持LineFollower的版本
    ObstacleAvoidance(SensorManager& sensorManager, MotionController& motionController, LineFollower& lineFollower);
    
    // 初始化
    void init();
    
    // 启动避障检测
    void startDetecting();
    
    // 停止避障检测
    void stopDetecting();
    
    // 更新状态
    void update();
    
    // 获取当前状态
    ObstacleAvoidanceState getCurrentState() const;
    
    // 检查避障是否已完成
    bool isAvoidanceCompleted() const;
    
    // 重置状态
    void reset();
    
    // 获取是否使用LineFollower
    bool isUsingLineFollower() const { return m_useLineFollower; }
    
    // 设置障碍物检测阈值
    void setObstacleThreshold(float threshold);
};

#endif // OBSTACLE_AVOIDANCE_H 
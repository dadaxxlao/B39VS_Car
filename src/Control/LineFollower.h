#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

class LineFollower {
public:
    // 触发类型枚举
    enum TriggerType {
        TRIGGER_NONE,
        TRIGGER_LEFT_EDGE,
        TRIGGER_RIGHT_EDGE
    };

private:
    // 红外传感器引用
    SensorManager& m_sensorManager;
    
    // PID控制参数
    float m_Kp;           // 比例系数
    float m_Ki;           // 积分系数
    float m_Kd;           // 微分系数
    int m_lastError;      // 上一次误差
    int m_integral;       // 积分项
    
    // 线丢失处理参数 - 这些将由NavigationController接管
    // unsigned long m_lineLastDetectedTime;  // 上次检测到线的时间
    // unsigned long m_maxLineLostTime;       // 最长允许丢线的时间（毫秒）
    float m_lastTurnAmount;                // 上次转向量
    
    // 基础速度
    int m_baseSpeed;      // 基础速度

public:
    // 构造函数
    LineFollower(SensorManager& sensorManager);
    
    // 初始化函数
    void init();
    
    // 设置PID参数
    void setPIDParams(float Kp, float Ki, float Kd);
    
    // 设置丢线处理参数 - 将被NavigationController接管
    // void setLineLostParams(unsigned long maxLineLostTime);
    
    // 设置基础速度
    void setBaseSpeed(int speed);
    
    // 巡线函数 - 更新机器人移动 (修改返回类型)
    TriggerType update();
    
    // 重置状态
    void reset();
    
    // 获取当前PID参数
    float getKp() const { return m_Kp; }
    float getKi() const { return m_Ki; }
    float getKd() const { return m_Kd; }
    
    // 获取基础速度
    int getBaseSpeed() const { return m_baseSpeed; }
    
    // 获取上次计算的转向量
    float getLastTurnAmount() const { return m_lastTurnAmount; }
    
    // 获取丢线最大时间 - 将被NavigationController接管
    // unsigned long getMaxLineLostTime() const { return m_maxLineLostTime; }
};

#endif // LINE_FOLLOWER_H 
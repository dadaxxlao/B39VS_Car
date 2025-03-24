#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include "../Motor/MotionController.h"
#include "../Sensor/Infrared.h"
#include "../Utils/Config.h"
#include "../Utils/Logger.h"

class LineFollower {
private:
    // 红外传感器和运动控制器的引用
    InfraredArray& m_infraredSensor;
    MotionController& m_motionController;

    // PID控制参数
    float m_Kp;           // 比例系数
    float m_Ki;           // 积分系数
    float m_Kd;           // 微分系数
    int m_lastError;      // 上一次误差
    int m_integral;       // 积分项
    
    // 线丢失处理参数
    unsigned long m_lineLastDetectedTime;  // 上次检测到线的时间
    unsigned long m_maxLineLostTime;       // 最长允许丢线的时间（毫秒）
    float m_lastForwardSpeed;              // 上次前进速度
    float m_lastTurnAmount;                // 上次转向量
    
    // 基础速度
    int m_baseSpeed;      // 基础速度
    
    // 转向增益参数
    float m_leftTurnGain;  // 左转增益
    float m_rightTurnGain; // 右转增益

public:
    // 构造函数
    LineFollower(InfraredArray& infraredSensor, MotionController& motionController);
    
    // 初始化函数
    void init();
    
    // 设置PID参数
    void setPIDParams(float Kp, float Ki, float Kd);
    
    // 设置丢线处理参数
    void setLineLostParams(unsigned long maxLineLostTime);
    
    // 设置基础速度
    void setBaseSpeed(int speed);
    
    // 设置转向增益
    void setTurnGain(float leftGain, float rightGain);
    
    // 巡线函数 - 更新机器人移动
    void update();
    
    // 重置状态
    void reset();
    
    // 获取当前PID参数
    float getKp() const { return m_Kp; }
    float getKi() const { return m_Ki; }
    float getKd() const { return m_Kd; }
    
    // 获取基础速度
    int getBaseSpeed() const { return m_baseSpeed; }
    
    // 获取丢线最大时间
    unsigned long getMaxLineLostTime() const { return m_maxLineLostTime; }
    
    // 获取转向增益
    float getLeftTurnGain() const { return m_leftTurnGain; }
    float getRightTurnGain() const { return m_rightTurnGain; }
};

#endif // LINE_FOLLOWER_H 
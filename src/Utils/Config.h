#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// 引脚定义
// 麦克纳姆轮电机引脚定义
// 左前电机 (FL)
#define MOTOR_FL_PWM         2
#define MOTOR_FL_IN1         3
#define MOTOR_FL_IN2         4
// 右前电机 (FR)
#define MOTOR_FR_PWM         10
#define MOTOR_FR_IN1         5
#define MOTOR_FR_IN2         6
// 左后电机 (RL)
#define MOTOR_RL_PWM         11
#define MOTOR_RL_IN1         7
#define MOTOR_RL_IN2         8
// 右后电机 (RR)
#define MOTOR_RR_PWM         13
#define MOTOR_RR_IN1         14
#define MOTOR_RR_IN2         15

// RGB LED灯环引脚
#define RGB_LED_PIN          6
#define RGB_LED_COUNT        16

// 舵机引脚
#define ARM_SERVO_PIN        9
#define GRIPPER_SERVO_PIN    10

// 传感器引脚
#define ULTRASONIC_TRIG_PIN  11
#define ULTRASONIC_ECHO_PIN  12

// 蓝牙HM-10模块引脚
#define BT_RX_PIN            16  // Arduino接收，连接到HM-10的TX
#define BT_TX_PIN            17  // Arduino发送，连接到HM-10的RX
#define BT_BAUD_RATE         9600

// I2C地址
#define INFRARED_ARRAY_ADDR  0x12
#define COLOR_SENSOR_ADDR    0x29

// 运动参数
#define MAX_SPEED            255
#define TURN_SPEED           150
#define FOLLOW_SPEED         180
#define SHARP_TURN_SPEED     200
#define DEFAULT_SPEED        200  // 麦克纳姆轮默认移动速度

// 阈值参数
#define NO_OBJECT_THRESHOLD  50   // 超声波检测无障碍物阈值(cm)
#define GRAB_DISTANCE        10   // 抓取距离(cm)
#define LINE_THRESHOLD       500  // 红外线检测阈值

// 机械臂参数
#define ARM_UP_ANGLE         90
#define ARM_DOWN_ANGLE       0
#define GRIPPER_OPEN_ANGLE   180
#define GRIPPER_CLOSE_ANGLE  90
#define SERVO_DELAY          15   // 舵机移动延迟(ms)

// 路口类型
enum JunctionType {
    NO_JUNCTION,
    T_LEFT,     // 左旋T字路口
    T_RIGHT,    // 右旋T字路口
    T_FORWARD,  // 正T字路口（倒T）
    CROSS,      // 十字路口
    LEFT_TURN,  // 左转弯
    RIGHT_TURN, // 右转弯
    END_OF_LINE // 线路终点
};

// 颜色编码
enum ColorCode {
    COLOR_UNKNOWN,
    COLOR_RED,    // 1
    COLOR_BLUE,  // 2
    COLOR_YELLOW,   // 3
    COLOR_WHITE,  // 4
    COLOR_BLACK,  // 5
    COLOR_COUNT   // 总颜色数量
};

// 系统状态
enum SystemState {
    INITIALIZED,     // 初始化状态
    OBJECT_FIND,     // 寻找物块状态
    OBJECT_GRAB,     // 抓取物块状态
    OBJECT_LOCATE,   // 定位放置区域状态
    OBJECT_PLACING,  // 放置物块状态
    RETURN_BASE,     // 返回基地状态
    END,             // 任务结束状态
    ERROR_STATE      // 错误状态
};

// OBJECT_LOCATE子状态
enum LocateSubState {
    TURN_AROUND,     // 掉头
    COUNT_JUNCTIONS, // 统计经过的路口
    SELECT_TARGET    // 选择目标路口
};

// 启用蓝牙通信
#define ENABLE_BLUETOOTH     0

#endif // CONFIG_H 
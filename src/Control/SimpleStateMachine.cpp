#include "SimpleStateMachine.h"
#include <avr/pgmspace.h>  // 添加PROGMEM支持

// 调试宏定义，启用更详细的状态机日志
#define DEBUG_STATE_MACHINE 1

// 定义特殊操作的超时时间（毫秒）
#define GRAB_TIMEOUT         60000
#define PLACING_TIMEOUT      3000
#define TURN_AROUND_TIMEOUT  2000
#define FORWARD_TIMEOUT      1000
#define BASE_ARRIVE_FORWARD_TIME 2000
#define CONFIRMATION_TIMEOUT 10000 // 等待确认的超时时间

// 定义阈值常量
#define OBJECT_DETECTION_THRESHOLD 50.0f  // 物体检测阈值，单位cm
#define LINE_THRESHOLD 500 // 假设的线传感器阈值

// 设置日志级别，2表示只记录错误和警告，1表示记录一般信息，0表示记录所有
#define USE_MINIMAL_LOGGING 0

/**
 * 构造函数
 */
SimpleStateMachine::SimpleStateMachine(SensorManager& sm, MotionController& mc, 
                           RoboticArm& arm, NavigationController& nc)
    : m_sensorManager(sm)
    , m_motionController(mc)
    , m_roboticArm(arm)
    , m_navigationController(nc)
    , m_accurateTurn(mc, sm)  // 使用电机控制器和传感器管理器初始化精确转向
    , m_currentState(INITIALIZED)
    , m_locateSubState(TURN_AROUND)
    , m_zoneCounter(0)
    , m_colorCounter(1)
    , m_blockCounter(0) // 初始化物块计数器
    , m_detectedColorCode(COLOR_UNKNOWN)
    , m_actionStartTime(0)
{
    // 初始化位域结构体的所有标志位
    m_flags.m_isActionComplete = false;
    m_flags.m_isTurning = false;
}

/**
 * 初始化函数
 */
void SimpleStateMachine::init() {
    // 设置初始状态
    m_currentState = INITIALIZED;
    m_zoneCounter = 0;
    m_colorCounter = 1;
    m_blockCounter = 0; // 重置物块计数器
    m_detectedColorCode = COLOR_UNKNOWN;
    m_flags.m_isTurning = false;
    
    // 初始化精确转向控制器
    m_accurateTurn.init();
    
    // 记录初始化
#if USE_MINIMAL_LOGGING < 2
    Logger::info("SimpleStateMachine", F("状态机初始化完成，等待启动信号..."));
#endif
}

/**
 * 主循环更新函数 - 使用if-else逻辑而非switch-case
 */
void SimpleStateMachine::update() {
    // Modify entry log to use F() and print state
    // Logger::info("SimpleStateMachine", F("Update entered. State: %d - Logger+F test"), m_currentState);
    // Remove Serial test line
    // Serial.println("SimpleStateMachine Update - Serial test");

    // 处理转向状态
    if (m_flags.m_isTurning) {
        m_accurateTurn.update();
        
        if (m_accurateTurn.isTurnComplete()) {
#if USE_MINIMAL_LOGGING == 0
            // saLogger::info("SimpleStateMachine", F("转向完成，继续状态机逻辑"));
#endif
            m_flags.m_isTurning = false;
            m_accurateTurn.reset(); // Reset the AccurateTurn state to IDLE
            m_navigationController.resumeFollowing();
        } else {
            return;
        }
    }

    // 更新传感器数据
    m_sensorManager.updateAll();
    
    // 使用if-else处理不同状态
    if (m_currentState == INITIALIZED) {
        // 等待触发信号
        float distance;
        bool success = false;
        
        // 测量新的距离
        unsigned long duration = m_sensorManager.measurePulseDuration();
        if (duration > 0 && duration < ULTRASONIC_PULSE_TIMEOUT) {
            distance = m_sensorManager.getDistanceCmFromDuration(duration);
            success = true;
        }
        
        if (success) {
            if (distance < 30.0f && distance > 0) {
#if USE_MINIMAL_LOGGING == 0
            Logger::info("SimpleStateMachine", "检测到启动触发，距离: %.2f cm", distance);
#endif
            m_motionController.moveForward();
            delay(500);
            transitionTo(OBJECT_FIND);
        }
        } else {
            Logger::debug("SimpleStateMachine", "无法测量有效距离");
        }


    }
    else if (m_currentState == OBJECT_FIND) {
        // 寻找物块状态
        m_zoneCounter = 0;
        m_navigationController.setObstacleAvoidanceReverse(false);
        if (!m_flags.m_isTurning) {
            m_navigationController.update();
            
            NavigationState navState = m_navigationController.getCurrentNavigationState();
            
            if (navState == NAV_AT_JUNCTION) {
                JunctionType junction = m_navigationController.getDetectedJunctionType();
                
#if USE_MINIMAL_LOGGING == 0
                // Modify this log: Remove F() macro
                // Logger::info("SimpleStateMachine", "OBJECT_FIND - Junction: %s (No F() test)", this->junctionTypeToString(junction));
                // Original line commented out:
                // Logger::info("SimpleStateMachine", F("OBJECT_FIND状态 - 检测到路口: %s"), 
                //            this->junctionTypeToString(junction));
#endif
                
                if (junction == T_LEFT || junction == LEFT_TURN) {
#if USE_MINIMAL_LOGGING == 0
                    Logger::info("SimpleStateMachine", F("左T形路口或左转弯，直接进入超声波检测"));
#endif
                    // 不需要左转，直接进入超声波检测状态
                    transitionTo(ULTRASONIC_DETECT);
                }
                else if (junction == T_RIGHT) {
                    m_navigationController.resumeFollowing();
                }
                else if (junction == RIGHT_TURN ) {
#if USE_MINIMAL_LOGGING == 0
                    Logger::info("SimpleStateMachine", F("执行精确右转"));
#endif
                    m_accurateTurn.startTurnRight();
                    m_flags.m_isTurning = true;
                    m_navigationController.resumeFollowing();
                }
                else {
                    m_navigationController.resumeFollowing();
                }
            }
            else if (navState == NAV_ERROR) {
                Logger::error("SimpleStateMachine", F("导航错误，进入ERROR状态"));
                transitionTo(ERROR_STATE);
            }
        }
    }
    else if (m_currentState == ULTRASONIC_DETECT) {
        // 超声波检测状态
        if (m_flags.m_isTurning) {
            return;
        }
        
        if (m_actionStartTime == 0) {
#if USE_MINIMAL_LOGGING == 0
            Logger::info("SimpleStateMachine", F("超声波检测状态，执行精确左转"));
#endif
            m_accurateTurn.startTurnLeft();
            m_flags.m_isTurning = true;
            
            m_zoneCounter++;
            m_actionStartTime = millis();
#if USE_MINIMAL_LOGGING == 0
            // Logger::info("SimpleStateMachine", "进入超声波检测状态，执行左转，区域计数: %d", m_zoneCounter);
#endif
            return;
        }
        //m_navigationController.resumeFollowing();
        float distance;
        bool success = false;
        
        // 测量新的距离
        unsigned long duration = m_sensorManager.measurePulseDuration();
        if (duration > 0 && duration < ULTRASONIC_PULSE_TIMEOUT) {
            distance = m_sensorManager.getDistanceCmFromDuration(duration);
            success = true;
        }

        
        if (distance < OBJECT_DETECTION_THRESHOLD) {
#if USE_MINIMAL_LOGGING == 0
            Logger::info("SimpleStateMachine", "检测到物块，距离: %f cm", distance);
#endif
            m_actionStartTime = 0;
            transitionTo(OBJECT_GRAB);
        } 
        else if (millis() - m_actionStartTime > 1000) {
#if USE_MINIMAL_LOGGING == 0
            Logger::info("SimpleStateMachine", "未检测到物块，执行右转并继续循线");
#endif
            m_actionStartTime = 0;
            
            // 执行右转
            m_accurateTurn.startTurnRight();
            m_flags.m_isTurning = true;
            
            // 转到CONTINUE_SEARCH状态，继续循线前进直到遇到左转或左T路口
            // 这样不会重置zoneCounter
            transitionTo(CONTINUE_SEARCH);
        }
    }
    else if (m_currentState == CONTINUE_SEARCH) {
        // 继续搜索状态 - 右转后继续循线寻找物块
        // 此状态逻辑类似OBJECT_FIND，但不会重置zoneCounter
        
        if (m_flags.m_isTurning) {
            return;
        }
        
        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
#if USE_MINIMAL_LOGGING == 0
            Logger::info("SimpleStateMachine", F("CONTINUE_SEARCH状态 - 检测到路口: %s"), 
                       this->junctionTypeToString(junction));
#endif
            
            if (junction == T_LEFT || junction == LEFT_TURN) {
#if USE_MINIMAL_LOGGING == 0
                Logger::info("SimpleStateMachine", F("左T形路口或左转弯，进入超声波检测"));
#endif
                // 遇到左转或左T路口，再次进入超声波检测
                transitionTo(ULTRASONIC_DETECT);
            }
            else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", F("导航错误，进入ERROR状态"));
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == OBJECT_GRAB) {
        // 抓取物块状态
        if (m_flags.m_isTurning) {
            return;
        }

        // 在此状态禁用避障
        m_navigationController.setObstacleAvoidanceEnabled(false);
        // 设置临时基础速度
        m_navigationController.setBaseSpeed(40);
        if (m_actionStartTime == 0) {
            m_actionStartTime = millis();
            m_flags.m_isActionComplete = false;
            
            static bool isFollowingLine = true;
            
            if (isFollowingLine) {
                m_sensorManager.updateAll();
                m_navigationController.update();
                
                NavigationState navState = m_navigationController.getCurrentNavigationState();
                
                if (navState == NAV_ERROR) {
                    Logger::error("SimpleStateMachine", F("导航控制器处于错误状态！"));
                    m_motionController.emergencyStop();
                    transitionTo(ERROR_STATE);
                    return;
                }
                
                if (m_roboticArm.checkGrabCondition()) {
                    m_motionController.emergencyStop();
                    isFollowingLine = false;
#if USE_MINIMAL_LOGGING < 2
                    Logger::info("SimpleStateMachine", F("检测到物体，停止循迹，开始抓取"));
#endif
                } else {
                    m_actionStartTime = 0;
                    return;
                }
            }
            
            if (!isFollowingLine) {
                // 执行机械臂操作（完整版）
                m_roboticArm.adjustArm(2150, 450, 0);
                if (millis() - m_actionStartTime < 2000) {
                    m_sensorManager.updateAll();
                    return;
                }
                
                m_roboticArm.adjustArm(2150, 450, 900);
                if (millis() - m_actionStartTime < 2000) {
                    m_sensorManager.updateAll();
                    return;
                }
                
                m_roboticArm.adjustArm(1500, 1600, 900);
                if (millis() - m_actionStartTime < 2000) {
                    m_sensorManager.updateAll();
                    return;
                }
                
                m_roboticArm.adjustArm(1500, 1600, 0);
                if (millis() - m_actionStartTime < 2000) {
                    m_sensorManager.updateAll();
                    return;
                }
                delay(1000);
#if USE_MINIMAL_LOGGING < 2
                Logger::info("SimpleStateMachine", F("抓取序列完成"));
#endif
                //Serial2.println("抓取序列完成");
                //Serial2.println("等待颜色输入");
                
                // 调用新的函数读取颜色代码
                m_detectedColorCode = readColorCodeFromSerial2();
                
                // 根据读取结果进行日志记录或处理
                if (m_detectedColorCode == COLOR_UNKNOWN) {
                    //Logger::warning("StateMachine", "未能从 Serial2 读取有效的颜色代码或超时。");
                    // 这里可以添加错误处理逻辑，例如重试或进入错误状态
                    // transitionTo(ERROR_STATE);
                    m_detectedColorCode = COLOR_RED;
                } else {
                    //Logger::info("StateMachine", "从 Serial2 读取到颜色代码: %d", m_detectedColorCode);
                }
                
                m_flags.m_isActionComplete = true;
                isFollowingLine = true;
            }
        }
        
        if (m_flags.m_isActionComplete) {
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", F("准备掉头并转换到放置状态"));
            Logger::info("SimpleStateMachine", F("执行精确U型转弯"));
#endif
            Serial.println("准备掉头并转换到放置状态");
            m_accurateTurn.startUTurn();
            m_flags.m_isTurning = true;
            
            m_actionStartTime = 0; 
            m_flags.m_isActionComplete = false;
            
            //Serial.println("\n0\n");
            
            transitionTo(OBJECT_PLACING);
        }
        
        if (millis() - m_actionStartTime > GRAB_TIMEOUT * 3 && !m_flags.m_isActionComplete) {
             Logger::error("SimpleStateMachine", F("抓取操作超时！"));
             m_actionStartTime = 0;
             transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == OBJECT_PLACING) {
        // 放置物体状态
        // Check if currently turning first
        if (m_flags.m_isTurning) {
            return; // Wait for turn to complete
            //Serial.println("\n ----- 1 ----- \n");
        }
        // Serial.println("\n ----- 2 ----- \n");
        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        m_navigationController.setObstacleAvoidanceReverse(true);
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
#if USE_MINIMAL_LOGGING < 2
            //Logger::info("SimpleStateMachine", "OBJECT_PLACING状态 - 检测到路口: %s", 
            //           this->junctionTypeToString(junction));
#endif
            
            if (junction == RIGHT_TURN || junction == T_FORWARD) {
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnRight();
                m_flags.m_isTurning = true;
                // Remove resumeFollowing, handled after turn completes
            }
            else if (junction == T_RIGHT) {
                m_navigationController.resumeFollowing(); // Continue straight
            }
            else if (junction == LEFT_TURN) {
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnLeft();
                m_flags.m_isTurning = true;
                // Remove resumeFollowing, handled after turn completes
            }
            else if (junction == T_LEFT) {
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnLeft();
                m_flags.m_isTurning = true;
                m_colorCounter = 1; // Reset counter before counting
                transitionTo(COUNT_INTERSECTION);
            }
            else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", "导航错误，进入ERROR状态");
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == COUNT_INTERSECTION) {
        // 路口计数状态
        // Check if currently turning first
        if (m_flags.m_isTurning) {
            return; // Wait for turn to complete
        }
        m_navigationController.setBaseSpeed(40);
        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
            if (junction == T_RIGHT || junction == RIGHT_TURN) {
                if (m_colorCounter == m_detectedColorCode) {
#if USE_MINIMAL_LOGGING < 2
                    Logger::info("SimpleStateMachine", "达到目标区域，执行精确右转");
#endif
                    // Replace MotionController turn with AccurateTurn
                    m_accurateTurn.startTurnRight();
                    m_flags.m_isTurning = true;
                    transitionTo(OBJECT_RELEASE);
                } else {
                    m_colorCounter++;
#if USE_MINIMAL_LOGGING < 2
                    Logger::info("SimpleStateMachine", "颜色计数: %d/%d", m_colorCounter, m_detectedColorCode);
#endif
                    m_navigationController.resumeFollowing();
                }
            } else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", "导航错误，进入ERROR状态");
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == OBJECT_RELEASE) {
        // 释放物体状态
        if (m_actionStartTime == 0) {
            m_actionStartTime = millis();
            m_flags.m_isActionComplete = false;
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "进入物体释放状态，循迹前进1.5秒后停车");
#endif
            
            m_navigationController.resumeFollowing();
        }
        
        if (!m_flags.m_isActionComplete && (millis() - m_actionStartTime < 1500)) {
            m_navigationController.update();
            return;
        }
        
        if (!m_flags.m_isActionComplete) {
            m_motionController.emergencyStop();
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "停车执行放置操作");
            Logger::info("SimpleStateMachine", "抓紧物体");
#endif
            m_roboticArm.adjustArm(1500, 1600, 900);
            if (millis() - m_actionStartTime < 2000) {
                m_sensorManager.updateAll();
                return;
            }
            
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "放下物体");
#endif
            m_roboticArm.adjustArm(2150, 450, 900);
            if (millis() - m_actionStartTime < 2000) {
                m_sensorManager.updateAll();
                return;
            }
            
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "完全松开夹爪");
#endif
            m_roboticArm.adjustArm(2150, 450, -50);
            if (millis() - m_actionStartTime < 2000) {
                m_sensorManager.updateAll();
                return;
            }
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "复位机械臂");
#endif
            m_roboticArm.reset();
            if (millis() - m_actionStartTime < 2000) {
                m_sensorManager.updateAll();
                return;
            }
            
            m_flags.m_isActionComplete = true;
            m_blockCounter++; // 物块计数器加1
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "物体放置完成，物块计数: %d", m_blockCounter);
#endif
        }
        
        if (m_flags.m_isActionComplete) {
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "开始掉头");
#endif
            m_accurateTurn.startUTurn();
            m_flags.m_isTurning = true;
            
            m_actionStartTime = 0;
            m_flags.m_isActionComplete = false;
            
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "掉头中，完成后进入遍历判断状态");
#endif
            transitionTo(ERGODIC_JUDGE);
        }
    }
    else if (m_currentState == ERGODIC_JUDGE) {
        // 遍历判断状态
        // Check if currently turning first
        if (m_flags.m_isTurning) {
            return; // Wait for turn to complete
        }

        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
            if (junction == T_FORWARD || junction == LEFT_TURN || junction == RIGHT_TURN) {
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnLeft();
                m_flags.m_isTurning = true;
                
                // Subsequent logic remains the same, executed after turn completes
                if (m_blockCounter < 2) {
#if USE_MINIMAL_LOGGING < 2
                    Logger::info("SimpleStateMachine", F("已放置物块数: %d，继续寻找物块"), m_blockCounter);
#endif
                    transitionTo(BACK_OBJECT_FIND);
                } else {
#if USE_MINIMAL_LOGGING < 2
                    Logger::info("SimpleStateMachine", F("已放置%d个物块，返回基地"), m_blockCounter);
#endif
                    transitionTo(RETURN_BASE);
                }
            } else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", F("导航错误，进入ERROR状态"));
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == BACK_OBJECT_FIND) {
        // 返回寻找物块状态
        // Check if currently turning first
        if (m_flags.m_isTurning) {
            return; // Wait for turn to complete
        }

        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", "BACK_OBJECT_FIND状态 - 检测到路口: %s", 
                       this->junctionTypeToString(junction));
#endif
            
            if (junction == T_FORWARD || junction == LEFT_TURN || junction == RIGHT_TURN) {
#if USE_MINIMAL_LOGGING < 2
                Logger::info("SimpleStateMachine", "检测到T字路口，执行精确右转进入OBJECT_FIND状态");
#endif
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnRight();
                m_flags.m_isTurning = true;
                transitionTo(OBJECT_FIND);
            } else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", F("导航错误，进入ERROR状态"));
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == RETURN_BASE) {
        // 返回基地状态
        // Check if currently turning first
        if (m_flags.m_isTurning) {
            return; // Wait for turn to complete
        }

        m_navigationController.update();
        
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", F("RETURN_BASE状态 - 检测到路口: %s"), 
                       this->junctionTypeToString(junction));
#endif
            
            if (junction == T_FORWARD || junction == LEFT_TURN || junction == RIGHT_TURN) {
                // Replace MotionController turn with AccurateTurn
                m_accurateTurn.startTurnLeft();
                m_flags.m_isTurning = true;
                
                // Subsequent logic remains the same, executed after turn completes
                if (m_actionStartTime == 0) {
                    m_actionStartTime = millis();
                } else {
                    transitionTo(BASE_ARRIVE);
                }
            } else {
                m_navigationController.resumeFollowing();
            }
        }
        else if (navState == NAV_ERROR) {
            Logger::error("SimpleStateMachine", F("导航错误，进入ERROR状态"));
            transitionTo(ERROR_STATE);
        }
    }
    else if (m_currentState == BASE_ARRIVE) {
        // 到达基地状态
        if (m_actionStartTime == 0) {
            m_actionStartTime = millis();
            m_motionController.moveForward(FOLLOW_SPEED);
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", F("到达基地边缘，继续前进一段时间"));
#endif
        }
        
        m_sensorManager.updateAll();
        
        uint16_t irValues[8];
        m_sensorManager.getInfraredSensorValues(irValues);
        
        bool isAllBlack = true;
        for (int i = 0; i < 8; i++) {
            if (irValues[i] < LINE_THRESHOLD) {
                isAllBlack = false;
                break;
            }
        }
        
        if ((millis() - m_actionStartTime > 1000) || isAllBlack) {
            m_motionController.emergencyStop();
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", F("到达基地，任务完成"));
#endif
            transitionTo(END);
        }
    }
    else if (m_currentState == END) {
        // 结束状态
        m_motionController.emergencyStop();
    }
    else if (m_currentState == ERROR_STATE) {
        // 错误状态
        m_motionController.emergencyStop();
        
        float distance;
        bool success = false;
        
        // 测量新的距离
        unsigned long duration = m_sensorManager.measurePulseDuration();
        if (duration > 0 && duration < ULTRASONIC_PULSE_TIMEOUT) {
            distance = m_sensorManager.getDistanceCmFromDuration(duration);
            success = true;
        }
        static float lastDistance = distance;
        
        if (abs(distance - lastDistance) > 20.0f) {
#if USE_MINIMAL_LOGGING < 2
            Logger::info("SimpleStateMachine", F("检测到重置信号，重新初始化"));
#endif
            init();
            m_navigationController.init();
            transitionTo(INITIALIZED);
        }
        
        lastDistance = distance;
    }
    else {
        Logger::error("SimpleStateMachine", F("未知系统状态：%d"), m_currentState);
        transitionTo(ERROR_STATE);
    }
}

/**
 * 状态转换函数
 */
void SimpleStateMachine::transitionTo(SystemState newState) {
    if (newState == m_currentState) {
        return;
    }
    
    
#if USE_MINIMAL_LOGGING < 2
    Logger::info("SimpleStateMachine", "状态切换: %s -> %s", 
               systemStateToString(m_currentState), 
               systemStateToString(newState));
#endif
    // 恢复基础速度
    m_navigationController.setBaseSpeed(FOLLOW_SPEED);
    // 执行状态退出操作
    if (m_currentState == OBJECT_GRAB || 
        m_currentState == OBJECT_PLACING || 
        m_currentState == OBJECT_RELEASE) {
        // 重置计时器
        m_actionStartTime = 0;
    }
    
    // 更新状态
    m_currentState = newState;
    
    // 执行状态进入操作
    if (newState == OBJECT_FIND) {
        // 重置区域计数器
        if (m_currentState != ERGODIC_JUDGE) { // 如果不是从遍历判断进入，才重置
            m_zoneCounter = 0;
        }
        // 在此状态禁用避障
        m_navigationController.setObstacleAvoidanceEnabled(true);
        // 开始巡线
        m_navigationController.resumeFollowing();
    }
    else if (newState == CONTINUE_SEARCH) {
        // 继续搜索状态 - 不重置区域计数器
        // 保持zoneCounter值，继续巡线
        m_navigationController.resumeFollowing();
    }
    else if (newState == ULTRASONIC_DETECT) {
        // 准备进行超声波检测
        m_navigationController.setObstacleAvoidanceEnabled(false);
        m_actionStartTime = 0; // 确保计时器重置
    }
    else if (newState == OBJECT_PLACING) {
        // 重置颜色计数器
        m_navigationController.setObstacleAvoidanceEnabled(true);
        m_colorCounter = 1;
    }
    else if (newState == END) {
        // 停止所有动作
        m_motionController.emergencyStop();
    }
}

/**
 * 记录状态转换
 */
void SimpleStateMachine::logStateTransition(SystemState oldState, SystemState newState) {
#if USE_MINIMAL_LOGGING == 0
    Logger::info("SimpleStateMachine", F("状态转换: %s -> %s"), 
                systemStateToString(oldState), 
                systemStateToString(newState));
#endif
}

/**
 * 系统状态转字符串
 */
const char* SimpleStateMachine::systemStateToString(SystemState state) {
    switch (state) {
    
        case OBJECT_FIND: return "OBJECT_FIND";
        case ULTRASONIC_DETECT: return "ULTRASONIC_DETECT";
        case CONTINUE_SEARCH: return "CONTINUE_SEARCH";
        case OBJECT_GRAB: return "OBJECT_GRAB";
        case OBJECT_PLACING: return "OBJECT_PLACING";
        case COUNT_INTERSECTION: return "COUNT_INTERSECTION";
        case ERGODIC_JUDGE: return "ERGODIC_JUDGE";
        case BACK_OBJECT_FIND: return "BACK_OBJECT_FIND";
        case RETURN_BASE: return "RETURN_BASE";
        case BASE_ARRIVE: return "BASE_ARRIVE";
        case END: return "END";
        case ERROR_STATE: return "ERROR_STATE";
        default: return "UNKNOWN_STATE";
    }
}

/**
 * 路口类型转字符串函数
 */
const char* SimpleStateMachine::junctionTypeToString(JunctionType type) const {
    // 使用PROGMEM存储字符串常量
    static const char j0[] PROGMEM = "无路口";
    static const char j1[] PROGMEM = "T型左路口";
    static const char j2[] PROGMEM = "T型右路口";
    static const char j3[] PROGMEM = "T型前路口";
    static const char j4[] PROGMEM = "十字路口";
    static const char j5[] PROGMEM = "左转弯";
    static const char j6[] PROGMEM = "右转弯";
    static const char j7[] PROGMEM = "线路终点";
    static const char j8[] PROGMEM = "未知路口";
    
    static const char* const string_table[] PROGMEM = {
        j0, j1, j2, j3, j4, j5, j6, j7, j8
    };
    
    if (type > END_OF_LINE) {
        return (const char*)pgm_read_word(&string_table[8]);
    }
    
    return (const char*)pgm_read_word(&string_table[type]);
}

/**
 * 处理上位机命令
 */
void SimpleStateMachine::handleCommand(const char* command) {
    // 处理上位机发来的命令，例如启动、停止、重置等
    if (strcmp(command, "START") == 0 && m_currentState == INITIALIZED) {
        transitionTo(OBJECT_FIND);
    }
    else if (strcmp(command, "STOP") == 0) {
        m_motionController.emergencyStop();
        m_navigationController.stop();
    }
    else if (strcmp(command, "RESET") == 0) {
        init();
        m_navigationController.init();
    }
}

/**
 * 获取当前系统状态
 */
SystemState SimpleStateMachine::getCurrentState() const {
    return m_currentState;
}

/**
 * 获取计数器值
 */
int SimpleStateMachine::getJunctionCounter() const {
    return m_zoneCounter;
}

/**
 * 获取检测到的颜色
 */
ColorCode SimpleStateMachine::getDetectedColor() const {
    return m_detectedColorCode;
}

// 新增的函数实现
ColorCode SimpleStateMachine::readColorCodeFromSerial2(unsigned long timeoutMillis) {
    //Logger::debug("StateMachine", "readColorCodeFromSerial2: 开始尝试读取颜色代码...");
    String colorStr = "";
    unsigned long startTime = millis();
    bool receivedData = false;
    ColorCode detectedColor = COLOR_UNKNOWN; // 默认值

    while (millis() - startTime < timeoutMillis) {
        if (Serial2.available() > 0) {
            receivedData = true;
            char c = Serial2.read();
            // Logger::debug("StateMachine", "Read char: %c (ASCII: %d)", isprint(c) ? c : '.', c);

            if (c == '\n' || c == '\r') {
                if (colorStr.length() > 0) {
                    // Logger::debug("StateMachine", "readColorCodeFromSerial2: 收到结束符，字符串为: '%s'", colorStr.c_str());
                    break; // 收到结束符且字符串非空，结束读取
                } else {
                    // 忽略开头的或连续的换行/回车符
                    continue;
                }
            }
            // 只添加数字字符到字符串，防止接收到其他调试信息干扰
            if (isdigit(c)) {
                 colorStr += c;
            }
             else {
                 // Logger::warning("StateMachine", "readColorCodeFromSerial2: 收到非数字字符 '%c' (ASCII: %d)，已忽略。", isprint(c) ? c : '.', c);
             }

        } 
        // 不需要delay，让循环尽快检查 Serial2
    }

    if (colorStr.length() > 0) {
        // Logger::debug("StateMachine", "readColorCodeFromSerial2: 读取完成，最终字符串: '%s'", colorStr.c_str());
        int colorValue = colorStr.toInt();
        // Logger::debug("StateMachine", "readColorCodeFromSerial2: 转换为整数: %d", colorValue);

        if (colorValue >= 1 && colorValue <= 5) {
            detectedColor = static_cast<ColorCode>(colorValue);
            Logger::info("StateMachine", "readColorCodeFromSerial2: 解析到有效颜色代码: %d", detectedColor);
        } else {
            Logger::warning("StateMachine", "readColorCodeFromSerial2: 收到无效或超出范围的颜色代码值: %d (来自字符串 '%s')", colorValue, colorStr.c_str());
            detectedColor = COLOR_UNKNOWN;
        }
    } else if (receivedData) {
        // Logger::debug("StateMachine", "readColorCodeFromSerial2: 只收到了换行/回车或非数字字符，未读取到有效颜色代码。");
        detectedColor = COLOR_UNKNOWN;
    } else {
        // 超时
        Logger::warning("StateMachine", "readColorCodeFromSerial2: 读取颜色代码超时 (%lu ms)，未收到有效数据。", timeoutMillis);
        detectedColor = COLOR_UNKNOWN;
    }

    // Logger::debug("StateMachine", "readColorCodeFromSerial2: 函数返回: %d", detectedColor);
    return detectedColor;
} 
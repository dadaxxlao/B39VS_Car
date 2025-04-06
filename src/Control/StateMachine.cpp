#include "StateMachine.h"

// 调试宏定义，启用更详细的状态机日志
#define DEBUG_STATE_MACHINE 1

// 定义特殊操作的超时时间（毫秒）
#define GRAB_TIMEOUT         3000
#define PLACING_TIMEOUT      3000
#define TURN_AROUND_TIMEOUT  2000
#define FORWARD_TIMEOUT      1000
#define BASE_ARRIVE_FORWARD_TIME 2000

// 定义阈值常量
#define OBJECT_DETECTION_THRESHOLD 30.0f  // 物体检测阈值，单位cm

/**
 * 构造函数
 */
StateMachine::StateMachine(SensorManager& sm, MotionController& mc, 
                           RoboticArm& arm, NavigationController& nc)
    : m_sensorManager(sm)
    , m_motionController(mc)
    , m_roboticArm(arm)
    , m_navigationController(nc)
    , m_currentState(INITIALIZED)
    , m_locateSubState(TURN_AROUND)
    , m_zoneCounter(0)
    , m_colorCounter(0)
    , m_detectedColorCode(COLOR_UNKNOWN)
    , m_actionStartTime(0)
    , m_isActionComplete(false)
{
    // 构造函数中只进行初始化，不执行实际操作
}

/**
 * 初始化函数
 */
void StateMachine::init() {
    // 设置初始状态
    m_currentState = INITIALIZED;
    m_zoneCounter = 0;
    m_colorCounter = 0;
    m_detectedColorCode = COLOR_UNKNOWN;
    
    // 记录初始化
    Logger::info("StateMachine", "状态机初始化完成，等待启动信号...");
}

/**
 * 主循环更新函数
 */
void StateMachine::update() {
    // 更新传感器数据
    m_sensorManager.updateAll();
    
    // 根据当前状态执行相应处理逻辑
    switch (m_currentState) {
        case INITIALIZED:
            handleInitialized();
            break;
        case OBJECT_FIND:
            handleObjectFind();
            break;
        case ULTRASONIC_DETECT:
            handleUltrasonicDetect();
            break;
        case ZONE_JUDGE:
            handleZoneJudge();
            break;
        case ZONE_TO_BASE:
            handleZoneToBase();
            break;
        case OBJECT_GRAB:
            handleObjectGrab();
            break;
        case OBJECT_PLACING:
            handleObjectPlacing();
            break;
        case COUNT_INTERSECTION:
            handleCountIntersection();
            break;
        case OBJECT_RELEASE:
            handleObjectRelease();
            break;
        case ERGODIC_JUDGE:
            handleErgodicJudge();
            break;
        case BACK_OBJECT_FIND:
            handleBackObjectFind();
            break;
        case RETURN_BASE:
            handleReturnBase();
            break;
        case BASE_ARRIVE:
            handleBaseArrive();
            break;
        case END:
            handleEnd();
            break;
        case ERROR_STATE:
            handleErrorState();
            break;
        default:
            Logger::error("StateMachine", "未知系统状态：%d", m_currentState);
            transitionTo(ERROR_STATE);
            break;
    }
}

/**
 * 处理初始化状态
 */
void StateMachine::handleInitialized() {
    // 等待上位机启动信号或传感器触发
    // 简化为使用超声波距离作为启动条件
    float distance = m_sensorManager.getUltrasonicDistance();
    
    // 如果检测到物体接近（小于30cm），则启动任务
    if (distance < 30.0f && distance > 0) {
        Logger::info("StateMachine", "检测到启动触发，距离: %.2f cm", distance);
        transitionTo(OBJECT_FIND);
    }
}

/**
 * 处理寻找物块状态
 */
void StateMachine::handleObjectFind() {
    // 每次进入此状态都重置区域计数器
    m_zoneCounter = 0;
    
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        Logger::info("StateMachine", "OBJECT_FIND状态 - 检测到路口: %s", 
                     this->junctionTypeToString(junction));
        
        // 根据路口类型和OBJECT_FIND状态决定行动
        switch (junction) {
            case T_LEFT: // 左旋T字形路口
            case LEFT_TURN: // 左转弯
                // 直接进入超声波检测状态，左转操作在ULTRASONIC_DETECT开始时执行
                transitionTo(ULTRASONIC_DETECT);
                break;
                
            case T_RIGHT: // 右旋T字形路口
                // 直行
                m_navigationController.resumeFollowing();
                break;
                
            case RIGHT_TURN: // 右转弯
            case T_FORWARD: // 正T字（倒T）
                // 右转
                m_motionController.turnRight(TURN_SPEED);
                m_navigationController.resumeFollowing();
                break;
                
            default:
                // 对于其他情况，恢复巡线
                m_navigationController.resumeFollowing();
                break;
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理超声波检测状态
 */
void StateMachine::handleUltrasonicDetect() {
    // 执行左转操作（从OBJECT_FIND状态移到这里）
    if (m_actionStartTime == 0) {
        // 执行左转
        m_motionController.turnLeft(TURN_SPEED);
        
        // 计数器加一（标识当前是哪个Zone）
        m_zoneCounter++;
        m_actionStartTime = millis();
        Logger::info("StateMachine", "进入超声波检测状态，执行左转，区域计数: %d", m_zoneCounter);
    }
    
    // 超声波检测逻辑
    float distance = m_sensorManager.getUltrasonicDistance();
    
    // 检测物块
    if (distance < OBJECT_DETECTION_THRESHOLD) {
        // 检测到物块
        Logger::info("StateMachine", "检测到物块，距离: %.2f cm", distance);
        m_actionStartTime = 0;
        transitionTo(OBJECT_GRAB);
    } else if (millis() - m_actionStartTime > 1000) { // 等待1秒确认没有物块
        // 未检测到物块
        Logger::info("StateMachine", "未检测到物块");
        m_actionStartTime = 0;
        transitionTo(ZONE_JUDGE);
    }
}

/**
 * 处理区域判断状态
 */
void StateMachine::handleZoneJudge() {
    // 根据区域计数器进行判断
    if (m_zoneCounter <= 2) {
        // 检查是否是刚进入该状态
        if (m_actionStartTime == 0) {
            // 执行右转操作
            Logger::info("StateMachine", "区域%d判断：执行右转，继续巡线", m_zoneCounter);
            m_motionController.turnRight(TURN_SPEED);
            m_navigationController.resumeFollowing();
            m_actionStartTime = millis(); // 标记已执行右转
        }
        
        // 持续更新导航控制器
        m_navigationController.update();
        
        // 获取当前导航状态
        NavigationState navState = m_navigationController.getCurrentNavigationState();
        
        // 如果到达路口，检查是否为左转或左T路口
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
            if (junction == T_LEFT || junction == LEFT_TURN) {
                Logger::info("StateMachine", "区域%d检测到左转或左T路口，进入超声波检测状态", m_zoneCounter);
                m_actionStartTime = 0; // 重置状态变量
                transitionTo(ULTRASONIC_DETECT);
            } else {
                // 非左转或左T路口，继续巡线
                m_navigationController.resumeFollowing();
            }
        }
        // 处理导航错误
        else if (navState == NAV_ERROR) {
            Logger::error("StateMachine", "导航错误，进入ERROR状态");
            m_actionStartTime = 0; // 重置状态变量
            transitionTo(ERROR_STATE);
        }
    } else if (m_zoneCounter == 3) {
        // 执行左转操作
        Logger::info("StateMachine", "区域3判断：执行左转进入ZONE_TO_BASE");
        m_motionController.turnLeft(TURN_SPEED);
        m_navigationController.resumeFollowing();
        transitionTo(ZONE_TO_BASE);
    }
}

/**
 * 处理区域到基地状态
 */
void StateMachine::handleZoneToBase() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        Logger::info("StateMachine", "ZONE_TO_BASE状态 - 检测到路口: %s", 
                     this->junctionTypeToString(junction));
        
        // 根据路口类型决定行动
        switch (junction) {
            case LEFT_TURN: // 左转弯
                m_motionController.turnLeft(TURN_SPEED);
                m_navigationController.resumeFollowing();
                break;
                
            case T_FORWARD: // T字形路口
                transitionTo(BASE_ARRIVE);
                break;
                
            default:
                // 对于其他情况，默认直行
                m_navigationController.resumeFollowing();
                break;
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理抓取物块状态
 */
void StateMachine::handleObjectGrab() {
    // 如果刚进入状态，记录开始时间
    if (m_actionStartTime == 0) {
        m_actionStartTime = millis();
        m_isActionComplete = false;
        
        // 向物体靠近到抓取距离
        float distance = m_sensorManager.getUltrasonicDistance();
        if (distance > GRAB_DISTANCE) {
            m_motionController.moveForward(FOLLOW_SPEED / 2); // 低速接近
        } else {
            m_motionController.emergencyStop();
            // 执行抓取动作
            m_roboticArm.grab();
            
            // 颜色检测
            m_detectedColorCode = m_sensorManager.getColor();
            Logger::info("StateMachine", "检测到物体颜色: %d", m_detectedColorCode);
            
            m_isActionComplete = true;
        }
    }
    
    // 检查抓取是否完成或超时
    if (m_isActionComplete || (millis() - m_actionStartTime > GRAB_TIMEOUT)) {
        m_actionStartTime = 0; // 重置计时器
        
        // 掉头
        m_motionController.uTurn();
        
        // 进入放置状态
        transitionTo(OBJECT_PLACING);
    }
}

/**
 * 处理放置物体状态
 */
void StateMachine::handleObjectPlacing() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        Logger::info("StateMachine", "OBJECT_PLACING状态 - 检测到路口: %s", 
                     this->junctionTypeToString(junction));
        
        switch (junction) {
            case RIGHT_TURN:
            case T_FORWARD:
                // 右转
                m_motionController.turnRight(TURN_SPEED);
                m_navigationController.resumeFollowing();
                break;
                
            case T_RIGHT:
                // 直行
                m_navigationController.resumeFollowing();
                break;
                
            case LEFT_TURN:
                // 左转
                m_motionController.turnLeft(TURN_SPEED);
                m_navigationController.resumeFollowing();
                break;
                
            case T_LEFT:
                // 左T字路口左转，初始化计数器，进入COUNT_INTERSECTION状态
                m_motionController.turnLeft(TURN_SPEED);
                m_colorCounter = 0; // 重置颜色计数器
                transitionTo(COUNT_INTERSECTION);
                break;
                
            default:
                // 继续巡线
                m_navigationController.resumeFollowing();
                break;
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理路口计数状态
 */
void StateMachine::handleCountIntersection() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        if (junction == T_RIGHT) {
            // 如果计数器等于颜色代号，执行右转进入释放状态
            if (m_colorCounter == m_detectedColorCode) {
                Logger::info("StateMachine", "达到目标区域，执行右转");
                m_motionController.turnRight(TURN_SPEED);
                transitionTo(OBJECT_RELEASE);
            } else {
                // 否则计数器加1，继续巡线
                m_colorCounter++;
                Logger::info("StateMachine", "颜色计数: %d/%d", m_colorCounter, m_detectedColorCode);
                m_navigationController.resumeFollowing();
            }
        } else {
            // 继续巡线
            m_navigationController.resumeFollowing();
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理释放物体状态
 */
void StateMachine::handleObjectRelease() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 检查是否刚进入状态
    if (m_actionStartTime == 0) {
        m_actionStartTime = millis();
        m_isActionComplete = false;
        Logger::info("StateMachine", "进入物体释放状态，循迹前进直到检测到全黑");
    }
    
    // 第一阶段：寻找全黑线并放置物体，然后掉头
    if (!m_isActionComplete) {
        // 如果到达路口（全黑模式/T字形路口），执行放置操作
        if (navState == NAV_AT_JUNCTION) {
            JunctionType junction = m_navigationController.getDetectedJunctionType();
            
            // 确认是否为T_FORWARD（全黑模式）
            if (junction == T_FORWARD) {
                Logger::info("StateMachine", "检测到全黑模式/T字路口，停止执行放置操作");
                m_motionController.emergencyStop();
                
                // 执行放置动作
                m_roboticArm.release();
                Logger::info("StateMachine", "物体放置完成");
                
                // 等待放置完成
                delay(1000);
                
                // 掉头
                Logger::info("StateMachine", "开始掉头");
                m_motionController.uTurn();
                delay(2000); // 等待掉头完成
                
                // 恢复巡线，直接进入遍历判断状态
                Logger::info("StateMachine", "掉头完成，直接进入遍历判断状态");
                m_navigationController.resumeFollowing();
                
                // 重置状态变量
                m_actionStartTime = 0;
                m_isActionComplete = false;
                
                // 直接进入遍历判断状态
                transitionTo(ERGODIC_JUDGE);
            } else {
                // 其他类型路口，继续巡线
                m_navigationController.resumeFollowing();
            }
        }
    }
    
    // 处理导航错误
    if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        
        // 重置状态变量
        m_actionStartTime = 0;
        m_isActionComplete = false;
        
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理遍历判断状态
 */
void StateMachine::handleErgodicJudge() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        if (junction == T_LEFT || junction == LEFT_TURN) {
            // 左转
            m_motionController.turnLeft(TURN_SPEED);
            
            // 根据区域计数器进行判断
            if (m_zoneCounter <= 2) {
                Logger::info("StateMachine", "区域%d未遍历完，进入返回寻找物块状态", m_zoneCounter);
                transitionTo(BACK_OBJECT_FIND);
            } else if (m_zoneCounter == 3) {
                Logger::info("StateMachine", "所有区域已遍历，返回基地");
                transitionTo(RETURN_BASE);
            }
        } else {
            // 继续巡线
            m_navigationController.resumeFollowing();
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理返回寻找物块状态
 */
void StateMachine::handleBackObjectFind() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        Logger::info("StateMachine", "BACK_OBJECT_FIND状态 - 检测到路口: %s", 
                     this->junctionTypeToString(junction));
        
        // 遇到T字形路口执行右转随后进入OBJECT_FIND状态
        if (junction == T_FORWARD) {
            Logger::info("StateMachine", "检测到T字路口，执行右转进入OBJECT_FIND状态");
            m_motionController.turnRight(TURN_SPEED);
            transitionTo(OBJECT_FIND);
        } else {
            // 继续巡线
            m_navigationController.resumeFollowing();
        }
    }
    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理返回基地状态
 */
void StateMachine::handleReturnBase() {
    // 更新导航控制器
    m_navigationController.update();
    
    // 获取当前导航状态
    NavigationState navState = m_navigationController.getCurrentNavigationState();
    
    // 如果到达路口，执行路口决策
    if (navState == NAV_AT_JUNCTION) {
        JunctionType junction = m_navigationController.getDetectedJunctionType();
        
        Logger::info("StateMachine", "RETURN_BASE状态 - 检测到路口: %s", 
                     this->junctionTypeToString(junction));
        
        if (junction == T_FORWARD) {
            // T字路口左转
            m_motionController.turnLeft(TURN_SPEED);
            
            // 可以在这里设置一个标记，表示已经执行过左转，
            // 下次遇到路口就进入BASE_ARRIVE状态
            if (m_actionStartTime == 0) {
                m_actionStartTime = millis(); // 使用actionStartTime作为标记
            } else {
                // 如果已经左转过一次，再次检测到路口说明到达基地
                transitionTo(BASE_ARRIVE);
            }
        } else {
            // 继续巡线
            m_navigationController.resumeFollowing();
        }
    }

    // 处理导航错误
    else if (navState == NAV_ERROR) {
        Logger::error("StateMachine", "导航错误，进入ERROR状态");
        transitionTo(ERROR_STATE);
    }
}

/**
 * 处理到达基地状态
 */
void StateMachine::handleBaseArrive() {
    // 到达基地边缘，继续直行一段时间
    if (m_actionStartTime == 0) {
        m_actionStartTime = millis();
        m_motionController.moveForward(FOLLOW_SPEED);
        Logger::info("StateMachine", "到达基地边缘，继续前进一段时间");
    }
    
    // 直行设定时长后进入END状态
    if (millis() - m_actionStartTime > BASE_ARRIVE_FORWARD_TIME) {
        m_motionController.emergencyStop();
        Logger::info("StateMachine", "到达基地，任务完成");
        transitionTo(END);
    }
}

/**
 * 处理结束状态
 */
void StateMachine::handleEnd() {
    // 停止所有动作
    m_motionController.emergencyStop();
    
    // 可以添加结束任务的其他操作，如LED指示、声音提示等
    
    // 等待重置信号或新任务开始
}

/**
 * 处理错误状态
 */
void StateMachine::handleErrorState() {
    // 停止所有动作
    m_motionController.emergencyStop();
    
    // 闪烁LED或发出警报（如果有）
    
    // 可以实现错误恢复策略
    // 例如，重置传感器、重新初始化导航等
    
    // 简单示例：检测到超声波距离变化作为重置信号
    float distance = m_sensorManager.getUltrasonicDistance();
    static float lastDistance = distance;
    
    if (abs(distance - lastDistance) > 20.0f) {
        Logger::info("StateMachine", "检测到重置信号，重新初始化");
        init();
        m_navigationController.init();
        transitionTo(INITIALIZED);
    }
    
    lastDistance = distance;
}

/**
 * 获取当前系统状态
 */
SystemState StateMachine::getCurrentState() const {
    return m_currentState;
}

/**
 * 获取计数器值
 */
int StateMachine::getJunctionCounter() const {
    return m_zoneCounter;
}

/**
 * 获取检测到的颜色
 */
ColorCode StateMachine::getDetectedColor() const {
    return m_detectedColorCode;
}

/**
 * 状态转换函数
 */
void StateMachine::transitionTo(SystemState newState) {
    // 记录状态转换
    logStateTransition(m_currentState, newState);
    
    // 执行状态退出操作
    switch (m_currentState) {
        case OBJECT_GRAB:
        case OBJECT_PLACING:
        case OBJECT_RELEASE:
            // 重置计时器
            m_actionStartTime = 0;
            break;
        default:
            break;
    }
    
    // 更新状态
    m_currentState = newState;
    
    // 执行状态进入操作
    switch (newState) {
        case OBJECT_FIND:
            // 重置区域计数器
            if (m_currentState != ERGODIC_JUDGE) { // 如果不是从遍历判断进入，才重置
                m_zoneCounter = 0;
            }
            // 开始巡线
            m_navigationController.resumeFollowing();
            break;
        case ULTRASONIC_DETECT:
            // 准备进行超声波检测
            m_actionStartTime = 0; // 确保计时器重置
            break;
        case OBJECT_PLACING:
            // 重置颜色计数器
            m_colorCounter = 0;
            break;
        case END:
            // 停止所有动作
            m_motionController.emergencyStop();
            break;
        default:
            break;
    }
}

/**
 * 记录状态转换
 */
void StateMachine::logStateTransition(SystemState oldState, SystemState newState) {
    Logger::info("StateMachine", "状态转换: %s -> %s", 
                systemStateToString(oldState), 
                systemStateToString(newState));
}

/**
 * 系统状态转字符串
 */
const char* StateMachine::systemStateToString(SystemState state) const {
    switch (state) {
        case INITIALIZED: return "INITIALIZED";
        case OBJECT_FIND: return "OBJECT_FIND";
        case ULTRASONIC_DETECT: return "ULTRASONIC_DETECT";
        case ZONE_JUDGE: return "ZONE_JUDGE";
        case ZONE_TO_BASE: return "ZONE_TO_BASE";
        case OBJECT_GRAB: return "OBJECT_GRAB";
        case OBJECT_PLACING: return "OBJECT_PLACING";
        case COUNT_INTERSECTION: return "COUNT_INTERSECTION";
        case OBJECT_RELEASE: return "OBJECT_RELEASE";
        case ERGODIC_JUDGE: return "ERGODIC_JUDGE";
        case BACK_OBJECT_FIND: return "BACK_OBJECT_FIND";
        case RETURN_BASE: return "RETURN_BASE";
        case BASE_ARRIVE: return "BASE_ARRIVE";
        case END: return "END";
        case ERROR_STATE: return "ERROR_STATE";
        default: return "UNKNOWN";
    }
}

/**
 * 处理上位机命令
 */
void StateMachine::handleCommand(const char* command) {
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
 * 路口类型转字符串函数
 */
const char* StateMachine::junctionTypeToString(JunctionType type) const {
    switch(type) {
        case NO_JUNCTION: return "无路口";
        case T_LEFT: return "T型左路口";
        case T_RIGHT: return "T型右路口";
        case T_FORWARD: return "T型前路口";
        case CROSS: return "十字路口";
        case LEFT_TURN: return "左转弯";
        case RIGHT_TURN: return "右转弯";
        case END_OF_LINE: return "线路终点";
        default: return "未知路口";
    }
} 
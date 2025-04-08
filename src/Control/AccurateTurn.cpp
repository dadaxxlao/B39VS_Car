#include "AccurateTurn.h"

// Define the default timeout duration in milliseconds
// Consider moving this to Config.h if it needs to be configurable
static const unsigned long DEFAULT_ACCURATE_TURN_TIMEOUT_MS = 10000; 
static const unsigned long DEFAULT_ACCURATE_TURN_DELAY_MS = 500;

AccurateTurn::AccurateTurn(MotionController& mc, SensorManager& sm)
    : m_motionController(mc)
    , m_sensorManager(sm)
    , m_currentState(AT_IDLE)
    , m_startTime(0)
    , m_targetSpeed(TURN_SPEED)
    , m_timeoutDuration(DEFAULT_ACCURATE_TURN_TIMEOUT_MS) // Use the defined constant
{
    Logger::info("AccurateTurn", "AccurateTurn module created.");
}

void AccurateTurn::init() {
    m_currentState = AT_IDLE;
    Logger::info("AccurateTurn", "AccurateTurn module initialized. State: IDLE");
}

void AccurateTurn::startTurnLeft(int speed) {
    if (m_currentState != AT_IDLE) {
        Logger::warning("AccurateTurn", "Cannot start left turn, already active (State: %d)", m_currentState);
        return;
    }
    m_currentState = AT_TURNING_LEFT;
    m_targetSpeed = speed;
    m_startTime = millis();
    m_motionController.spinLeft(m_targetSpeed);
    Logger::info("AccurateTurn", "Starting Left Turn (Speed: %d, Timeout: %lu ms)", m_targetSpeed, m_timeoutDuration);
    delay(DEFAULT_ACCURATE_TURN_DELAY_MS);
}

void AccurateTurn::startTurnRight(int speed) {
    if (m_currentState != AT_IDLE) {
        Logger::warning("AccurateTurn", "Cannot start right turn, already active (State: %d)", m_currentState);
        return;
    }
    m_currentState = AT_TURNING_RIGHT;
    m_targetSpeed = speed;
    m_startTime = millis();
    m_motionController.spinRight(m_targetSpeed);
    Logger::info("AccurateTurn", "Starting Right Turn (Speed: %d, Timeout: %lu ms)", m_targetSpeed, m_timeoutDuration);
    delay(DEFAULT_ACCURATE_TURN_DELAY_MS);
}

void AccurateTurn::startUTurn(int speed) {
    if (m_currentState != AT_IDLE) {
        Logger::warning("AccurateTurn", "Cannot start U-turn, already active (State: %d)", m_currentState);
        return;
    }
    m_currentState = AT_TURNING_UTURN;
    m_targetSpeed = speed;
    m_startTime = millis();
    // U-Turn implemented as spinning left
    m_motionController.spinLeft(m_targetSpeed);
    Logger::info("AccurateTurn", "Starting U-Turn (Spin Left, Speed: %d, Timeout: %lu ms)", m_targetSpeed, m_timeoutDuration);
    delay(DEFAULT_ACCURATE_TURN_DELAY_MS);
}

void AccurateTurn::update() {
    // If idle or already finished, do nothing
    if (m_currentState == AT_IDLE || m_currentState == AT_COMPLETED || m_currentState == AT_TIMED_OUT) {
        return;
    }

    unsigned long currentTime = millis();

    // 1. Check for Timeout
    if (currentTime - m_startTime > m_timeoutDuration) {
        m_motionController.emergencyStop();
        m_currentState = AT_TIMED_OUT;
        Logger::warning("AccurateTurn", "Turn timed out after %lu ms! State: TIMED_OUT", m_timeoutDuration);
        return;
    }

    // 2. Read Infrared Sensors
    uint16_t sensorValues[8];
    bool success = m_sensorManager.getInfraredSensorValues(sensorValues);

    if (!success) {
        Logger::warning("AccurateTurn", "Failed to read IR sensors during turn. Continuing turn.");
        // Do not return here; rely on timeout if sensors persistently fail.
        // Alternatively, implement an error counter and stop after too many failures.
        return; // Decision from plan: Return on sensor read failure
    }

    // 3. Check for Stop Condition (Line Detected)
    // Assuming 0 means black line is detected on sensors 3 and 4 (middle ones)
    // IMPORTANT: Verify sensor indexing and black line value for your specific hardware!
    switch (m_currentState) {
        case AT_TURNING_LEFT:
            if (sensorValues[4] == 0 || sensorValues[3] == 0) {
                m_motionController.emergencyStop();
                m_currentState = AT_COMPLETED;
                break;
            }
            break;
        case AT_TURNING_RIGHT:
            if (sensorValues[3] == 0 || sensorValues[4] == 0) {
                m_motionController.emergencyStop();
                m_currentState = AT_COMPLETED;
                break;
            }
            break;  
        case AT_TURNING_UTURN:
            if (sensorValues[4] == 0 || sensorValues[3] == 0) {
                m_motionController.emergencyStop();
                m_currentState = AT_COMPLETED;
                break;
            }
            break;
        default:
            break;
    }
    

    }
    
    
    // If still turning and no stop condition met, continue (motor command persists)
    // Logger::debug("AccurateTurn", "Turning... State: %d", m_currentState); // Optional debug log



AccurateTurnState AccurateTurn::getCurrentState() const {
    return m_currentState;
}

bool AccurateTurn::isTurnComplete() const {
    return (m_currentState == AT_COMPLETED || m_currentState == AT_TIMED_OUT);
}

void AccurateTurn::reset() {
    if (m_currentState != AT_IDLE) {
        m_currentState = AT_IDLE;
        Logger::info("AccurateTurn", "State reset to IDLE.");
        return;
    } else {
        // Optionally log that it was already idle, or do nothing
        // Logger::debug("AccurateTurn", "Reset called but already in IDLE state.");
    }
} 
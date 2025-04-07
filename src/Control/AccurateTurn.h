#ifndef ACCURATE_TURN_H
#define ACCURATE_TURN_H

#include <Arduino.h>
#include "../Motor/MotionController.h"
#include "../Sensor/SensorManager.h"
#include "../Utils/Config.h"  // Assuming TURN_SPEED might be here
#include "../Utils/Logger.h"

// Enum defining the possible states of the AccurateTurn module
enum AccurateTurnState {
    AT_IDLE,          // Not currently turning
    AT_TURNING_LEFT,  // Executing an accurate left turn
    AT_TURNING_RIGHT, // Executing an accurate right turn
    AT_TURNING_UTURN, // Executing a U-turn (e.g., spinning left until line detected)
    AT_COMPLETED,     // Turn successfully completed (line detected)
    AT_TIMED_OUT      // Turn stopped due to timeout
};

class AccurateTurn {
public:
    /**
     * @brief Constructor for AccurateTurn.
     * @param mc Reference to the MotionController instance.
     * @param sm Reference to the SensorManager instance.
     */
    AccurateTurn(MotionController& mc, SensorManager& sm);

    /**
     * @brief Initializes the AccurateTurn module. Sets state to IDLE.
     */
    void init();

    /**
     * @brief Starts an accurate left turn.
     * The turn continues until the middle IR sensors detect the line or a timeout occurs.
     * Only starts if the current state is AT_IDLE.
     * @param speed The spinning speed (0-255). Defaults to TURN_SPEED from Config.h if available, otherwise a default.
     */
    void startTurnLeft(int speed = TURN_SPEED);

    /**
     * @brief Starts an accurate right turn.
     * The turn continues until the middle IR sensors detect the line or a timeout occurs.
     * Only starts if the current state is AT_IDLE.
     * @param speed The spinning speed (0-255). Defaults to TURN_SPEED.
     */
    void startTurnRight(int speed = TURN_SPEED);

    /**
     * @brief Starts a U-turn (implemented as spinning left until line detected).
     * The turn continues until the middle IR sensors detect the line or a timeout occurs.
     * Only starts if the current state is AT_IDLE.
     * @param speed The spinning speed (0-255). Defaults to TURN_SPEED.
     */
    void startUTurn(int speed = TURN_SPEED);

    /**
     * @brief Updates the state machine for AccurateTurn.
     * This function should be called repeatedly in the main loop.
     * It checks for line detection or timeout to stop the turn.
     */
    void update();

    /**
     * @brief Gets the current state of the AccurateTurn module.
     * @return The current AccurateTurnState.
     */
    AccurateTurnState getCurrentState() const;

    /**
     * @brief Checks if the turn operation has finished (either completed or timed out).
     * @return True if the state is AT_COMPLETED or AT_TIMED_OUT, false otherwise.
     */
    bool isTurnComplete() const;

private:
    MotionController& m_motionController; // Reference to the motion controller
    SensorManager& m_sensorManager;     // Reference to the sensor manager
    AccurateTurnState m_currentState;   // Current state of the turn operation
    unsigned long m_startTime;          // Timestamp when the turn started (for timeout)
    int m_targetSpeed;                  // The speed for the current turn operation
    unsigned long m_timeoutDuration;    // Maximum duration allowed for a turn (in milliseconds)
};

#endif // ACCURATE_TURN_H 
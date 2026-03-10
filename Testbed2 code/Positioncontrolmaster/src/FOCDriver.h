#include <ACAN_T4.h>
#include <Arduino.h>

struct variable {
    float value = 0;
    unsigned long lastUpdate = 0;
};

/**
 * @file FOCDriver.h
 * @brief Contains the declaration of the FOCDriver class, which allows control of an FOC driver through a CAN bus.
 */

class FOCDriver {
/**
 * @class FOCDriver
 * @brief The FOCDriver class allows control of an FOC driver through a CAN bus.
 */
private : 
    // CAN
    ACAN_T4* CAN; /**< Pointer to the CAN object used for communication. */
    uint8_t ID ; /**< ID of the FOC driver. */

    // motor state
    variable alpha; /**< Current alpha value of the motor. */
    variable alphaVelocity; /**< Current velocity of the alpha value. */
    variable targetForce; /**< Target force for the motor. */

    // methods
    bool sendMessage(uint8_t service_id, uint8_t sub_service_id, float data); /**< Sends a CAN message with float data. */
    bool sendMessage(uint8_t service_id, uint8_t sub_service_id, uint8_t data); /**< Sends a CAN message with uint8_t data. */
    bool sendMessage(uint8_t service_id, uint8_t sub_service_id); /**< Sends a CAN message without data. */
    

public:
    // constructor
    FOCDriver(); /**< Default constructor. */
    FOCDriver(ACAN_T4* _CAN, uint8_t _ID); /**< Constructor that initializes the FOCDriver object with the given CAN object and ID. */

    // methods
    void processMessage(CANMessage message); /**< Runs the FOCDriver. */

    bool setTargetForce(float targetForce); /**< Sets the target force for the motor. */

    float getAlpha(); /**< Gets the alpha value of the motor. */
    float getAlphaVelocity(); /**< Gets the velocity of the alpha value. */

    void updateAlpha(); /**< Updates the alpha value of the motor. */
    void updateAlphaVelocity(); /**< Updates the velocity of the alpha value. */
};

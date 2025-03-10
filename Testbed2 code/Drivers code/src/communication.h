#include <Arduino.h>
#include "STM32_CAN.h"
#include "SimpleFOC.h"
#include "debug.h"


struct variable {
    float value = 0;
    unsigned long lastUpdate = 0;
};

/**
 * @file Controller.h
 * @brief Contains the declaration of the Controller class, which allows control of an FOC driver through a CAN bus.
 */

class Controller {
/**
 * @class Controller
 * @brief The Controller class allows control of an FOC driver through a CAN bus.
 */
private : 
    // CAN
    STM32_CAN* CAN; /**< Pointer to the CAN object used for communication. */
    BLDCMotor* motor; /**< Pointer to the motor object. */
    MagneticSensorSPI* sensorBeta; /**< Pointer to the beta sensor object. */
    Debug* debug; /**< Pointer to the debug object. */

    // parameters
    uint8_t ID ; /**< ID of the FOC driver. */
    CAN_message_t CAN_RX_msg; /**< CAN message received. */

    // motor state
    variable alpha; /**< Current alpha value of the motor. */
    variable alphaVelocity; /**< Current velocity of the alpha value. */
    variable targetForce; /**< Target force for the motor. */

    // methods
    bool sendMessage(uint8_t service_id, uint8_t sub_service_id, float data); /**< Sends a CAN message with float data. */
    bool sendMessage(uint8_t service_id, uint8_t sub_service_id, uint8_t data); /**< Sends a CAN message with uint8_t data. */

    void floatToBytes(uint8_t bytes[8], float value); /**< Converts a float to bytes. */
    float bytesToFloat(uint8_t bytes[8]); /**< Converts bytes to a float. */
    void intToBytes(uint8_t bytes[8], int value); /**< Converts an integer to bytes. */
    int bytesToInt(uint8_t bytes[8]); /**< Converts bytes to an integer. */

public:
    // constructor
    Controller(); /**< Default constructor. */
    Controller(STM32_CAN* _CAN, uint8_t _ID, Debug* debug = nullptr); /**< Constructor that initializes the Controller object with the given CAN object and ID. */
    Controller(STM32_CAN* _CAN, uint8_t _ID, HardwareSerial* _serial); /**< Constructor that initializes the Controller object with the given CAN object and ID. */

    // methods
    void run(); /**< Runs the Controller. */
    void addMotor(BLDCMotor* motor);
    void addSensor(MagneticSensorSPI* sensorBeta);

    float getTargetForce();
    unsigned long getTargetForceLastUpdate();
};

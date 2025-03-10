#include <Arduino.h>
#include "STM32_CAN.h"
#include "SimpleFOC.h"
#include "debug.h"


struct variable {
    float value = 0;
    unsigned long lastUpdate = 0;
    float velocity = 0;
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
    //STM32_CAN* CAN; /**< Pointer to the CAN object used for communication. */
    BLDCMotor* motor; /**< Pointer to the motor object. */
    MagneticSensorSPI* sensorBeta; /**< Pointer to the beta sensor object. */
    Debug* debug; /**< Pointer to the debug object. */

    // parameters
    //uint8_t ID ; /**< ID of the FOC driver. */
    float maxTemp = 0; /**< Maximum temperature allowed for the FOC driver. */
    float maxCurrent = 0; /**< Maximum current allowed for the FOC driver. */
    uint8_t polePairs = 0; /**< Number of pole pairs of the motor. */
    float resistance = 0; /**< Resistance of the motor. */
    float TorqueConstant = 0; /**< Torque constant of the motor. */
    float offsetShaftAngleAlpha = 0.0; //< offset anggle in radian between the sensor and alpha, set trough the can bus

    //CAN_message_t CAN_RX_msg; /**< CAN message received. */

    // motor state
    variable alpha; /**< Current alpha value of the motor. */
    variable beta; /**< Current beta value of the motor. */
    variable alphaVelocity; /**< Current velocity of the alpha value. */
    variable betaVelocity; /**< Current velocity of the beta value. */
    variable temp; /**< Current temperature of the FOC driver. */
    variable targetForce; /**< Target force for the motor. */

    // methods
    //bool sendMessage(uint8_t service_id, uint8_t sub_service_id, float data); /**< Sends a CAN message with float data. */
    //bool sendMessage(uint8_t service_id, uint8_t sub_service_id, uint8_t data); /**< Sends a CAN message with uint8_t data. */
    //bool sendMessage(uint8_t service_id, uint8_t sub_service_id); /**< Sends a CAN message without data. */
    //bool sendMessage(uint8_t service_id,uint8_t sub_service_id, float data1, float data2);

    //void readCanMessage(); /**< Reads a CAN message. */

    void floatToBytes(uint8_t bytes[8], float value); /**< Converts a float to bytes. */
    float bytesToFloat(uint8_t bytes[8]); /**< Converts bytes to a float. */
    void intToBytes(uint8_t bytes[8], int value); /**< Converts an integer to bytes. */
    int bytesToInt(uint8_t bytes[8]); /**< Converts bytes to an integer. */

public:
    // constructor
    Controller(); /**< Default constructor. */
    Controller(Debug* debug = nullptr); /**< Constructor that initializes the Controller object with the given CAN object and ID. */
    Controller(HardwareSerial* _serial); /**< Constructor that initializes the Controller object with the given CAN object and ID. */

    // methods
    void init(float resistance, int polePairs, float TorqueConstant); /**< Initializes the Controller object with the specified motor parameters. */
    void run(); /**< Runs the Controller. */
    void addMotor(BLDCMotor* motor);
    void addSensorBeta(MagneticSensorSPI* sensorBeta);

    void setAlpha(float alpha); /**< Sets the alpha value of the motor. */

    bool setTargetForce(float targetForce); /**< Sets the target force for the motor. */
    bool setMaxTemp(float maxTemp); /**< Sets the maximum temperature allowed for the FOC driver. */
    bool setMaxCurrent(float maxCurrent); /**< Sets the maximum current allowed for the FOC driver. */
    bool setMotorParams(int polePairs, float resistance, float TorqueConstant); /**< Sets the motor parameters. */

    float getAlpha(); /**< Gets the alpha value of the motor. */
    float getBeta(); /**< Gets the beta value of the motor. */
    float getAlphaVelocity(); /**< Gets the velocity of the alpha value. */
    float getBetaVelocity(); /**< Gets the velocity of the beta value. */

    float getTargetForce(); /**< Gets the target force for the motor. */
    unsigned long getTargetForceLastUpdate(); /**< Gets the last update time. */
    float getMaxTemp(); /**< Gets the maximum temperature allowed for the FOC driver. */
    float getMaxCurrent(); /**< Gets the maximum current allowed for the FOC driver. */
    int getPolePairs(); /**< Gets the number of pole pairs of the motor. */
    float getResistance(); /**< Gets the resistance of the motor. */
    float getTorqueConstant(); /**< Gets the torque constant of the motor. */
    
    void updateAlpha(); /**< Updates the alpha value of the motor. */
    void updateBeta(); /**< Updates the beta value of the motor. */
    void updateTemp(); /**< Updates the temperature of the FOC driver. */
    void updateAlphaVelocity(); /**< Updates the velocity of the alpha value. */
    void updateBetaVelocity(); /**< Updates the velocity of the beta value. */
};

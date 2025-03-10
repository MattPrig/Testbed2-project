#include "FOCDriver.h"

/**
 * @file FOCDriver.cpp
 * @brief Implementation file for the FOCDriver class.
 */
FOCDriver::FOCDriver(){
}

/**
 * @brief Constructs a new FOCDriver object.
 * 
 * @param _CAN The CAN bus to use.
 * @param _ID The ID of the driver.
 */
FOCDriver::FOCDriver(ACAN_T4* _CAN, uint8_t _ID){
    CAN = _CAN;
    ID = _ID;
}

/**
 * @brief Function to call regularly in the loop to receive the messages that have been received and handle them.
 */
void FOCDriver::processMessage(CANMessage message){
        uint16_t control_id =    (message.id & 0b10000000000)>>10;
        uint16_t driver_id =     (message.id & 0b01111000000)>>6;
        uint16_t service_id =    (message.id & 0b00000111100)>>2;
        uint16_t sub_service_id = message.id & 0b00000000011;
        if (control_id == 1 && driver_id == ID){
            switch (service_id){
                case 0:
                    alpha.value = message.dataFloat[0];
                    alpha.lastUpdate = millis();
                    break;
                case 1:
                    alphaVelocity.value = message.dataFloat[0];
                    alphaVelocity.lastUpdate = millis();
                    break;
                default:
                    break;
            }
    }
}

// ------------------ setters -----------------------------------
bool FOCDriver::setTargetForce(float _targetForce){
    targetForce.value = _targetForce;
    targetForce.lastUpdate = millis();
    return sendMessage(2,0,targetForce.value);
}

// ------------------ getters -----------------------------------

/**
 * @brief Get the alpha value of the motor.
 * 
 * @return The alpha value of the motor.
 */
float FOCDriver::getAlpha(){
    updateAlpha();
    return alpha.value;
}

/**
 * @brief Get the alpha velocity value of the motor.
 * 
 * @return The alpha velocity value of the motor.
 */
float FOCDriver::getAlphaVelocity(){
    updateAlphaVelocity();
    return alphaVelocity.value;
}

// ------------------ updates -----------------------------------

/**
 * @brief Update the alpha value of the motor.
 */
void FOCDriver::updateAlpha(){
    sendMessage(0,0);
}

/**
 * @brief Update the alpha velocity value of the motor.
 */
void FOCDriver::updateAlphaVelocity(){
    sendMessage(1,0);
}

// ------------------ private methods ---------------------------

/**
 * @brief Send a CAN message with float data.
 * 
 * @param service_id The service ID of the message.
 * @param sub_service_id The sub-service ID of the message.
 * @param data The float data to send.
 * @return True if the message was sent successfully, false otherwise.
 */
bool FOCDriver::sendMessage(uint8_t service_id,uint8_t sub_service_id, float data){
    CANMessage message;
    message.id = 0b00000000000 | ID << 6 | service_id << 2 | sub_service_id;
    message.len = 4;
    
    message.dataFloat[0] = data;
    message.rtr = false;
    message.ext = false;
    
    return CAN->tryToSendReturnStatus(message);
}

/**
 * @brief Send a CAN message with uint8_t data.
 * 
 * @param service_id The service ID of the message.
 * @param sub_service_id The sub-service ID of the message.
 * @param data The uint8_t data to send.
 * @return True if the message was sent successfully, false otherwise.
 */
bool FOCDriver::sendMessage(uint8_t service_id,uint8_t sub_service_id, uint8_t data){
    CANMessage message;
    message.id = 0b00000000000 | ID << 6 | service_id << 2 | sub_service_id;
    message.len = 1;
    message.rtr = false;
    message.data[0] = data;
    message.ext = false;
    
    return CAN->tryToSendReturnStatus(message);
}

/**
 * @brief Send a CAN message without data.
 * 
 * @param service_id The service ID of the message.
 * @param sub_service_id The sub-service ID of the message.
 * @return True if the message was sent successfully, false otherwise.
 */
bool FOCDriver::sendMessage(uint8_t service_id,uint8_t sub_service_id){
    CANMessage message;
    message.id = 0b00000000000 | ID << 6 | service_id << 2 | sub_service_id;
    message.len = 0;
    message.rtr = false;
    message.ext = false;
    
    return CAN->tryToSendReturnStatus(message);
}

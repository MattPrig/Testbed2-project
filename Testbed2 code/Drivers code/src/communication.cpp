#include "communication.h"

// -------------------Controller class Init--------------------------

/**
 * @file Controller.cpp
 * @brief Implementation file for the Controller class.
 */
Controller::Controller(){
}

/**
 * @brief Constructs a new Controller object.
 * 
 * @param _CAN The CAN bus to use.
 * @param _ID The ID of the driver.
 */
Controller::Controller(STM32_CAN* _CAN, uint8_t _ID, Debug* _debug){
    CAN = _CAN;
    ID = _ID;
    debug = _debug;
    if(debug == nullptr){
        debug = new Debug(&Serial, DEBUG_NORMAL);
    }
}

Controller::Controller(STM32_CAN* _CAN, uint8_t _ID, HardwareSerial* _serial){
    CAN = _CAN;
    ID = _ID;
    debug = new Debug(_serial, DEBUG_NORMAL);
}

void Controller::addMotor(BLDCMotor* motor){
    this->motor = motor;
}

// -------------------Read messages--------------------------

/**
 * @brief Function to call regularly in the loop to receive the messages that have been received and handle them.
 * We should probably rename this read instead of run lol
 */
void Controller::run(){

    if (CAN->read(CAN_RX_msg) ) {

        uint16_t control_id =    (CAN_RX_msg.id & 0b10000000000)>>10;
        uint16_t driver_id =     (CAN_RX_msg.id & 0b01111000000)>>6;
        uint16_t service_id =    (CAN_RX_msg.id & 0b00000111100)>>2;
        uint16_t sub_service_id = CAN_RX_msg.id & 0b00000000011;

        if (control_id == 0 && driver_id == ID){
            switch (service_id){
                case 0:
                    sendMessage(0,0,(motor->shaft_angle));
                    break;
                case 1:
                    sendMessage(1,0,motor->shaft_velocity);
                    break;
                case 2:
                    targetForce.value = bytesToFloat(CAN_RX_msg.buf);
                    targetForce.lastUpdate = millis();
                    break;
                default:
                    break;
            }
        }
    }
}
// -------------------Getters--------------------------

float Controller::getTargetForce(){
    return targetForce.value;
}

unsigned long Controller::getTargetForceLastUpdate(){
    return targetForce.lastUpdate;
}

// ------------------ sendMessage ---------------------------
//here is 2 private methods that enable to send float or int datas throught CAN.

/**
 * @brief Send a CAN message with float data.
 * 
 * @param service_id The service ID of the message.
 * @param sub_service_id The sub-service ID of the message.
 * @param data The float data to send.
 */
bool Controller::sendMessage(uint8_t service_id,uint8_t sub_service_id, float data){
    CAN_message_t message;
    message.id = 0b10000000000 | ID << 6 | service_id << 2 | sub_service_id;
    message.len = 4;
    floatToBytes(message.buf, data);
    message.flags.remote = false;
    message.flags.extended = false;
    return CAN->write(message);
}

/**
 * @brief Send a CAN message with uint8_t data.
 * 
 * @param service_id The service ID of the message.
 * @param sub_service_id The sub-service ID of the message.
 * @param data The uint8_t data to send.
 */
bool Controller::sendMessage(uint8_t service_id,uint8_t sub_service_id, uint8_t data){
    CAN_message_t message;
    message.id = 0b10000000000 | ID << 6 | service_id << 2 | sub_service_id;
    message.len = 1;
    intToBytes((message.buf), data);
    message.flags.remote = false;
    message.flags.extended = false;
    
    return CAN->write(message);
}

//------------------ a few useful functions for data conversion ---------------------------

void Controller::floatToBytes(uint8_t (bytes)[4],float value){
    memcpy(bytes, (unsigned char*)(&value), 4);
}

void Controller::intToBytes(uint8_t (bytes[8]), int value){
    memcpy(bytes, &value, sizeof(value));
}

float Controller::bytesToFloat(uint8_t bytes[4]){
    float value;
    memcpy(&value, bytes, sizeof(value));
    return value;
}

int Controller::bytesToInt(uint8_t bytes[8]){
    int value;
    memcpy(&value, bytes, sizeof(value));
    return value;
}
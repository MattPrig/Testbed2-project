#include "communication.h"


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
Controller::Controller(Debug* _debug){
    //CAN = _CAN;
    //ID = _ID;
    debug = _debug;
    if(debug == nullptr){
        debug = new Debug(&Serial, DEBUG_NORMAL);
    }
}

Controller::Controller(HardwareSerial* _serial){
    //CAN = _CAN;
    //ID = _ID;
    debug = new Debug(_serial, DEBUG_NORMAL);
}


/**
 * @brief Initializes the Controller object.
 * 
 * @param resistance The resistance of the winding of the motor.
 * @param polePairs The number of pole pairs of the motor.
 * @param TorqueConstant The torque constant of the motor.
 */
void Controller::init(float resistance, int polePairs, float TorqueConstant){
    this->resistance = resistance;
    this->polePairs = polePairs;
    this->TorqueConstant = TorqueConstant;
}

/**
 * @brief Function to call regularly in the loop to receive the messages that have been received and handle them.
 */
void Controller::run(){

    /*if (CAN->read(CAN_RX_msg) ) {
        //readCanMessage();

        uint16_t control_id =    (CAN_RX_msg.id & 0b10000000000)>>10;
        uint16_t driver_id =     (CAN_RX_msg.id & 0b01111000000)>>6;
        uint16_t service_id =    (CAN_RX_msg.id & 0b00000111100)>>2;
        uint16_t sub_service_id = CAN_RX_msg.id & 0b00000000011;

        if (control_id == 0 && driver_id == ID){
            switch (service_id){
                case 0:
                    switch (sub_service_id){
                        case 0:
                            sendMessage(0,0,motor->shaft_angle);
                            break;
                        default:
                            break;
                    }
                    break;
                case 1:
                    switch (sub_service_id){
                        case 0:
                            getBeta();
                            sendMessage(1,0,beta.value,beta.velocity);
                            //Serial6.print("B");
                            //Serial6.println(beta.value);
                            break;
                        default:
                            break;
                    }
                case 2:
                    alphaVelocity.value = bytesToFloat(CAN_RX_msg.buf);
                    alphaVelocity.lastUpdate = millis();
                    //to do
                    break;
                case 3:
                    betaVelocity.value = bytesToFloat(CAN_RX_msg.buf);
                    betaVelocity.lastUpdate = millis();
                    //to do
                    break;
                case 4:
                    switch (sub_service_id){
                        case 1:
                            setTargetForce(bytesToFloat(CAN_RX_msg.buf));
                            sendMessage(0,0,getAlpha());
                            break;
                        case 2:
                            setTargetForce(bytesToFloat(CAN_RX_msg.buf));
                            getBeta();
                            sendMessage(1,0,beta.value,beta.velocity);
                            //Serial6.print("B");
                            //Serial6.println(beta.value);
                            break;
                        default:
                            break;
                    }
                    break;
                case 5:
                    temp.value = bytesToFloat(CAN_RX_msg.buf);
                    temp.lastUpdate = millis();
                    //to do
                    break;
                case 6:
                    maxTemp = bytesToFloat(CAN_RX_msg.buf);
                    //to do
                    break;
                case 7:
                    maxCurrent = bytesToFloat(CAN_RX_msg.buf);
                    motor->current_limit = maxCurrent;
                    break;
                case 8:
                    polePairs = bytesToInt(CAN_RX_msg.buf);
                    motor->pole_pairs = polePairs;
                    break;
                case 9:
                    resistance = bytesToFloat(CAN_RX_msg.buf);
                    motor->phase_resistance = resistance;
                    break;

                default:
                    break;
            }
        }
    }*/
}

void Controller::addMotor(BLDCMotor* motor){
    this->motor = motor;
}


void Controller::addSensorBeta(MagneticSensorSPI* sensorBeta){
    this->sensorBeta = sensorBeta;
}

LowPassFilter anglefilter = LowPassFilter(0.001); //lowpass filter for the angle measurement. 1.0 = 1s time constant
LowPassFilter velocityfilter = LowPassFilter(0.001); //lowpass filter for the velocity measurement. 1.0 = 1s time constant 
//at 0.01 I already get instabilities at high stiffnesses

// -------------------setters -----------------------------------

/**
 * @brief Set the alpha value of the motor, changes the offset of the sensor.
 * 
 * @param _alpha The alpha value to set.
 */
void Controller::setAlpha(float _alpha){
    offsetShaftAngleAlpha = -motor->shaft_angle + _alpha;
    alpha.lastUpdate = millis();
}

/**
 * @brief Set the target force of the motor, the driver will send beta as a reply.
 * 
 * @param _targetForce The target force to set.
 * @return True if the message was sent successfully, false otherwise.
 */
bool Controller::setTargetForce(float _targetForce){
    targetForce.value = _targetForce;
    targetForce.lastUpdate = millis();
    return(true);
}

/**
 * @brief Set the maximum temperature of the motor.
 * 
 * @param _maxTemp The maximum temperature to set.
 * @return True if the message was sent successfully, false otherwise.
 */
/**
 * @brief Set the maximum current of the motor.
 * 
 * @param _maxCurrent The maximum current to set.
 * @return True if the message was sent successfully, false otherwise.
 */
/**
 * @brief Set the parameters of the motor.
 * 
 * @param _polePairs The number of pole pairs of the motor.
 * @param _resistance The resistance of the winding of the motor.
 * @param _TorqueConstant The torque constant of the motor.
 * @return True if all messages were sent successfully, false otherwise.


// ------------------ getters -----------------------------------

/**
 * @brief Get the alpha value of the motor.
 * 
 * @return The alpha value of the motor.
 */
float Controller::getAlpha(){
    return motor->shaft_angle - offsetShaftAngleAlpha;
}

/**
 * @brief Get the beta value of the motor.
 * 
 * @return The beta value of the motor.
 */
float Controller::getBeta(){
    sensorBeta->update();
    beta.lastUpdate = millis();
    beta.value = sensorBeta->getAngle();
    beta.velocity = sensorBeta->getVelocity();
    beta.value = anglefilter(beta.value);
    beta.velocity = velocityfilter(beta.velocity);
    return beta.value;
}

/**
 * @brief Get the alpha velocity value of the motor.
 * 
 * @return The alpha velocity value of the motor.
 */
float Controller::getAlphaVelocity(){
    return alphaVelocity.value;
}

/**
 * @brief Get the beta velocity value of the motor.
 * 
 * @return The beta velocity value of the motor.
 */
float Controller::getBetaVelocity(){
    return betaVelocity.value;
}

/**
 * @brief Get the target force value of the motor.
 * 
 * @return The target force value of the motor.
 */
float Controller::getTargetForce(){
    return targetForce.value;
}

unsigned long Controller::getTargetForceLastUpdate(){
    return targetForce.lastUpdate;
}

/**
 * @brief Get the maximum temperature of the motor.
 * 
 * @return The maximum temperature of the motor.
 */
float Controller::getMaxTemp(){
    return maxTemp;
}

/**
 * @brief Get the maximum current of the motor.
 * 
 * @return The maximum current of the motor.
 */
float Controller::getMaxCurrent(){
    return maxCurrent;
}

/**
 * @brief Get the number of pole pairs of the motor.
 * 
 * @return The number of pole pairs of the motor.
 */
int Controller::getPolePairs(){
    return polePairs;
}




// ------------------ updates -----------------------------------


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
/*
This program controls the arm of the Variable Impedance Actuator prototype 2
Created: 2025-01-15
This code is what goes on the main Microcontroler (Teensy) that is connected to the motor drivers through a CAN bus. 

It makes use of the Scheduler library to run 3 different tasks every so often : the control loop callback and the PC uplink and downlink communication callback.

In the controlloopcallback function, you can make 3 different call to the drivers : getAlpha, getAlphavelocity and setTargetForceupdateAlpha. (I should add get currents as well to deduce torque)
The rest of the calculations are up to you.

The PC communication callback simply use Serial writes while taking care of the fact that CAN messages are read often enough (because Serial coms is slow)

You need to execute processCANmessage as often as possible so not to miss any messages, that's why it is called so often.

The FOCdriver class is handling the CAN bus. You can use setters to send a command to the controller (either zeroing the position or setting a target force) and getters to get the current position and velocity of the motor.
Updates are for updating the current stored value of the motor position and velocity.

*/

//=======================includes==================================
#include <Arduino.h>
#include "TaskScheduler.h"
#include <Encoder.h>
#include "debug.h"
#include <ACAN_T4.h>
#include "FOCDriver.h"
#include <vector>
#include <math.h>

//=======================Force parameters===============================
#define MAXIMAL_FORCE 9 // maximal force in A
#define MINIMAL_FORCE 0.4 // minimal force in A

//=======================constants=======================
//geometric constants
const float d = 0.026;     // diameter of the motor pulleys in meter
const std::vector<float> D = {0.114, 0.06, 0.042};    //diameters of the arm pulley in meters
const std::vector<float> L = {0.209,0.159,0.085};  //Length of the arm joints 
//=======================monitoring variables=======================
String Serbuffer = ""; //buffer for the serial communication
int receivercounter = 0; //counter for the serial communication 
//======================control loop variables=======================
//control loop parameters
float coactivation = 0;
float Base_force = 0;

//targeted torque
std::vector<double> target_torque = {0, 0 ,0};        //target torque of the arm in Nm
std::vector<double> target_motor_torque = {0, 0, 0, 0};     //target angles of the motors in radians
std::vector<double> target_force = {0, 0, 0, 0};       //target force of the motors in A


//=======================pins============================


//=======================Task scheduler============================
// Callback methods
void controlLoopCallback();
void PCuplinkCallback();
void PCdownlinkCallback();

// Tasks declaration (This is where you declare how often you want your tasks to be done, the timing is in milliseconds)
Task controlLoop(1, TASK_FOREVER, &controlLoopCallback);
Task PCuplink(500, TASK_FOREVER, &PCuplinkCallback);
Task PCdownlink(10, TASK_FOREVER, &PCdownlinkCallback);
Scheduler runner;

//=======================Declarations=======================

// Drivers
FOCDriver driver0(&ACAN_T4::can3, 0);
FOCDriver driver1(&ACAN_T4::can3, 1);
FOCDriver driver2(&ACAN_T4::can3, 2);
FOCDriver driver3(&ACAN_T4::can3, 3);
std::vector<FOCDriver> drivers = {driver0, driver1, driver2, driver3};

//functions
bool init_CAN(int baudrate = 1000*1000, bool print_params = false);
void processCANMessages();
void printSafelyln (String message);

//========================Setup===================================
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting system");
  init_CAN();

  //start tensioning the cables, send this a lot of time to ensure it happens well
  for (int i = 0; i < 100; ++i) {
    driver0.setTargetForce(0.83 * MINIMAL_FORCE);
    driver1.setTargetForce(MINIMAL_FORCE);
    driver2.setTargetForce(-MINIMAL_FORCE);
    driver3.setTargetForce(-MINIMAL_FORCE);
    processCANMessages();
    delay(10);
  }
  delay(1000);

  driver0.updateAlpha();
  processCANMessages();
  delay(10);
  driver1.updateAlpha();
  processCANMessages();
  delay(10);
  driver2.updateAlpha();
  processCANMessages();
  delay(10);
  driver3.updateAlpha();
  processCANMessages();
  delay(10);
  
  //Task scheduler startup
  runner.init();
  runner.addTask(controlLoop);
  runner.addTask(PCuplink);
  runner.addTask(PCdownlink);
  delay(10);

  // enabling the tasks
  controlLoop.enable();
  PCuplink.enable();
  PCdownlink.enable();
  delay(10);
  processCANMessages();
  
  Serial.println("System ready");
}

//=======================loop===================================
void loop()
{
  runner.execute();
  processCANMessages();
}

//=======================Functions=======================
//init CAN coms
bool init_CAN(int baudrate, bool print_params)
{
  /**
   * @brief initialise the can 3 bus, checks for erros and prints the settings 
   * 
   * @param baudrate the baudrate of the can bus, default is 1Mbit/s
   * @param print_params if true, prints the settings of the can bus, default is true
   * @return true if the can bus was initialised successfully
   */

  ACAN_T4_Settings settings (1000*1000) ;
  settings.mTxPinIsOpenCollector = false ;
  settings.mRxPinConfiguration = ACAN_T4_Settings::PULLUP_22k ;
  settings.mLoopBackMode = false ;
  settings.mSelfReceptionMode = false ;
  
  uint32_t errorCode = ACAN_T4::can3.begin (settings) ;
  if (0 == errorCode) {
    Serial.println("can3 online") ;
    return true;
  }else{
    Serial.println("Error can3: 0x") ;
  }
  delay (20) ;
  return !errorCode;
}

//handle CAN messages
void processCANMessages()
{
  CANMessage message;
  while (ACAN_T4::can3.receive(message))
  {
    uint16_t driver_id = (message.id & 0b01111000000)>>6;
    if (driver_id == 0)
    {
      driver0.processMessage(message);
    }
    if (driver_id == 1)
    {
      driver1.processMessage(message);
    }
    if (driver_id == 2)
    {
      driver2.processMessage(message);
    }
    if (driver_id == 3)
    {
      driver3.processMessage(message);
    }
  }
}

//Print to the serial while checking CAN coms to not slow them down
void printSafelyln (String message){
  Serial.println(message);
  runner.execute();
  processCANMessages();
}

std::vector<double> jointtomotor(std::vector<double> jointtorques)
{
  /**
   * @brief converts the angle of the arm to the angles of the motors
   * 
   * @param jointtorques the target torque of the joints in Nm
   * @return std::vector<float> the torque of the motors in Nm
   */
  std::vector<double> alphat = {0, 0, 0, 0};
  alphat[0] = d* (jointtorques[0]/D[0] + jointtorques[1]/D[1] + jointtorques[2]/D[2]);
  alphat[1] = d* (jointtorques[0]/D[0] - jointtorques[1]/D[1] - jointtorques[2]/D[2]);
  alphat[2] = d* (-jointtorques[0]/D[0] + jointtorques[1]/D[1]);
  alphat[3] = d* (-jointtorques[0]/D[0] - jointtorques[1]/D[1]);
  return alphat;
}

//=======================Callbacks=======================
//The callbacks are the functions called by the task scheduler at the specified interval

void controlLoopCallback()
{
  target_motor_torque = jointtomotor(target_torque);
  Base_force = abs(min(target_motor_torque[0], min(target_motor_torque[1], min(target_motor_torque[2], target_motor_torque[3])))-MINIMAL_FORCE) + coactivation;
  for (int i = 0; i < 4; ++i) {
    target_force[i] = target_motor_torque[i] + Base_force;
  	if (target_force[i] > MAXIMAL_FORCE)  //you don't want to put to much tension on it
  	  	{
  	    target_force[i] = MAXIMAL_FORCE;
  	 	 }
  }



  //The two first forces are rotating in the positive direction, the two last in the negative direction
  // (meaning applying a positive torque mean tensioning the cables for the first two motors and releasing them for the last two)
  driver0.setTargetForce(0.6*target_force[0]); //0.6 is a correcting coefficient from the fact that this motor is overperforming for no apparent reason
  processCANMessages();
  driver1.setTargetForce(target_force[1]);
  processCANMessages();
  driver2.setTargetForce(-target_force[2]);
  processCANMessages();
  driver3.setTargetForce(-0.9*target_force[3]);
  processCANMessages();
}

//write the Serial, uncomment what you wish to read, or write new lines for new datas to send back
void PCuplinkCallback(){
  printSafelyln("");
  printSafelyln("target torque 0: " + String(target_torque[0]));
  printSafelyln("target torque 1: " + String(target_torque[1]));
  printSafelyln("target torque 2: " + String(target_torque[2]));
  printSafelyln("target force 0: " + String(target_force[0]));
  printSafelyln("target force 1: " + String(target_force[1]));
  printSafelyln("target force 2: " + String(target_force[2]));
  printSafelyln("target force 3: " + String(target_force[3]));
}


void PCdownlinkCallback() {
  if (Serial.available() >= 4) { // 3 floats * 4 bytes each = 12 bytes
    Serbuffer = Serial.readStringUntil('\n');
    processCANMessages();
    float receivedfloat = Serbuffer.toFloat();
    printSafelyln("received float: " + String(receivedfloat));
    if (receivercounter == 0){
      target_torque[0] = receivedfloat;
      receivercounter++;
      }
    else if (receivercounter == 1){
        target_torque[1] = receivedfloat;
        receivercounter++;
      }
    else if (receivercounter == 2){
        target_torque[2] = receivedfloat;
        receivercounter++;
      }
    else if (receivercounter == 3){
        coactivation = receivedfloat;
        receivercounter = 0;
      }
    }
  }
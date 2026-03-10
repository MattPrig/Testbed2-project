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
#define MINIMAL_FORCE 0.6 // minimal force in A

//=======================constants=======================
//geometric constants
const float d = 0.014;     // diameter of the motor pulleys in meter
const std::vector<float> D = {0.06, 0.06, 0.03};    //diameters of the arm pulley in meters
const std::vector<float> L = {0.209,0.159,0.085};  //Length of the arm joints
//=======================monitoring variables=======================
long loopIterationSinceLastCommunication = 0; //number of loop iterations since the last communication with the PC, to check running speed
String Serbuffer = ""; //buffer for the serial communication
int receivercounter = 0; //counter for the serial communication 
//======================control loop variables=======================
//control loop parameters
float Base_force = 0;   //base force to be applied to tension the cables (coactivation)
std::vector<float> Kps = {9,6,0.1}; //target stiffness of the arm joints ; put 0 0 0 to have the arm able to be moved freely ; put 4.5 4 4 to have a smooth arm
std::vector<float> Kds = {1.6,1.2,0.8}; //target damping of the arm joints

//initial position of the motors to offset
std::vector<float> alpha_offset = {0, 0, 0, 0}; //offsets of the motors in radians

//actual positions
std::vector<float> alpha = {0, 0, 0, 0}; //angles of the motors in radians
std::vector<float> alpha_velocity = {0, 0, 0, 0}; //angular velocities of the motors in radians per second
std::vector<float> theta = {0, 0, 0}; //angles of the arm in radians
std::vector<float> theta_velocity = {0, 0, 0}; //angular velocities of the arm in radians per second

//targeted positions
std::vector<float> target_theta = {0, 0 ,0};        //target angle of the arm joint in radians

//calculations variables
std::vector<float> error_theta = {0, 0, 0};       //errors between target and actual angles of the arm in radians
std::vector<float> theta_force = {0, 0, 0};       //forces to apply to the arm joints in N
std::vector<float> alpha_force = {0, 0, 0, 0}; //forces to add to the motors due to distance from desired position and speed, in Amps
std::vector<float> target_force = {0, 0, 0, 0}; //forces to apply to the motors in Amps, sum of add

//=======================pins============================


//=======================Task scheduler============================
// Callback methods
void controlLoopCallback();
void PCuplinkCallback();
void PCdownlinkCallback();

// Tasks declaration (This is where you declare how often you want your tasks to be done, the timing is in milliseconds)
Task controlLoop(1, TASK_FOREVER, &controlLoopCallback);  //1000hz
Task PCuplink(500, TASK_FOREVER, &PCuplinkCallback);      //2hz
Task PCdownlink(20, TASK_FOREVER, &PCdownlinkCallback);   //50hz
Scheduler runner;

//=======================Declarations=======================

// Motor drivers
FOCDriver driver0(&ACAN_T4::can3, 0);
FOCDriver driver1(&ACAN_T4::can3, 1);
FOCDriver driver2(&ACAN_T4::can3, 2);
FOCDriver driver3(&ACAN_T4::can3, 3);
std::vector<FOCDriver> drivers = {driver0, driver1, driver2, driver3};

//Functions
bool init_CAN(int baudrate = 1000*1000);
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
    driver0.setTargetForce(0.6 * MINIMAL_FORCE);
    driver1.setTargetForce(MINIMAL_FORCE);
    driver2.setTargetForce(-MINIMAL_FORCE);
    driver3.setTargetForce(-0.9*MINIMAL_FORCE);
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

  //set the 0 position of the motors
  alpha_offset[0] = driver0.getAlpha();
  processCANMessages();
  delay(10);
  alpha_offset[1] = driver1.getAlpha();
  processCANMessages();
  delay(10);
  alpha_offset[2] = driver2.getAlpha();
  processCANMessages();
  delay(10);
  alpha_offset[3] = driver3.getAlpha();
  processCANMessages();
  delay(10);
  
  Serial.println("System ready");
}

//=======================loop===================================
void loop()
{
  runner.execute();
  processCANMessages();
  loopIterationSinceLastCommunication++;
}

//=======================Functions=======================

bool init_CAN(int baudrate)
{
  /**
   * @brief initialise the can 3 bus, checks for errors
   * @param baudrate the baudrate of the can bus, default is 1Mbit/s
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

void processCANMessages()
{
  //handle incoming CAN messages
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

void printSafelyln (String message){
  //Print to the serial while checking CAN coms to not slow them down
  Serial.println(message);
  runner.execute();
  processCANMessages();
}

std::vector<float> alphatotheta(std::vector<float> alpha)
{
  /**
   * @brief converts the angles of the motor to the angles of the arm
   * 
   * @param alpha the angles of the motors in radians
   * @return the angles of the arm in radians
   */
  std::vector<float> thetat = {0, 0, 0, 0};
  thetat[0] = d/D[0]*(alpha[0] + alpha[1] - alpha[2] - alpha[3])/4;
  thetat[1] = d/D[1]*(alpha[2] - alpha[3])/2;
  thetat[2] = d/D[2]*(alpha[0] - alpha[1])/2-2*thetat[1];
  return thetat;
}

std::vector<float> jointtomotor(std::vector<float> jointtorques)
{
  /**
   * @brief converts the angle of the arm to the angles of the motors
   * 
   * @param jointtorques the target torque of the joints in Nm
   * @return std::vector<float> the torque of the motors in Nm
   */
  std::vector<float> alphat = {0, 0, 0, 0};
  alphat[0] = d* (jointtorques[0]/D[0]+ jointtorques[2]/D[2]);
  alphat[1] = d* (jointtorques[0]/D[0] - jointtorques[2]/D[2]);
  alphat[2] = d* (-jointtorques[0]/D[0] + jointtorques[1]/D[1]);
  alphat[3] = d* (-jointtorques[0]/D[0] - jointtorques[1]/D[1]);
  return alphat;
}

//=======================Callbacks=======================
//The callbacks are the functions called by the task scheduler at the specified interval

void controlLoopCallback()
{
  alpha[0] = -driver0.getAlpha() + alpha_offset[0];
  processCANMessages();
  alpha[1] = -driver1.getAlpha() + alpha_offset[1];
  processCANMessages();
  alpha[2] = driver2.getAlpha() - alpha_offset[2];
  processCANMessages();
  alpha[3] = driver3.getAlpha() - alpha_offset[3];
  processCANMessages();
  alpha_velocity[0] = -driver0.getAlphaVelocity();
  processCANMessages();
  alpha_velocity[1] = -driver1.getAlphaVelocity();
  processCANMessages();
  alpha_velocity[2] = driver2.getAlphaVelocity();
  processCANMessages();
  alpha_velocity[3] = driver3.getAlphaVelocity();
  processCANMessages();
  theta = alphatotheta(alpha);
  theta_velocity = alphatotheta(alpha_velocity);
  for (int i = 0; i < 3 ; ++i) {
    error_theta[i] = theta[i] - target_theta[i];
    theta_force[i] = Kps[i] * error_theta[i] + Kds[i] * theta_velocity[i];
  }
  
  alpha_force = jointtomotor(theta_force);
  Base_force = abs(min(alpha_force[0], min(alpha_force[1], min(alpha_force[2], alpha_force[3])))-MINIMAL_FORCE);
  for (int i = 0; i < 4; ++i) {
    target_force[i] = alpha_force[i] + Base_force;
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

//write the Serial, uncomment what you wish to read
void PCuplinkCallback(){
  printSafelyln("");
  printSafelyln("target force 0: " + String(target_force[0]) + " A" );
  printSafelyln("target force 1: " + String(target_force[1]) + " A" );
  printSafelyln("target force 2: " + String(target_force[2]) + " A" );
  printSafelyln("target force 3: " + String(target_force[3]) + " A" );
  printSafelyln("alpha 0: " + String(1000*alpha[0]) + " millirads" );
  printSafelyln("alpha 1: " + String(1000*alpha[1]) + " millirads" );
  printSafelyln("alpha 2: " + String(1000*alpha[2]) + " millirads" );
  printSafelyln("alpha 3: " + String(1000*alpha[3]) + " millirads" );
  //printSafelyln("error theta 0: " + String(1000*error_theta[0]) + " millirads" );
  //printSafelyln("error theta 1: " + String(1000*error_theta[1]) + " millirads" );
  //printSafelyln("error theta 2: " + String(1000*error_theta[2]) + " millirads" );
  printSafelyln("theta 0: " + String(1000*theta[0]) + " millirads" );
  printSafelyln("theta 1: " + String(1000*theta[1]) + " millirads" );
  printSafelyln("theta 2: " + String(1000*theta[2]) + " millirads" );
  printSafelyln("target theta 0: " + String(1000*target_theta[0]) + " millirads" );
  printSafelyln("target theta 1: " + String(1000*target_theta[1]) + " millirads" );
  printSafelyln("target theta 2: " + String(1000*target_theta[2]) + " millirads" );
  //printSafelyln(">loopIterationSinceLastCommunication: " + String(loopIterationSinceLastCommunication));
  loopIterationSinceLastCommunication = 0;
}


void PCdownlinkCallback() {
  if (Serial.available() >= 4) { // 3 floats * 4 bytes each = 12 bytes
    Serbuffer = Serial.readStringUntil('\n');
    processCANMessages();
    float receivedfloat = Serbuffer.toFloat();
    printSafelyln("received float: " + String(receivedfloat));
    if (receivercounter == 0){
      target_theta[0] = receivedfloat;
      receivercounter++;
    }
    else if (receivercounter == 1){
      target_theta[1] = receivedfloat;
      receivercounter++;
    }
    else if (receivercounter == 2){
      target_theta[2] = receivedfloat;
      receivercounter = 0;
    }
  }
}
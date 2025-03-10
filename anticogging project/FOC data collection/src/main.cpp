#include <Arduino.h>
#include <board_vesc_6.h>
#include <SimpleFOC.h>
//#include "STM32_CAN.h"
#include "communication.h"
#include "debug.h"


const int motorPolePairs = 14;


HardwareSerial Serial6(USART6);
Debug debug(&Serial6, DEBUG_WARNING);

Controller controller(&debug);

BLDCMotor motor = BLDCMotor(motorPolePairs);
BLDCDriver6PWM driver(H1, L1, H2, L2, H3, L3, EN_GATE);
InlineCurrentSense current_sense = InlineCurrentSense(0.005f, 20.0f, CURRENT_1, CURRENT_2, CURRENT_3);

MagneticSensorSPI sensorAlpha = MagneticSensorSPI(AS5048_SPI, SPI1_NSS);
MagneticSensorSPI sensorBeta = MagneticSensorSPI(AS5048_SPI, SPI1_NSS_2);

float target_current = 0.0;
unsigned long lastCommand = 0;

// instantiate the commander
Commander command = Commander(debug);
void doTarget(char* cmd) {
  command.scalar(&target_current, cmd);
  lastCommand = millis();
}





void setup() {


  controller.addMotor(&motor);
  //controller.addSensorBeta(&sensorBeta);

  motor.phase_resistance = 0.195; // [Ohm]
  //motor.voltage_limit = 0.5; // [V] - if phase
  motor.current_limit = 6;      // [Amps] - if phase resistance defined
  motor.velocity_limit = 10; // [rad/s] 5 rad/s cca 50rpm
  driver.pwm_frequency = 20000; //default is 25000Hz

  delay(1000);
  Serial6.begin(115200);
  delay(1000);
  Serial6.println("Motor ready!");
  motor.useMonitoring(debug);
  motor.monitor_downsample = 3000;
  //motor.monitor_variables = _MON_ANGLE | _MON_VEL | _MON_CURR_Q | _MON_CURR_D| _MON_TARGET;
  motor.monitor_port = &Serial6;
  // initialise magnetic sensors hardware
  sensorAlpha.init();

  // link the motor to the sensorAlpha
  motor.linkSensor(&sensorAlpha);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 24;
  driver.init();
  // link driver
  motor.linkDriver(&driver);
  // link current sense and the driver
  current_sense.linkDriver(&driver);

  // current sense init hardware
  current_sense.init();
  // link the current sense to the motor
  motor.linkCurrentSense(&current_sense);

  // aligning voltage 
  motor.voltage_sensor_align = 0.9;
  // set motion control loop to be used 
  motor.controller = MotionControlType::angle;
  //motor.torque_controller = TorqueControlType::foc_current;
  motor.PID_current_q.P = 0.01;
  motor.PID_current_q.I = 0;
  motor.PID_current_d.P = 0.01; 
  motor.PID_current_d.I = 0;
  motor.LPF_current_q.Tf = 0.001f; // 1ms default
  motor.LPF_current_d.Tf = 0.001f; // 1ms default
  motor.LPF_velocity.Tf = 0.02f; 
  motor.LPF_angle.Tf = 0.001f;

  // angle PID controller 
  // default P=20
  motor.P_angle.P = 40; 
  motor.P_angle.I = 0;  // usually only P controller is enough 
  motor.P_angle.D = 0;  // usually only P controller is enough 
  // acceleration control using output ramp
  // this variable is in rad/s^2 and sets the limit of acceleration
  motor.P_angle.output_ramp = 10000; // default 1e6 rad/s^2

  // angle low pass filtering
  // default 0 - disabled  
  // use only for very noisy position sensors - try to avoid and keep the values very small
  motor.LPF_angle.Tf = 0; // default 0

  // setting the limits
  //  maximal velocity of the position control
  motor.velocity_limit = 20; // rad/s - default 20

  // default voltage_power_supply
  motor.voltage_limit = 3;

  // initialize motor
  motor.init();
  // align sensorAlpha and start FOC
  motor.initFOC();

  // add target command T
  command.add('T', doTarget, "target voltage");


}

// angle set point variable
  float target_angle = 0;
  long timestamp_us = _micros();
  long comtimestamp_us = _micros();
  float real_angle = 0;


void loop() {
  // Serial6.println("Looping");
  // main FOC algorithm function
  // the faster you run this function the better
  // Arduino UNO loop  ~1kHz
  // Bluepill loop ~10kHz 
  // each one second
  //if(_micros() - timestamp_us > 1e6) {
  //    timestamp_us = _micros();
  //    // inverse angle
  //    target_angle = -target_angle;   
  //}
  motor.loopFOC();
  motor.move(target_angle);

  if(_micros() - timestamp_us > 5e6) {
    timestamp_us = _micros();
    // inverse angle
    target_angle = target_angle + 0.001;   
  }

  if(_micros() - comtimestamp_us > 5e4) {
    comtimestamp_us = _micros();
    Serial6.print("ta:"); //target angle
    Serial6.print(1000*target_angle);
    Serial6.println(" ");
    real_angle = motor.shaft_angle;
    DQCurrent_s current = current_sense.getFOCCurrents(real_angle);
    Serial6.print("Qcurrent:"); //Q current
    Serial6.print(1000*current.q);
    Serial6.println(" ");
  }

  

  //target_current = controller.getTargetForce();\
  target_current = constrain(target_current, -motor.current_limit, motor.current_limit);
  //lastCommand = controller.getTargetForceLastUpdate();
  //target_current = controller.getTargetForce();
  //if (millis() - lastCommand > 500) {
  //  target_current = 0;
  //}
  // Motion control function
  // velocity, position or voltage (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code
  //motor.move(target_current);

  // user communication
  //command.run();
  //motor.monitor();
  //controller.run();
  //controller.setAlpha(motor.shaft_angle);
  // We only read data from CAN bus if there is frames received, so that main code can do its thing efficiently.

}



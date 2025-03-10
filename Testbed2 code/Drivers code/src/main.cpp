#include <Arduino.h>
#include <board_vesc_6.h>
#include <SimpleFOC.h>
#include "STM32_CAN.h"
#include "communication.h"
#include "debug.h"



STM32_CAN Can( CAN1, ALT );  //Use PB8/9 pins for CAN1.

const int motorPolePairs = 14;

HardwareSerial Serial6(USART6);
Debug debug(&Serial6, DEBUG_WARNING);

//Be careful to change the can address depending on which controler you're uploading to.
Controller controller(&Can,2, &debug);
BLDCMotor motor = BLDCMotor(motorPolePairs);
BLDCDriver6PWM driver(H1, L1, H2, L2, H3, L3, EN_GATE);
InlineCurrentSense current_sense = InlineCurrentSense(0.005f, 20.0f, CURRENT_1, CURRENT_2, CURRENT_3);
MagneticSensorSPI sensorAlpha = MagneticSensorSPI(AS5048_SPI, SPI1_NSS);

float target_current = 0.0;
unsigned long lastCommand = 0;

// instantiate the commander
Commander command = Commander(debug);
void doTarget(char* cmd) {
  command.scalar(&target_current, cmd);
  lastCommand = millis();
}

void setup() {

  Can.begin();
  Can.setBaudRate(1000000);  //1000KBPS

  controller.addMotor(&motor);

  motor.phase_resistance = 0.195; // [Ohm]
  motor.current_limit = 6;      // [Amps] - if phase resistance defined
  motor.velocity_limit = 10; // [rad/s] 5 rad/s cca 50rpm
  driver.pwm_frequency = 20000; //default is 25000Hz

  delay(1000);
  Serial6.begin(115200);
  delay(1000);
  Serial6.println("Motor ready!");
  motor.useMonitoring(debug);
  motor.monitor_downsample = 3000;
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
  motor.controller = MotionControlType::torque;
  motor.torque_controller = TorqueControlType::foc_current;
  motor.PID_current_q.P = 0.01;
  motor.PID_current_q.I = 3;
  motor.PID_current_d.P = 0.01;
  motor.PID_current_d.I = 3;
  motor.LPF_current_q.Tf = 0.001f; // 1ms default
  motor.LPF_current_d.Tf = 0.001f; // 1ms default
  motor.LPF_velocity.Tf = 0.02f; 
  motor.LPF_angle.Tf = 0.001f; //These two "preimplemented" filters does not seem to work (I tried them at Tf very long), so we should use the LowPassFilter class instead.
  // But it seems to have a difference in the implementation, as the preimplemented filters may affect the Torque loop as well.


  motor.voltage_limit = 3;

  // initialize motor
  motor.init();
  delay(4000);
  // align sensorAlpha and start FOC
  motor.initFOC();

  // add target command T
  command.add('T', doTarget, "target voltage");
}

unsigned long lastCanMessage = 0;

void loop() {
  // main FOC algorithm function
  // the faster you run this function the better
  motor.loopFOC();
  lastCommand = controller.getTargetForceLastUpdate();
  target_current = constrain(controller.getTargetForce(), -motor.current_limit, motor.current_limit);
  if (millis() - lastCommand > 500) {
    target_current = 0;
  }
  motor.move(target_current);

  // user communication
  command.run();
  motor.monitor();
  controller.run();
}



# 🤖 Variable Mechanical Stiffness Robotics Arm - Testbed 2

![Testbed Overview](./Latex%20report/testbed2_1.jpg)

## 🌟 Overview
This repository documents the development, construction, and testing of **Testbed 2**, a variable mechanical stiffness robotic arm. Built by **Matthieu Prigent**, this prototype serves as a learning platform for the development of tendon-driven actuation and variable impedance control.

> [!NOTE]
> For a comprehensive technical analysis, please refer to the [Testbed 2 report.pdf](./Testbed%202%20report.pdf) located in the project root.

---

## 🛠️ System Architecture

### 🦴 Mechanical Design
The arm features a **tendon-driven system** with 3 joints controlled by 4 motors.
- **Cable Routing**: Designed for high symmetry to simplify control implementation.
- **Construction**: 3D printed PLA, lasercut Delrin, and Plexiglas.
- **Pulleys**: Sized to achieve a target force of ~16N at the end effector.
- **Interactive CAD**: The Onshape model can be accessed [here](https://cad.onshape.com/documents/c940a6fae7dfa620c9d885fc/w/fe90fad0bb4f54c2b765f4a7/e/dd32a71d364b7df9178d1a8b).

### ⚡ Electronics
A robust electronic backbone ensures high-speed control and communication:
- **MCU**: [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) (Master controller).
- **Motor Drivers**: FLIPSKY Mini FSESC6.7 PRO (modified with $0.05\Omega$ resistors).
- **Motors**: 5008 EEE 170kV MAD brushless motors.
- **Sensors**: AS5048A magnetic absolute encoders.
- **Bus**: CAN bus for master-slave communication.

### 💻 Software Stack
- **Framework**: PlatformIO on VSCode.
- **Real-time Logic**: Based on the `TaskScheduler` library for deterministic execution.
- **FOC Control**: Utilizes the [SimpleFOC](https://docs.simplefoc.com/) library for precise torque and position control.
- **PC Interface**: Python-based GUI for real-time monitoring and control.

---

## 🕹️ Control Strategies

### 1. Torque Control
Implemented with a **coactivation** principle to maintain cable tension even when zero net torque is applied to the joints.

### 2. Position Control
A PD control loop in the joint space ($\theta$ space), allowing for stiff or compliant behavior by tuning $K_p$ and $K_d$ values.

### 3. Cartesian Control
Inverse kinematics implemented for the first two joints, allowing the end effector to follow lines or reach specific points in a 2D plane.

---

## 🔬 Anticogging Project
A specialized effort was made to mitigate **cogging torque**—the parasitic magnetic friction in brushless motors.
- **Status**: Captured cogging patterns using Fourier analysis (identifying 14Hz and 112Hz harmonics).
- **Location**: See the `anticogging project` directory for data collection scripts and analysis.

---

## 📂 Repository Structure
```text
.
├── anticogging project   # Scripts and data for cogging mitigation
├── Latex report         # Source files and images for the technical report
├── Testbed 2 report.pdf  # Final compiled report
└── Testbed2 code         # Source code for Teensy and Motor Drivers
    ├── Drivers code      # SimpleFOC implementation for drivers
    ├── Positioncontrolmaster # Teensy code for position control
    └── Torquecontrolmaster   # Teensy code for torque control
```

---

## 🚀 Future Improvements for Testbed 3
- [ ] **Structural Rigidity**: Link the top and bottom of the arm; use aluminum for load-bearing parts.
- [ ] **Cable Management**: Switch to spiral drums and Dyneema cables to reduce wear.
- [ ] **Electronics**: Switch to ODrive S1 drivers for native anticogging and better reliability.
- [ ] **Automation**: Fully implement absolute encoder sensing for the anticogging startup.

---
**Author**: Matthieu Prigent  
**Contact**: [matthieu.prigent@student-cs.fr](mailto:matthieu.prigent@student-cs.fr)

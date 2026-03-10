import serial.tools.list_ports
import serial
import sys
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QTextEdit, QPushButton, QGridLayout, QLabel
from PyQt5.QtGui import QPalette, QColor
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt5.QtWidgets import QVBoxLayout
import matplotlib
matplotlib.use('QtAgg')  # Or another suitable backend like 'Agg' if you don't need GUI
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import numpy as np

# List available serial ports
def listports():
    ports = serial.tools.list_ports.comports()
    print("Available serial ports:")
    for port in ports:
        print(f"  {port.device}")

listports()

# Connect to a serial port
port_name0 = "/dev/ttyACM0"
port_name1 = "/dev/ttyACM1"
try:
    ser = serial.Serial(port_name0, baudrate=115200) 
except serial.SerialException:
    try:
        ser = serial.Serial(port_name1, baudrate=115200)
    except serial.SerialException:
        print(f"Error: Could not connect to {port_name0} or {port_name1}.")

# Read from the serial port
while True:
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').rstrip()
        try:
            #trouver dans le message "target_force1: 0.0" et extraire la valeur
            #if "theta:" in line:
            #    line = line.split(":")[1].strip()
            print(line)
        except ValueError:
            pass
        except IndexError:
            pass
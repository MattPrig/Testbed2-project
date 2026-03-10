import serial.tools.list_ports
import serial
import sys
import os
import time
import json
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
    pass
try:
    ser = serial.Serial(port_name1, baudrate=115200)
except serial.SerialException:
    print(f"Error: Could not connect to {port_name0} or {port_name1}.")


def save_data_to_json(data, count):
    # Ensure the "data" directory exists
    data_dir = os.path.join(os.path.dirname(__file__), 'data')
    os.makedirs(data_dir, exist_ok=True)
    # Save the data to a JSON file in the "data" directory
    name = os.path.join(data_dir, f'data{count}.json')
    with open(name, 'w') as json_file:
        json.dump(data, json_file)


class ReadThread(QThread):
    # Define a signal to communicate with the main thread (for printing to console)
    update_console_signal = pyqtSignal(str)
    update_plot_signal = pyqtSignal(float)

    def run(self):
            count = 0
            while os.path.exists(f'data/data{count}.json'):
                count += 1
            data = []
            while True:
                if ser.in_waiting > 0:
                    line = ser.readline().decode('utf-8').rstrip()
                    self.update_console_signal.emit(line)
                    data.append(line)
                if len(data) > 9000:
                    save_data_to_json(data,count)
                    data = []
                    count+=1

                #try:
                    #trouver dans le message "target_force1: 0.0" et extraire la valeur
                    #if "DC" in line:
                        #line = line.split(":")[1].strip()
                        #print(line)
                        #value = float(line)
                        #self.update_plot_signal.emit(value)
                #except ValueError:
                #    pass
                #except IndexError:
                #    pass
    


class MplCanvas(FigureCanvas):
    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig, self.ax = plt.subplots(figsize=(width, height), dpi=dpi)
        super(MplCanvas, self).__init__(fig)

class DarkGrayWindow(QWidget):
    def __init__(self):
        super().__init__()

        self.stiffness = 0
        self.position = 0

        # Set up the window
        self.setWindowTitle('Dark Gray Text Input with Console and Thread')
        self.setGeometry(100, 100, 1000, 800)  # Increased window height for the console

        # Set up the dark gray background color
        palette = QPalette()
        palette.setColor(QPalette.Window, QColor(50, 50, 50))  # Dark gray background
        self.setPalette(palette)

        # Create a layout
        layout = QVBoxLayout()

        # Add explanation text
        explanation_text = QLabel("Use the number keys (0-9) to change mode.\n"
                                  "Modes 1-9 are specific modes.\n"
                                  "Mode 0 is manual : \n"
                                  "W/S : Increase/Decrease stiffness\n"
                                  "A/D : Move left/right\n")
        explanation_text.setStyleSheet("color: white;")
        layout.addWidget(explanation_text)

        # Create a text console (QTextEdit) for output messages
        self.console = QTextEdit(self)
        self.console.setReadOnly(True)  # Make the console read-only
        self.console.setStyleSheet("""
            QTextEdit { 
                background-color: #222222;
                color: white;
                border: 1px solid #666666;
                padding: 5px;
            }
        """)



        # Add the console to the layout
        layout.addWidget(self.console)

        # Create a grid layout for the WASD buttons
        grid_layout = QGridLayout()

        # Add WASD buttons
        self.w_button = QPushButton('W', self)
        self.w_button.setStyleSheet("background-color: #555555; color: white; padding: 5px;")
        self.w_button.clicked.connect(lambda: self.send_key('W'))
        self.w_button.pressed.connect(lambda: self.change_button_color(self.w_button, 'green'))
        self.w_button.released.connect(lambda: self.change_button_color(self.w_button, '#555555'))
        grid_layout.addWidget(self.w_button, 0, 1)  # Top-center

        self.a_button = QPushButton('A', self)
        self.a_button.setStyleSheet("background-color: #555555; color: white; padding: 5px;")
        self.a_button.clicked.connect(lambda: self.send_key('A'))
        self.a_button.pressed.connect(lambda: self.change_button_color(self.a_button, 'green'))
        self.a_button.released.connect(lambda: self.change_button_color(self.a_button, '#555555'))
        grid_layout.addWidget(self.a_button, 1, 0)  # Middle-left

        self.s_button = QPushButton('S', self)
        self.s_button.setStyleSheet("background-color: #555555; color: white; padding: 5px;")
        self.s_button.clicked.connect(lambda: self.send_key('S'))
        self.s_button.pressed.connect(lambda: self.change_button_color(self.s_button, 'green'))
        self.s_button.released.connect(lambda: self.change_button_color(self.s_button, '#555555'))
        grid_layout.addWidget(self.s_button, 1, 1)  # Middle-center

        self.d_button = QPushButton('D', self)
        self.d_button.setStyleSheet("background-color: #555555; color: white; padding: 5px;")
        self.d_button.clicked.connect(lambda: self.send_key('D'))
        self.d_button.pressed.connect(lambda: self.change_button_color(self.d_button, 'green'))
        self.d_button.released.connect(lambda: self.change_button_color(self.d_button, '#555555'))
        grid_layout.addWidget(self.d_button, 1, 2)  # Middle-right

        # Add the grid layout to the main layout
        layout.addLayout(grid_layout)

        # Add the Matplotlib canvas
        self.canvas = MplCanvas(self, width=5, height=6, dpi=100)
        layout.addWidget(self.canvas)

        # Set the layout to the window
        self.setLayout(layout)

        # Initialize the read thread (but don't start it yet)
        self.worker_thread = ReadThread()
        self.start_background_task()
        self.worker_thread.update_console_signal.connect(self.update_console)
        self.worker_thread.update_plot_signal.connect(self.update_plot)

        # Data for plotting
        self.xdata = np.linspace(0, 10, 100)
        self.ydata = np.zeros(100)
        self.line, = self.canvas.ax.plot(self.xdata, self.ydata, 'r-')

        # Set up a timer to update the plot
        self.timer = QTimer()
        self.timer.setInterval(100)  # Update every 100 ms
        self.timer.timeout.connect(self.refresh_plot)
        self.timer.start()

    
    def keyPressEvent(self, event):
        key = event.key()
        #do it with a for loop
        for i in range(10):
            if key == getattr(Qt, f'Key_{i}'):
                self.console.append(f"> {i}")
                if ser.is_open:
                    ser.write(f'{i}\n'.encode('utf-8'))
                else:
                    self.console.append("Error: Serial port is not open.")
        for i in ['W', 'A', 'S', 'D']:
            if key == getattr(Qt, f'Key_{i}'):
                self.console.append(f"> {i}")
                if ser.is_open:
                    ser.write(f'{i}\n'.encode('utf-8'))
                else:
                    self.console.append("Error: Serial port is not open.")

    def send_key(self, key):
        """Send a key press to the serial port"""
        self.console.append(f"> {key}")
        if ser.is_open:
            ser.write(f'{key}\n'.encode('utf-8'))
        else:
            self.console.append("Error: Serial port is not open.")
        if key == 'W':
            self.stiffness += 1
        if key == 'A':
            self.position -= 1
        if key == 'S':
            self.stiffness -= 1
        if key == 'D':
            self.position += 1
        #tocomplete to take into account saturation

    def change_button_color(self, button, color):
        """Change the button's background color"""
        button.setStyleSheet(f"background-color: {color}; color: white; padding: 5px;")

    def update_console(self, message):
        """Update the console from the background thread"""
        self.console.append(message)

    def start_background_task(self):
        """Start the background task (run the thread)"""
        if not self.worker_thread.isRunning():
            self.console.append("Starting background task...")
            self.worker_thread.start()

    def update_plot(self, value):
        """Update the plot with new data"""
        self.ydata = np.roll(self.ydata, -1)
        self.ydata[-1] = value
        self.update_ylim()
    
    def update_ylim(self):
        """Update the y-axis limits based on the data"""
        ymin = np.min(self.ydata)
        ymax = np.max(self.ydata)
        self.canvas.ax.set_ylim(ymin, ymax)

    def refresh_plot(self):
        """Refresh the plot"""
        self.line.set_ydata(self.ydata)
        self.canvas.draw()


if __name__ == '__main__':
    app = QApplication(sys.argv)

    # Create and show the dark gray window with text input, console, and thread
    window = DarkGrayWindow()
    window.show()

    sys.exit(app.exec_())
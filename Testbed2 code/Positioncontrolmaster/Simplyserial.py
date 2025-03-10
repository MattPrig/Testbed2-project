import serial
import serial.tools.list_ports

# List available serial ports
def listports():
    ports = []
    for port in serial.tools.list_ports.comports():
        ports.append(port.device)
    return ports
    
    

# Connect to a serial port
def serconnect():
    port_name0 = "/dev/ttyACM0"
    port_name1 = "/dev/ttyACM1"
    A = True
    while A==True:
        try:
            ser = serial.Serial(port_name0, baudrate=115200)
            if ser.is_open:
                A = False
        except serial.SerialException:
            pass
        try:
            ser = serial.Serial(port_name1, baudrate=115200)
            if ser.is_open:
                A = False
        except serial.SerialException:
            pass
    return ser

# Send a message to the serial port
def sersendfloat(key, ser):
    if ser.is_open:
        ser.write(f'{key}\n'.encode())
    else:
        print("Error: Serial port is not open.")
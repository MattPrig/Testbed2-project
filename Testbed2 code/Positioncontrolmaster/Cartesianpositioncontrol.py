#region import libraries #
import numpy as np
import time
import matplotlib.pyplot as plt
from matplotlib.patches import Arc
from Simplyserial import serconnect,sersendfloat, listports

# endregion

# region init serial #

try:
    print(listports())
    ser = serconnect()
except:
    print("Error: Could not connect to serial port.")
    pass

# endregion

# region geometric parameters 
motor_pulley_d = 0.026
Arm_pulley_D = [0.114, 0.06, 0.042]
L = [0.209,0.159,0.085]  # Length of the arm joints

# theta to alpha matrix
inverse_kinematics_matrix = 1/motor_pulley_d * np.array([[ Arm_pulley_D[0], Arm_pulley_D[1],  Arm_pulley_D[2]],
                                                         [ Arm_pulley_D[0],  -Arm_pulley_D[1], -Arm_pulley_D[2]],
                                                         [-Arm_pulley_D[0],  Arm_pulley_D[1],  0              ],
                                                         [-Arm_pulley_D[0], -Arm_pulley_D[1],  0              ]])
# Function to calculate the angles of the motors based on the desired angles of the arm joints
def theta_to_alpha(theta):
    alpha = np.dot(inverse_kinematics_matrix, theta)
    return alpha

#position variables
xy = [0.367,0]
theta = [0,0,0]
alpha = [0,0,0,0]

# endregion

# region plot functions
# Function to calculate the positions of the arm parts
def calculate_positions(theta):
    x0, y0 = 0, 0
    x1 = L[0] * np.cos(theta[0])
    y1 = L[0] * np.sin(theta[0])
    x2 = x1 + L[1] * np.cos(theta[0] + theta[1])
    y2 = y1 + L[1] * np.sin(theta[0] + theta[1])
    x3 = x2 + L[2] * np.cos(theta[0] + theta[1] + theta[2])
    y3 = y2 + L[2] * np.sin(theta[0] + theta[1] + theta[2])
    return [(x0, y0), (x1, y1), (x2, y2), (x3, y3)]

# Function to plot the reach of the arm
def armreachplot(fig,ax):
    arc1 = Arc((0,0),2*0.204767,2*0.204767,angle=0,theta1=-82.413,theta2=82.413, color='r', linewidth=2)
    arc2 = Arc((0,0),2*0.368,2*0.368,angle=0,theta1=-37.23,theta2=37.23, color='r', linewidth=2)
    arc3 = Arc((0.166409,0.126448),2*0.159,2*0.159,angle=0,theta1=37.23,theta2=151.23, color='r', linewidth=2)
    arc4 = Arc((0.166409,-0.126448),2*0.159,2*0.159,angle=0,theta1=-151.23,theta2=-37.23, color='r', linewidth=2)
    ax.add_patch(arc1)
    ax.add_patch(arc2)
    ax.add_patch(arc3)
    ax.add_patch(arc4)
    #plot a line between (0.209,0) and (0.209+0.159,0)
    ax.plot([0.2047,0.2047+0.159],[0,0],'r')
    return fig,ax
# endregion

# Function to calculate inverse kinematics based on just 2 joints, third one will serve for sensing purposes
def inverse_kinematics(x, y):
    #kinematics: j'ai x = L[0]*cos(theta[0]) + L[1]*cos(theta[0]+theta[1])
    #            j'ai y = L[0]*sin(theta[0]) + L[1]*sin(theta[0]+theta[1])

    # Calculate distance d
    d = np.sqrt(x**2 + y**2)

    # Calculate angle psi
    psi = np.arctan2(y, x)

    # Use the law of cosines to find theta1
    cos_theta1 = (L[0]**2 + L[1]**2 - d**2) / (2 * L[0] * L[1])
    cos_theta1 = np.clip(cos_theta1, -1.0, 1.0)  # Ensure the value is within [-1, 1] for arccos
    theta1_elbow_up = np.pi - np.arccos(cos_theta1)
    theta1_elbow_down = np.arccos(cos_theta1)-np.pi

    # Find theta0 for both configurations
    theta0_elbow_up = psi - np.arctan2(L[1] * np.sin(theta1_elbow_up), L[0] + L[1] * np.cos(theta1_elbow_up))
    theta0_elbow_down = psi - np.arctan2(L[1] * np.sin(theta1_elbow_down), L[0] + L[1] * np.cos(theta1_elbow_down))

    # Choose solution based on the sign of
    if y >= 0:
        theta[0] = theta0_elbow_up
        theta[1] = theta1_elbow_up
    else:
        theta[0] = theta0_elbow_down
        theta[1] = theta1_elbow_down

    #check if thetas are within the limits
    if theta[0] < -37.23*np.pi/180:
        print("Theta 0 is out of reach")
        theta[0]=-37.23*np.pi/180
    elif theta[0] > 37.23*np.pi/180:
        print("Theta 0 is out of reach")
        theta[0]=37.23*np.pi/180
    if theta[1] < -114*np.pi/180:
        print("Theta 1 is out of reach")
        theta[1]=-114*np.pi/180
    elif theta[1] > 114*np.pi/180:
        print("Theta 1 is out of reach")
        theta[1]=114*np.pi/180
    return theta

#function to plot 10 waypoints between two position (previous and next)
def waypoints(previous, next):
    if np.sign(previous[1]) == np.sign(next[1]):
        distance = np.sqrt((next[0] - previous[0])**2 + (next[1] - previous[1])**2)
        num_points = int(distance / 0.01)
        x = np.linspace(previous[0], next[0], num_points)
        y = np.linspace(previous[1], next[1], num_points)
    else:
        distance1 = np.sqrt((0.3637 - previous[0])**2 + (0 - previous[1])**2)
        distance2 = np.sqrt((next[0] - 0.3637)**2 + (next[1] - 0)**2)
        num_points1 = int(distance1 / 0.01)
        num_points2 = int(distance2 / 0.01)
        x1 = np.linspace(previous[0], 0.3637, num_points1)
        y1 = np.linspace(previous[1], 0, num_points1)
        x2 = np.linspace(0.3637, next[0], num_points2)
        y2 = np.linspace(0, next[1], num_points2)
        x = np.concatenate((x1, x2))
        y = np.concatenate((y1, y2))
    ax.plot(x, y, 'bx')
    fig.canvas.draw_idle()
    return x, y

# region click and button events
def onclick(event):
    global xy
    x = event.xdata
    y = event.ydata
    theta = inverse_kinematics(x, y)
    positions = calculate_positions(theta)
    for i in range(2):
        lines[i].set_data([positions[i][0], positions[i+1][0]], [positions[i][1], positions[i+1][1]])
    xlist,ylist = waypoints(xy,[x,y])
    #here you will use xlist ylist to convert it to a list of theta angle and then send them over a certain duration 
    for i in range(len(xlist)):
        plt.plot(xlist[i], ylist[i], 'bx')
        theta = inverse_kinematics(xlist[i], ylist[i])
        try:
            sersendfloat(-theta[0],ser)
            sersendfloat(-theta[1],ser)
            sersendfloat(-theta[2],ser)
            print(f'{theta[0]},{theta[1]},{theta[2]}')
            time.sleep(0.2)
        except:
            #print(f'{theta[0]},{theta[1]},{theta[2]}')
            pass
    xy = [x,y]
    #for i in range(60):
    #    theta[2] = 1*np.sin(2 * np.pi * i / 20)
    #    try:
    #        sersendfloat(-theta[0],ser)
    #        sersendfloat(-2*theta[1],ser)
    #        sersendfloat(-theta[2],ser)
    #        time.sleep(0.05)
    #    except:
    #        pass


def onpress1(event):
    sersendfloat(50.0,ser)

def onpress2(event):
    global xy
    xy = [0.367,0]
    sersendfloat(51.0,ser)
# endregion

# region plot setup
fig, ax = plt.subplots()
ax.set_ylim(-0.4, 0.4)
ax.set_xlim(-0.2, 0.5)
ax.set_aspect('equal')

#add two buttons to the plot in the bottom right
#axbutton1 = plt.axes([0.7, 0.05, 0.1, 0.075])
#axbutton2 = plt.axes([0.81, 0.05, 0.1, 0.075])
#button1 = Button(axbutton1, 'compliant')
#button2 = Button(axbutton2, 'stiff')
#button1.on_clicked(onpress1)
#button2.on_clicked(onpress2)

# Initial plot
positions = calculate_positions(theta)
lines = []
for i in range(2):
    line, = ax.plot([positions[i][0], positions[i+1][0]], [positions[i][1], positions[i+1][1]], 'o-', lw=2)
    lines.append(line)

#add the arc to show the reach of the arm
fig, ax = armreachplot(fig,ax)

#Add an onclick event to the plot
cid = fig.canvas.mpl_connect('button_press_event', onclick)

plt.show()
# endregion
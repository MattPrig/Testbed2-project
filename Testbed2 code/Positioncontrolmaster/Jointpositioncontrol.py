#region import libraries #
from Simplyserial import serconnect,sersendfloat, listports
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider
# endregion


class torquecontrol():
    def __init__(self):
        try:
            print(listports())
            self.ser = serconnect()
        except:
            print("Error: Could not connect to serial port.")
        pass

        self.motor_pulley_d = 0.026
        self.Arm_pulley_D = [0.114, 0.06, 0.042]
        self.L = [0.209,0.159,0.085]  # Length of the arm joints
        self.thetas = [0,0,0]

    def interactive_torque_plot(self):
        self.add_sliders()

        self.theta2slider.on_changed(self.update_plot)
        self.theta1slider.on_changed(self.update_plot)
        self.theta0slider.on_changed(self.update_plot)
        plt.show()

    def add_sliders(self) -> None:
        self.fig, self.ax = plt.subplots()
        self.fig.subplots_adjust()
        # adjust torque of third joint
        self.ax_theta2 = self.fig.add_axes([0.35, 0.7, 0.35, 0.03])
        self.theta2slider = Slider(ax=self.ax_theta2, label='theta 2', valmin=-1,
                                   valmax=1, valinit=0, valfmt='%.2f', color='red')
        # adjust torque of second joint
        self.ax_theta1 = self.fig.add_axes([0.35, 0.55, 0.35, 0.03])
        self.theta1slider = Slider(ax=self.ax_theta1, label='theta 1', valmin=-0.25,
                                  valmax=0.25, valinit=0, valfmt='%.2f', color='red')
        # adjust torque of first joint
        self.ax_theta0 = self.fig.add_axes([0.35, 0.4, 0.35, 0.03])
        self.theta0slider = Slider(ax=self.ax_theta0, label='theta 0', valmin=-0.1,
                                  valmax=0.1, valinit=0, valfmt='%.2f', color='red')
        
    def update_plot(self, val) -> None:
        self.thetas[0] = self.theta0slider.val*10
        self.thetas[1] = self.theta1slider.val*10
        self.thetas[2] = self.theta2slider.val*10

        sersendfloat(self.thetas[0],self.ser)
        sersendfloat(self.thetas[1],self.ser)
        sersendfloat(self.thetas[2],self.ser)
    
        self.ax.autoscale_view()
        self.fig.canvas.draw_idle()


if __name__ == '__main__':
    quaid_waveform = torquecontrol()
    quaid_waveform.interactive_torque_plot()
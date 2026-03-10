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
        self.torques = [0,0,0]

    def interactive_torque_plot(self):
        self.add_sliders()

        self.torque2slider.on_changed(self.update_plot)
        self.torque1slider.on_changed(self.update_plot)
        self.torque0slider.on_changed(self.update_plot)
        self.coactslider.on_changed(self.update_plot)
        plt.show()

    def add_sliders(self) -> None:
        self.fig, self.ax = plt.subplots()
        self.fig.subplots_adjust()
        # adjust torque of third joint
        self.ax_torque2 = self.fig.add_axes([0.35, 0.7, 0.35, 0.03])
        self.torque2slider = Slider(ax=self.ax_torque2, label='torque 2', valmin=-0.05,
                                   valmax=0.05, valinit=0, valfmt='%.2f', color='red')
        # adjust torque of second joint
        self.ax_torque1 = self.fig.add_axes([0.35, 0.55, 0.35, 0.03])
        self.torque1slider = Slider(ax=self.ax_torque1, label='torque 1', valmin=-0.1,
                                  valmax=0.1, valinit=0, valfmt='%.2f', color='red')
        # adjust torque of first joint
        self.ax_torque0 = self.fig.add_axes([0.35, 0.4, 0.35, 0.03])
        self.torque0slider = Slider(ax=self.ax_torque0, label='torque 0', valmin=-0.15,
                                  valmax=0.15, valinit=0, valfmt='%.2f', color='red')
        #adjust coactivation
        self.ax_coact = self.fig.add_axes([0.35, 0.2, 0.35, 0.03])
        self.coactslider = Slider(ax=self.ax_coact, label='coactivation', valmin=0,
                                  valmax=3, valinit=0, valfmt='%.2f', color='red')
        
    def update_plot(self, val) -> None:
        self.torques[0] = self.torque0slider.val*10
        self.torques[1] = self.torque1slider.val*10
        self.torques[2] = self.torque2slider.val*10
        self.coactivation = self.coactslider.val

        sersendfloat(-self.torques[0],self.ser)
        sersendfloat(-self.torques[1],self.ser)
        sersendfloat(-self.torques[2],self.ser)
        sersendfloat(self.coactivation,self.ser)
    
        self.ax.autoscale_view()
        self.fig.canvas.draw_idle()


if __name__ == '__main__':
    quaid_waveform = torquecontrol()
    quaid_waveform.interactive_torque_plot()
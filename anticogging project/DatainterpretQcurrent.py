import json
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
from matplotlib.animation import FuncAnimation


#load the data from the json files ("ta" and "DC" values) and the derivative of the DC values
def load_data():
    all_data = []

    i = 0
    while True:
        file_path = f'./data/data{i}.json'
        if not os.path.exists(file_path):
            break
        with open(file_path, 'r') as file:
            data = json.load(file)
            all_data.extend(data)
        i += 1

    # Initialize lists to store the values
    ta_values = []
    dc_values = []

    # Extract the values from the data
    for item in all_data:
        if item.startswith('ta:'):
            ta_values.append(float(item.split(':')[1]))
        elif item.startswith('Qcurrent:'):
            dc_values.append(float(item.split(':')[1]))

            
    # Create a DataFrame
    print(len(ta_values))
    print(len(dc_values))
    df = pd.DataFrame({
        'ta': ta_values,
        'DC': dc_values
    })

    df = df.groupby('ta').mean()
    return df

df = load_data()

df['DC'] = df['DC'].rolling(window=7).mean()
df.index = df.index / 6284

#plot df and an approximation of the DC values with sinusoidal waves
plt.plot(df.index, df['DC'])
plt.axhline(y=0, color='r', linestyle='-')
#plot a combination of sinusoidal wave on the graph, with amplitude, frequency and phase as parameters
x = np.linspace(0, 14, 3141)
y = np.zeros(3141)
#amplitude = [69,23.44,21.3,20.5,19.2]
#frequency = [0.5,5.5,2.5,1.5,6.5]
#phase = [-2.91,0,1,1,0]
amplitude = [69,20.5,21.3,23.44,19.2]
frequency = [0.5,1.5,2.5,5.5,6.5]
phase = [-3,0,0,-1,-3]
#for i in range(4,5):
for i in range(len(amplitude)):
    y += amplitude[i] * np.sin(2 * np.pi * frequency[i] * x + phase[i])
plt.plot(x, y)
plt.xlabel('ta')
plt.ylabel('DC')
plt.show()

def plotanimation(df):
    # Initialize the plot
    fig, ax = plt.subplots()
    ax.plot(df.index, df['DC'])
    ax.axhline(y=0, color='r', linestyle='-')
    line, = ax.plot([], [], lw=2)
    ax.set_xlim(0, 14)
    ax.set_ylim(-200, 200)
    ax.set_xlabel('ta')
    ax.set_ylabel('DC')

    # Parameters for the sinusoidal waves
    x = np.linspace(0, 14, 3141)
    amplitude = [69, 23.44, 21.3]
    frequency = [0.5, 5.5, 2.5]
    phase = [3.14, 0, 0]  # Initial phase for the third wave

    def init():
        line.set_data([], [])
        return line,

    def update(frame):
        phase[2] = frame  # Update the phase of the third wave
        print(f"Current phase[2]: {phase[2]}")  # Print the current phase
        y = np.zeros(3141)
        for i in range(len(amplitude)):
            y += amplitude[i] * np.sin(2 * np.pi * frequency[i] * x + phase[i])
        line.set_data(x, y)
        return line,

    # Create the animation
    ani = FuncAnimation(fig, update, frames=np.linspace(0, 2 * np.pi, 128), init_func=init, blit=True)

    plt.show()

plotanimation(df)

#doing a fft on the datas and plotting it
def superfft(df):
    from scipy.fft import fft
    import numpy as np
    import matplotlib.pyplot as plt
    from scipy.signal import find_peaks

    # Change the x axis by dividing it by 6284
    dc_diff_values = df['DC'].dropna().to_numpy()
    # Perform FFT on the derivative of DC values
    N = len(dc_diff_values)
    T = (df.index[1] - df.index[0])  # Assuming uniform spacing in 'ta'
    yf = fft(dc_diff_values)
    xf = np.fft.fftfreq(N, T)[:N//2]

    # Filter the frequencies to be within 1000 Hz
    mask = xf <= 20
    xf_filtered = xf[mask]
    yf_filtered = 2.0/N * np.abs(yf[:N//2])[mask]

    # Find peaks
    peaks, _ = find_peaks(yf_filtered, height=15)

    # Calculate phase angles
    phases = np.angle(yf[:N//2])[mask]

    # Initialize lists for frequencies, phases, and amplitudes
    frequencies = []
    phases_list = []
    amplitudes = []

    # Append the values to the lists
    for peak in peaks:
        frequencies.append(xf_filtered[peak])
        phases_list.append(phases[peak])
        amplitudes.append(yf_filtered[peak])

    # Print the lists
    print("Frequencies:", frequencies)
    print("Phases:", phases_list)
    print("Amplitudes:", amplitudes)

    # Plot the FFT results
    plt.plot(xf_filtered, yf_filtered)
    # Plot the peaks
    plt.plot(xf_filtered[peaks], yf_filtered[peaks], "x")
    plt.xlabel('Frequency')
    plt.ylabel('Amplitude')
    plt.title('FFT of DC_diff')
    plt.grid()
    plt.show()

print(superfft(df))


df = df.round(2)
list_df = df['DC'].to_list()
target_length = 6284
current_length = len(list_df)
if current_length < target_length:
    list_df.extend([0] * (target_length - current_length))
#replace nan by 0
list_df = [0 if np.isnan(x) else x for x in list_df]
print(list_df)

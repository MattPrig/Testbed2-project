import json
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os


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
    # Compute the derivative of the DC values
    df['DC_diff'] = df['DC'].diff()
    #smooth the derivative
    df['DC_diff'] = df['DC_diff'].rolling(window=5).mean()
    #divide ta by 1000 then by 2pi to convert to per rotaiton
    df.index = df.index/1000/2/np.pi
    return df

df = load_data()

#plot all the datas
def derivativeplot(df):
    plt.plot(df.index, df['DC_diff'])
    plt.xlabel('ta')
    plt.ylabel('DC_diff')
    plt.axhline(y=0, color='r', linestyle='-')
    plt.show()
    print(df)
    df = df.round(2)
    list_df = df['DC_diff'].to_list()
    print(list_df)
    print(len(list_df))

derivativeplot(df)

#doing a fft on the derivative and plotting it
def superfft(df):
    from scipy.fft import fft
    dc_diff_values = df['DC_diff'].dropna().to_numpy()
    # Perform FFT on the derivative of DC values
    N = len(dc_diff_values)
    T = (df.index[1] - df.index[0])  # Assuming uniform spacing in 'ta'
    yf = fft(dc_diff_values)
    xf = np.fft.fftfreq(N, T)[:N//2]

    # Filter the frequencies to be within 100 Hz
    mask = xf <= 1000
    xf_filtered = xf[mask]
    yf_filtered = 2.0/N * np.abs(yf[:N//2])[mask]

    #find peaks
    from scipy.signal import find_peaks
    peaks, _ = find_peaks(yf_filtered, height=1)


    # Plot the FFT results
    plt.plot(xf_filtered, yf_filtered)
    #plot the peaks
    plt.plot(xf_filtered[peaks], yf_filtered[peaks], "x")
    plt.xlabel('Frequency')
    plt.ylabel('Amplitude')
    plt.title('FFT of DC_diff')
    plt.grid()
    plt.show()


#print(superfft(df))


#slice the data into several df of length 0.03571
for i in range(8):
    newdf = df.loc[i*0.03571:(i+1)*0.03571]
    #substract i*0.03571 to the index
    newdf.index = newdf.index - i*0.03571
    plt.plot(newdf.index, newdf['DC_diff'],label=f"slice {i}")
#using the fft function, plot an approximation of the functions
plt.legend(loc='upper left')
plt.xlabel('ta')
plt.ylabel('DC_diff')
plt.axhline(y=0, color='r', linestyle='-')


#now let's compute a set that is the mean of these slices
mean_df = pd.DataFrame(np.zeros(224),columns=['DC_diff'])
for i in range(8):
    newdf = df.loc[i*0.03571:(i+1)*0.03571]
    #reindex the newdf from 0 to len(newdf)
    newdf.index = np.arange(len(newdf))
    #if newdf is bigger than mean_df, cut it
    if len(newdf)>len(mean_df):
        newdf = newdf[:len(mean_df)]
    mean_df = mean_df + newdf 
mean_df = mean_df/8
#round the values to the second decimal
mean_df = mean_df.round(2)
#put the DC_diff in a list
list_df = mean_df['DC_diff'].to_list()
#print(list_df)
#print(len(list_df))
#reindex between 0 and 0.03571
mean_df.index = mean_df.index/224*0.03571
#plot the mean with a width of the line bigger than the slices
plt.plot(mean_df.index, mean_df['DC_diff'],label="mean",linewidth=7)
plt.xlabel('ta')
plt.ylabel('DC_diff')
plt.axhline(y=0, color='r', linestyle='-')
plt.show()


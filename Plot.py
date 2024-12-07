import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import correlate


# Leer archivo
freq, data = wavfile.read("pav_418.wav")

#Normalizar la señal
if data.dtype == np.int16:
    data = data / 32768.0

segmento = 30#30 ms
t0 = 0.600 # Tiempo inicial
tf = t0 + 0.03 # el tiempo inicial mas los 30 ms

inicial = int(t0 * freq) #samples iniciales
final = int(tf * freq) #samples finales

# Crear ventana
frame = data[inicial:final]
frame = frame - np.mean(frame)

window = len(frame)

# Autocorrelacion
autocorr = correlate(frame, frame, mode="full")
autocorr = autocorr[len(autocorr)//2:] #parte positiva


# pitch
pitch = np.argmax(autocorr[1:]) + 1
periodo = pitch / freq * 1e3

time = np.arange(inicial, final) / freq

# Plotting

fig, ax = plt.subplots(2, 1, figsize=(16, 10))

ax[0].plot(time, frame)
ax[0].set_title("señal temporal (30ms)")
ax[0].set_xlabel("tiempo (s)")
ax[0].set_xlim(t0, tf)

lags = np.arange(window) /freq
ax[1].plot(lags, autocorr)
ax[1].set_title("Autocorrelacion")
ax[1].set_xlabel("Lag (s)")
ax[1].set_xlim(0, (window -1) / freq)

plt.tight_layout()
plt.show()
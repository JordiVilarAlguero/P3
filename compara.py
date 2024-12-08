import matplotlib
matplotlib.use('Agg')  # Establece el backend sin GUI
import numpy as np
import matplotlib.pyplot as plt

wavesurfer_pitch = np.loadtxt("prueba.f0ref")  
mi_pitch = np.loadtxt("prueba.f0")  

# Verifica que ambos archivos tienen el mismo número de frames
if len(wavesurfer_pitch) != len(mi_pitch):
    raise ValueError("Los archivos no tienen el mismo número de frames. Verifica los datos.")

# Crear un eje temporal basado en el número de frames
num_frames = len(wavesurfer_pitch)
frame_duration = 0.015  
tiempo = np.linspace(0, num_frames * frame_duration, num_frames)

# Graficar los datos
plt.figure(figsize=(10, 6))
plt.plot(tiempo, wavesurfer_pitch, label="Wavesurfer", color="blue")
plt.plot(tiempo, mi_pitch, label="Mi estimador", color="orange", linestyle="dashed")
plt.xlabel("Tiempo (s)")
plt.ylabel("Frecuencia (Hz)")
plt.title("Comparación de estimadores de pitch")
plt.legend()
plt.grid()

# Guardar la imagen 
plt.savefig('comparacion_pitch.png', dpi=300)  

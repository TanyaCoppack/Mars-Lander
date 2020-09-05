
import numpy as np
import matplotlib.pyplot as plt
results = np.loadtxt(r"C:\Users\Owner\Desktop\Tanya\lander\Lander real\results.txt")

v_list = []
Kh = 0.03

if results[:, 0][0] > 190000:  #dont include ideal descent for 200km path
    plt.figure(1)
    plt.clf()
    plt.xlabel('altitude (s)')
    plt.grid()
    plt.plot(results[:, 0], results[:, 1], label ='velocity (m/s)')
    plt.legend()
    plt.show()
else:
    for h in results[:, 0]:
        v_list.append(-(0.5 + Kh*h))
    v_array = np.array(v_list)

    plt.figure(1)
    plt.clf()
    plt.xlabel('altitude (s)')
    plt.grid()
    plt.plot(results[:, 0], results[:, 1], label ='velocity (m/s)')
    plt.plot(results[:, 0], v_array, label ='ideal vel (m/s)')
    plt.legend()
    plt.show()

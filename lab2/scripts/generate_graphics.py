import matplotlib.pyplot as plt

# data calculate random matrix 1000 x 1000 (1 <= elements <= 10000), 100 repeated, window size 3x3

# physic cores 16 - 1 = 15

len = 9

n_threads = [1, 2, 3, 5, 10, 15, 30, 100, 1000]
time = [18.481, 9.280, 6.247, 3.840, 2.392, 2.284, 2.188, 2.203, 2.146]


plt.figure(figsize=(10, 6))
plt.plot(n_threads, time, 'bo-', linewidth=2, markersize=8)
plt.xlabel('Количество потоков (лог. шкала)')
plt.ylabel('Время выполнения программы (сек.)')
plt.title('Зависимость времени от количества потоков')
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.xticks([1, 2, 3, 5, 10, 15, 30, 100, 1000],
           ['1', '2', '3', '5', '10', '15', '30', '100', '1000'])
plt.show()

boost = []

for i in range(len):
    boost.append(time[0] / time[i])

plt.plot(n_threads, boost, 'bo-', linewidth=2, markersize=8)
plt.xlabel('Количество потоков (лог. шкала)')
plt.ylabel('Ускорение')
plt.title('Зависимость ускорения от количества потоков')
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.xticks([1, 2, 3, 5, 10, 15, 30, 100, 1000],
           ['1', '2', '3', '5', '10', '15', '30', '100', '1000'])

plt.show()

effectiveness = []

for i in range(len):
    effectiveness.append(boost[i] / n_threads[i])

plt.plot(n_threads, effectiveness, 'bo-', linewidth=2, markersize=8)
plt.xlabel('Количество потоков (лог. шкала)')
plt.ylabel('Эффективность')
plt.title('Зависимость эффективности от количества потоков')
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.xticks([1, 2, 3, 5, 10, 15, 30, 100, 1000],
           ['1', '2', '3', '5', '10', '15', '30', '100', '1000'])

plt.show()

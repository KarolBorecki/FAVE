import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Parametry symulacji
N = 50  # Rozmiar siatki (NxN)
dt = 0.01  # Krok czasowy
iterations = 100  # Liczba kroków symulacji
g = -9.81  # Przyspieszenie grawitacyjne
viscosity = 0.1  # Współczynnik lepkości
rho = 1.0  # Gęstość płynu

# Inicjalizacja pól prędkości i ciśnienia
u = np.zeros((N, N))  # Składowa prędkości w kierunku x
v = np.zeros((N, N))  # Składowa prędkości w kierunku y
p = np.zeros((N, N))  # Ciśnienie

# Pojemnik z markerami
markers = np.array([[N//2, N//2], [N//2 + 1, N//2]])  # Pozycje markerów (na starcie dwa punkty centralne)

# Funkcje pomocnicze
def divergence(u, v):
    """Obliczanie dywergencji pola prędkości."""
    return (np.roll(u, -1, axis=0) - u + np.roll(v, -1, axis=1) - v)

def pressure_solve(p, div, iters=20):
    """Rozwiązywanie równania Poissona dla ciśnienia."""
    for _ in range(iters):
        p = (np.roll(p, 1, axis=0) + np.roll(p, -1, axis=0) + 
             np.roll(p, 1, axis=1) + np.roll(p, -1, axis=1) - div) / 4.0
    return p

def advect(u, v, field):
    """Adwekcja pola przy użyciu półkrokowego algorytmu."""
    N = field.shape[0]
    advected = np.zeros_like(field)
    for i in range(N):
        for j in range(N):
            x = i - dt * u[i, j]
            y = j - dt * v[i, j]
            x = np.clip(x, 0, N-1)
            y = np.clip(y, 0, N-1)
            advected[i, j] = field[int(x), int(y)]
    return advected

# Główna pętla symulacji
def simulate():
    global u, v, p, markers

    for _ in range(iterations):
        # 1. Dodawanie siły grawitacji
        v += g * dt

        # 2. Obliczanie lepkości (dyfuzja prędkości)
        u = advect(u, v, u) + viscosity * (np.roll(u, 1, axis=0) + np.roll(u, -1, axis=0) +
                                           np.roll(u, 1, axis=1) + np.roll(u, -1, axis=1) - 4 * u) * dt
        v = advect(u, v, v) + viscosity * (np.roll(v, 1, axis=0) + np.roll(v, -1, axis=0) +
                                           np.roll(v, 1, axis=1) + np.roll(v, -1, axis=1) - 4 * v) * dt

        # 3. Wyznaczenie dywergencji prędkości
        div = divergence(u, v)

        # 4. Rozwiązanie równania Poissona dla ciśnienia
        p = pressure_solve(p, div)

        # 5. Korekta prędkości, aby zachować bezdywergencję
        u -= 0.5 * (np.roll(p, -1, axis=0) - np.roll(p, 1, axis=0))
        v -= 0.5 * (np.roll(p, -1, axis=1) - np.roll(p, 1, axis=1))

        # Aktualizacja pozycji markerów
        for m in range(len(markers)):
            x, y = markers[m]
            x_new = x + u[int(x), int(y)] * dt
            y_new = y + v[int(x), int(y)] * dt
            markers[m] = [np.clip(x_new, 0, N-1), np.clip(y_new, 0, N-1)]

        yield u, v, markers  # Zwraca wartości do animacji

# Wizualizacja
fig, ax = plt.subplots()
quiver = ax.quiver(u, v)
marker_plot, = ax.plot([], [], 'ro')  # Markery

def update(frame):
    u, v, markers = frame
    quiver.set_UVC(u, v)
    marker_plot.set_data(markers[:, 1], markers[:, 0])
    return quiver, marker_plot

ani = FuncAnimation(fig, update, frames=simulate, blit=True, interval=50)
plt.show()

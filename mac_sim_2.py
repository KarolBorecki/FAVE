import numpy as np
import matplotlib.pyplot as plt

# Parametry symulacji
NX, NY = 50, 50  # Rozmiary siatki
DX, DY = 1.0, 1.0  # Rozmiar komórki
DT = 0.1  # Krok czasowy
RHO = 1.0  # Gęstość płynu
VIS = 0.1  # Lepkość

# Inicjalizacja zmiennych
u = np.zeros((NX + 1, NY))  # Prędkość w kierunku x (na krawędziach pionowych)
v = np.zeros((NX, NY + 1))  # Prędkość w kierunku y (na krawędziach poziomych)
p = np.zeros((NX, NY))  # Ciśnienie (w środku komórek)

# Siły zewnętrzne (np. grawitacja)
fx = np.zeros((NX + 1, NY))
fy = np.zeros((NX, NY + 1))
fy[:, :] = -9.81  # Grawitacja w dół


# Adwekcja prędkości
def advect(u, v, field, dt, dx, dy):
    nx, ny = field.shape
    new_field = np.zeros_like(field)
    for i in range(nx - 1):
        for j in range(ny - 1):
            x = i - u[i, j] * dt / dx
            y = j - v[i, j] * dt / dy
            x0 = np.clip(int(x), 0, nx - 1)
            y0 = np.clip(int(y), 0, ny - 1)
            new_field[i, j] = field[x0, y0]
    return new_field


# Obliczanie dywergencji prędkości
def compute_divergence(u, v, dx, dy):
    div = np.zeros((NX, NY))
    div[:, :] = (u[1:, :] - u[:-1, :]) / dx + (v[:, 1:] - v[:, :-1]) / dy
    return div


# Rozwiązywanie równania Poissona (dla korekcji ciśnienia)
def solve_pressure(div, dx, dy, iterations=50):
    for _ in range(iterations):
        p[1:-1, 1:-1] = 0.25 * (
            p[2:, 1:-1]
            + p[:-2, 1:-1]
            + p[1:-1, 2:]
            + p[1:-1, :-2]
            - div[1:-1, 1:-1] * dx * dy
        )
    return p


# Korekcja prędkości na podstawie ciśnienia
def correct_velocity(u, v, p, dx, dy):
    u[1:-1, :] -= DT * (p[1:, :] - p[:-1, :]) / dx
    v[:, 1:-1] -= DT * (p[:, 1:] - p[:, :-1]) / dy
    return u, v


# Główna pętla symulacji
for step in range(1000):
    # Adwekcja
    u = advect(u, v, u, DT, DX, DY)
    v = advect(u, v, v, DT, DX, DY)

    # Dodanie sił zewnętrznych
    u += DT * fx
    v += DT * fy

    # Obliczanie dywergencji
    div = compute_divergence(u, v, DX, DY)

    # Rozwiązywanie Poissona dla ciśnienia
    p = solve_pressure(div, DX, DY)

    # Korekcja prędkości
    u, v = correct_velocity(u, v, p, DX, DY)

    # Wizualizacja
    if step % 10 == 0:
        plt.clf()
        plt.quiver(u[:-1, :], v[:, :-1])
        plt.imshow(
            p.T,
            origin="lower",
            extent=[0, NX * DX, 0, NY * DY],
            alpha=0.5,
            cmap="coolwarm",
        )
        plt.colorbar(label="Pressure")
        plt.title(f"Step {step}")
        plt.pause(0.1)

plt.show()

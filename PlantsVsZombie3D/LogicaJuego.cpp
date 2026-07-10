#include "LogicaJuego.h"
#include <cmath>
#include <cstdlib>

LogicaJuego::LogicaJuego() : soles(150), oleadaActual(0), temporizadorOleada(0.0f) {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            tablero[f][c] = nullptr;
        }
    }
}

LogicaJuego::~LogicaJuego() {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            if (tablero[f][c] != nullptr) {
                delete tablero[f][c];
            }
        }
    }
}

bool LogicaJuego::plantar(int fila, int columna, TipoPlanta tipo) {
    if (fila < 0 || fila >= 5 || columna < 0 || columna >= 9) return false;
    if (tablero[fila][columna] != nullptr) return false;

    int costo = (tipo == GIRASOL) ? 50 : 100;
    if (soles < costo) return false;

    soles -= costo;
    tablero[fila][columna] = new Planta(tipo, fila, columna);
    return true;
}

void LogicaJuego::aparecerZombie(int fila, TipoZombie tipo) {
    if (fila >= 0 && fila < 5) {
        listaZombies.push_back(Zombie(tipo, fila));
    }
}

Planta* LogicaJuego::obtenerPlanta(int fila, int columna) {
    return tablero[fila][columna];
}

void LogicaJuego::gestionarOleadas(float deltaTime) {
    temporizadorOleada += deltaTime;
    // Cada 20 segundos del juego simulado se incrementa y lanza una oleada automática
    if (temporizadorOleada >= 20.0f || (listaZombies.empty() && oleadaActual == 0)) {
        oleadaActual++;
        temporizadorOleada = 0.0f;
        int cantidadZombies = oleadaActual + 1;
        for (int i = 0; i < cantidadZombies; i++) {
            int filaAleatoria = rand() % 5;
            aparecerZombie(filaAleatoria, NORMAL);
        }
    }
}

void LogicaJuego::actualizar(float deltaTime) {
    gestionarOleadas(deltaTime);

    // --- 1. PROYECTILES (MOVIMIENTO E IMPACTOS) ---
    for (int i = (int)listaProyectiles.size() - 1; i >= 0; i--) {
        Proyectil& guisante = listaProyectiles[i];

        if (guisante.debeEliminarse) {
            guisante.escala -= 5.0f * deltaTime; // El proyectil reduce progresivamente su escala a cero
            if (guisante.escala <= 0.0f) {
                listaProyectiles.erase(listaProyectiles.begin() + i);
            }
            continue;
        }

        guisante.posicionX += guisante.velocidad * deltaTime;

        if (guisante.posicionX > 9.5f) {
            listaProyectiles.erase(listaProyectiles.begin() + i);
            continue;
        }

        // Si supera o iguala la posición X de un zombie en la misma fila [especificación de impacto]
        for (size_t z = 0; z < listaZombies.size(); z++) {
            Zombie& zombie = listaZombies[z];
            if (zombie.fila == guisante.fila && guisante.posicionX >= zombie.posicionX && (guisante.posicionX - zombie.posicionX) < 0.5f) {
                zombie.vida -= guisante.danio;
                guisante.debeEliminarse = true;
                break;
            }
        }
    }

    // --- 2. PLANTAS (DISPAROS Y PRODUCCIÓN) ---
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            Planta* planta = tablero[f][c];
            if (planta == nullptr) continue;

            planta->cooldownAtaque += deltaTime;

            if (planta->tipo == LANZAGUISANTES && planta->cooldownAtaque >= 2.0f) {
                bool hayZombie = false;
                for (const auto& z : listaZombies) {
                    if (z.fila == f) { hayZombie = true; break; }
                }
                if (hayZombie) {
                    listaProyectiles.push_back(Proyectil(f, c + 0.5f));
                    planta->cooldownAtaque = 0.0f;
                }
            }
            else if (planta->tipo == GIRASOL && planta->cooldownAtaque >= 7.0f) {
                soles += 25;
                planta->cooldownAtaque = 0.0f;
            }
        }
    }

    // --- 3. ZOMBIES (MOVIMIENTO Y DAÑO A PLANTAS) ---
    for (int i = (int)listaZombies.size() - 1; i >= 0; i--) {
        Zombie& zombie = listaZombies[i];

        if (zombie.vida <= 0) {
            listaZombies.erase(listaZombies.begin() + i);
            continue;
        }

        int columnaActual = (int)std::floor(zombie.posicionX);

        // Si el zombie coincide con una casilla ocupada por una planta en su fila
        if (columnaActual >= 0 && columnaActual < 9 && tablero[zombie.fila][columnaActual] != nullptr) {
            zombie.estaComiendo = true;
            Planta* plantaAfectada = tablero[zombie.fila][columnaActual];
            plantaAfectada->vida -= (int)(zombie.danio * deltaTime);

            if (plantaAfectada->vida <= 0) {
                delete tablero[zombie.fila][columnaActual];
                tablero[zombie.fila][columnaActual] = nullptr;
                zombie.estaComiendo = false;
            }
        }
        else {
            zombie.estaComiendo = false;
        }

        if (!zombie.estaComiendo) {
            zombie.posicionX -= zombie.velocidad * deltaTime;
        }
    }
}
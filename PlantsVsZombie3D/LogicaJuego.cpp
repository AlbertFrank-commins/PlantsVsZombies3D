#include "LogicaJuego.h"
#include <cmath>
#include <cstdlib>

LogicaJuego* g_Logica = nullptr; // Inicialización de la variable global externa

LogicaJuego::LogicaJuego() : soles(100), oleadaActual(0), temporizadorOleada(0.0f), temporizadorSol(0.0f) {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            tablero[f][c] = nullptr;
        }
    }
    // Arranque limpio: sin plantas ni zombies de maqueta. El jugador
    // empieza con 100 soles para comprar su primera planta, y los
    // zombies llegan solos mediante gestionarOleadas() (la primera
    // oleada se dispara automaticamente porque listaZombies empieza vacia).
}

LogicaJuego::~LogicaJuego() {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            if (tablero[f][c] != nullptr) delete tablero[f][c];
        }
    }
}

bool LogicaJuego::plantar(int fila, int columna, TipoPlanta tipo) {
    if (fila < 0 || fila >= 5 || columna < 0 || columna >= 9) return false;
    if (tablero[fila][columna] != nullptr) return false;
    int costo = CostoPlanta(tipo); // <- centralizado en Estructuras.h (lo usa tambien el HUD)
    if (soles < costo) return false;
    soles -= costo;
    tablero[fila][columna] = new Planta(tipo, fila, columna);
    return true;
}

bool LogicaJuego::quitarPlanta(int fila, int columna) {
    if (fila < 0 || fila >= 5 || columna < 0 || columna >= 9) return false;
    Planta* planta = tablero[fila][columna];
    if (planta == nullptr) return false;

    soles += CostoPlanta(planta->tipo); // reembolso completo de lo que costo plantarla
    delete planta;
    tablero[fila][columna] = nullptr;
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
    if (temporizadorOleada >= 20.0f || (listaZombies.empty() && oleadaActual == 0)) {
        oleadaActual++;
        temporizadorOleada = 0.0f;
        int cantidadZombies = oleadaActual + 1;
        for (int i = 0; i < cantidadZombies; i++) {
            int filaAleatoria = rand() % 5;
            aparecerZombie(filaAleatoria, (rand() % 2 == 0) ? NORMAL : CON_CONO);
        }
    }
}

void LogicaJuego::gestionarSolesCaidos(float deltaTime) {
    // 1) Aparicion periodica de soles nuevos en una celda aleatoria del tablero
    temporizadorSol += deltaTime;
    if (temporizadorSol >= SOL_INTERVALO_APARICION) {
        temporizadorSol = 0.0f;
        float columna = (float)(rand() % COLUMNAS_TABLERO) + 0.5f;
        float fila = (float)(rand() % FILAS_TABLERO) + 0.5f;
        listaSoles.push_back(SolCaido(columna, fila));
    }

    // 2) Simulacion de caida y desvanecimiento de cada sol activo
    for (int i = (int)listaSoles.size() - 1; i >= 0; i--) {
        SolCaido& sol = listaSoles[i];
        if (!sol.enSuelo) {
            sol.y -= SOL_VELOCIDAD_CAIDA * deltaTime;
            if (sol.y <= SOL_ALTURA_SUELO) {
                sol.y = SOL_ALTURA_SUELO;
                sol.enSuelo = true;
            }
        }
        else {
            sol.tiempoEnSuelo += deltaTime;
            if (sol.tiempoEnSuelo >= SOL_TIEMPO_VIDA_SUELO) {
                sol.debeEliminarse = true;
            }
        }
        if (sol.debeEliminarse) {
            listaSoles.erase(listaSoles.begin() + i);
        }
    }
}

void LogicaJuego::recolectarSol(int indice) {
    if (indice < 0 || indice >= (int)listaSoles.size()) return;
    soles += SOL_VALOR;
    listaSoles.erase(listaSoles.begin() + indice);
}

void LogicaJuego::actualizar(float deltaTime) {
    gestionarOleadas(deltaTime);
    gestionarSolesCaidos(deltaTime);

    // 1. Proyectiles
    for (int i = (int)listaProyectiles.size() - 1; i >= 0; i--) {
        Proyectil& guisante = listaProyectiles[i];
        if (guisante.debeEliminarse) {
            guisante.escala -= 5.0f * deltaTime;
            if (guisante.escala <= 0.0f) {
                listaProyectiles.erase(listaProyectiles.begin() + i);
            }
            continue;
        }
        guisante.posicionX += guisante.velocidad * deltaTime;
        if (guisante.posicionX > 18.0f) { // Ajustado al tamaño de tu tablero (9 celdas * 2.0f)
            listaProyectiles.erase(listaProyectiles.begin() + i);
            continue;
        }
        for (size_t z = 0; z < listaZombies.size(); z++) {
            Zombie& zombie = listaZombies[z];
            if (zombie.fila == guisante.fila && guisante.posicionX >= zombie.posicionX &&
                (guisante.posicionX - zombie.posicionX) < 0.5f) {
                zombie.vida -= guisante.danio;
                guisante.debeEliminarse = true;
                break;
            }
        }
    }

    // 2. Plantas
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            Planta* planta = tablero[f][c];
            if (planta == nullptr) continue;
            planta->cooldownAtaque += deltaTime;
            if (planta->tipo == LANZAGUISANTES && planta->cooldownAtaque >= 2.0f) {
                bool hayZombie = false;
                for (const auto& z : listaZombies) {
                    if (z.fila == f && z.posicionX > (c * 2.0f)) { hayZombie = true; break; }
                }
                if (hayZombie) {
                    listaProyectiles.push_back(Proyectil(f, (c * 2.0f) + 1.2f));
                    planta->cooldownAtaque = 0.0f;
                }
            }
            else if (planta->tipo == GIRASOL && planta->cooldownAtaque >= 7.0f) {
                soles += 25;
                planta->cooldownAtaque = 0.0f;
            }
        }
    }

    // 3. Zombies (sin colision con plantas por el momento: caminan
    // de largo y no se acumulan sobre las celdas ocupadas). Solo
    // se eliminan si su vida llega a 0 (por ejemplo, por proyectiles).
    for (int i = (int)listaZombies.size() - 1; i >= 0; i--) {
        Zombie& zombie = listaZombies[i];
        if (zombie.vida <= 0) {
            listaZombies.erase(listaZombies.begin() + i);
            continue;
        }
        zombie.estaComiendo = false;
        zombie.posicionX -= zombie.velocidad * deltaTime;
    }
}
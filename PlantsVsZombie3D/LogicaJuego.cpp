#include "LogicaJuego.h"
#include <cmath>
#include <cstdlib>

LogicaJuego* g_Logica = nullptr; // Inicializaci�n de la variable global externa

LogicaJuego::LogicaJuego() : soles(100), oleadaActual(0), temporizadorOleada(0.0f),
temporizadorSol(0.0f), estadoJuego(JUEGO_MENU) {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            tablero[f][c] = nullptr;
        }
        listaCarretillas.push_back(Carretilla(f)); // NUEVO: una por fila
    }
}

LogicaJuego::~LogicaJuego() {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            if (tablero[f][c] != nullptr) delete tablero[f][c];
        }
    }
}

// NUEVO: reinicia toda la partida (se llama al elegir "Jugar" en el
// menu, o al reiniciar despues de un Game Over).
void LogicaJuego::ReiniciarJuego() {
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            if (tablero[f][c] != nullptr) delete tablero[f][c];
            tablero[f][c] = nullptr;
        }
    }
    listaZombies.clear();
    listaProyectiles.clear();
    listaSoles.clear();
    eventosSonido.clear();
    listaCarretillas.clear();
    for (int f = 0; f < 5; f++) listaCarretillas.push_back(Carretilla(f)); // NUEVO
    soles = 100;
    oleadaActual = 0;
    temporizadorOleada = 0.0f;
    temporizadorSol = 0.0f;
    estadoJuego = JUEGO_JUGANDO;
}

bool LogicaJuego::plantar(int fila, int columna, TipoPlanta tipo) {
    if (fila < 0 || fila >= 5 || columna < 0 || columna >= 9) return false;
    if (tablero[fila][columna] != nullptr) return false;
    int costo = CostoPlanta(tipo);
    if (soles < costo) return false;
    soles -= costo;
    tablero[fila][columna] = new Planta(tipo, fila, columna);
    eventosSonido.push_back("plantar"); // NUEVO
    return true;
}

bool LogicaJuego::quitarPlanta(int fila, int columna) {
    if (fila < 0 || fila >= 5 || columna < 0 || columna >= 9) return false;
    Planta* planta = tablero[fila][columna];
    if (planta == nullptr) return false;
    soles += CostoPlanta(planta->tipo);
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
    temporizadorSol += deltaTime;
    if (temporizadorSol >= SOL_INTERVALO_APARICION) {
        temporizadorSol = 0.0f;
        float columna = (float)(rand() % COLUMNAS_TABLERO) + 0.5f;
        float fila = (float)(rand() % FILAS_TABLERO) + 0.5f;
        listaSoles.push_back(SolCaido(columna, fila));
    }
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

// ============================================================
// NUEVO: colision real planta-zombie.
// Cada celda mide 2.0f en el sistema de coordenadas de
// posicionX (ver el limite de 18.0f = 9 celdas * 2.0f usado en
// el resto del archivo). Si un zombie esta parado sobre la
// celda de una planta, se detiene y le resta vida por segundo
// en vez de atravesarla.
// ============================================================
void LogicaJuego::gestionarColisionesPlantaZombie(float deltaTime) {
    const float ANCHO_CELDA_LOGICO = 2.0f;
    for (auto& zombie : listaZombies) {
        int columna = (int)(zombie.posicionX / ANCHO_CELDA_LOGICO);
        if (columna < 0 || columna >= COLUMNAS_TABLERO) {
            zombie.estaComiendo = false;
            continue;
        }
        Planta* planta = tablero[zombie.fila][columna];
        if (planta == nullptr) {
            zombie.estaComiendo = false;
            continue;
        }
        // Hay una planta en la celda actual del zombie: se detiene y ataca
        zombie.estaComiendo = true;
        planta->vida -= (int)(ZOMBIE_DANIO_POR_SEGUNDO * deltaTime);
        if (planta->vida <= 0) {
            eventosSonido.push_back("planta_destruida");
            delete planta;
            tablero[zombie.fila][columna] = nullptr;
            zombie.estaComiendo = false;
        }
    }
}

// ============================================================
// NUEVO: mueve las carretillas activas y arrasa zombies en su fila.
// ============================================================
void LogicaJuego::gestionarCarretillas(float deltaTime) {
    for (auto& carretilla : listaCarretillas) {
        if (!carretilla.activa) continue;
        carretilla.posicionX += VELOCIDAD_CARRETILLA * deltaTime;
        for (int i = (int)listaZombies.size() - 1; i >= 0; i--) {
            Zombie& z = listaZombies[i];
            if (z.fila == carretilla.fila && z.posicionX <= carretilla.posicionX) {
                listaZombies.erase(listaZombies.begin() + i);
            }
        }
        if (carretilla.posicionX > 19.0f) {
            carretilla.activa = false; // termino su recorrido: ya se gasto
        }
    }
}

void LogicaJuego::recolectarSol(int indice) {
    if (indice < 0 || indice >= (int)listaSoles.size()) return;
    soles += SOL_VALOR;
    listaSoles.erase(listaSoles.begin() + indice);
    eventosSonido.push_back("sol"); // NUEVO
}

void LogicaJuego::actualizar(float deltaTime) {
    // Fuera de la partida (menu, pausa, game over) no se simula nada.
    if (estadoJuego != JUEGO_JUGANDO) return;

    gestionarOleadas(deltaTime);
    gestionarSolesCaidos(deltaTime);
    gestionarColisionesPlantaZombie(deltaTime); // NUEVO
    gestionarCarretillas(deltaTime); // NUEVO

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
        if (guisante.posicionX > 18.0f) {
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

    // 2. Plantas: ataque y produccion de soles, + actualizar EstadoVida (NUEVO)
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            Planta* planta = tablero[f][c];
            if (planta == nullptr) continue;

            planta->estadoVida = CalcularEstadoVida(planta->vida, planta->vidaMaxima); // NUEVO

            planta->cooldownAtaque += deltaTime;
            if (planta->tipo == LANZAGUISANTES && planta->cooldownAtaque >= 2.0f) {
                bool hayZombie = false;
                for (const auto& z : listaZombies) {
                    if (z.fila == f && z.posicionX > (c * 2.0f)) { hayZombie = true; break; }
                }
                if (hayZombie) {
                    listaProyectiles.push_back(Proyectil(f, (c * 2.0f) + 1.2f));
                    planta->cooldownAtaque = 0.0f;
                    eventosSonido.push_back("disparo"); // NUEVO
                }
            }
            else if (planta->tipo == GIRASOL && planta->cooldownAtaque >= 7.0f) {
                soles += 25;
                planta->cooldownAtaque = 0.0f;
            }
        }
    }

    // 3. Zombies: ahora respetan estaComiendo (NUEVO) y no avanzan
    // mientras estan comiendose una planta.
    for (int i = (int)listaZombies.size() - 1; i >= 0; i--) {
        Zombie& zombie = listaZombies[i];
        if (zombie.vida <= 0) {
            listaZombies.erase(listaZombies.begin() + i);
            continue;
        }
        if (!zombie.estaComiendo) {
            zombie.posicionX -= zombie.velocidad * deltaTime;
        }
        // NUEVO: si un zombie cruza el borde izquierdo del tablero...
        if (zombie.posicionX <= 0.0f) {
            Carretilla& carretilla = listaCarretillas[zombie.fila];
            if (carretilla.disponible && !carretilla.activa) {
                // ...y la carretilla de esa fila sigue disponible: se
                // activa y sale a arrasar la fila (ver gestionarCarretillas).
                carretilla.activa = true;
                carretilla.disponible = false;
                carretilla.posicionX = 0.0f;
                eventosSonido.push_back("carretilla");
            }
            else {
                // ...y esa fila ya se quedo sin carretilla: recien ahi
                // se pierde la partida.
                estadoJuego = JUEGO_GAME_OVER;
            }
            listaZombies.erase(listaZombies.begin() + i);
            continue;
        }
    }
}
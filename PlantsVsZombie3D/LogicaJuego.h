#ifndef LOGICAJUEGO_H
#define LOGICAJUEGO_H

#include <vector>
#include "Estructuras.h"

class LogicaJuego {
private:
    Planta* tablero[5][9];
    float temporizadorOleada;
    float temporizadorSol;

    void gestionarOleadas(float deltaTime);
    void gestionarSolesCaidos(float deltaTime);

public:
    std::vector<Zombie> listaZombies;
    std::vector<Proyectil> listaProyectiles;
    std::vector<SolCaido> listaSoles;
    int soles;
    int oleadaActual;

    LogicaJuego();
    ~LogicaJuego();

    bool plantar(int fila, int columna, TipoPlanta tipo);

    // Quita la planta de (fila, columna) si existe y devuelve al
    // jugador exactamente lo que le costo plantarla (CostoPlanta),
    // igual que "desplantar" en el juego original. Devuelve false
    // si la celda esta vacia o fuera de rango, sin cobrar nada.
    bool quitarPlanta(int fila, int columna);

    void aparecerZombie(int fila, TipoZombie tipo);
    Planta* obtenerPlanta(int fila, int columna);
    void actualizar(float deltaTime);

    // Recolecta el sol en la posicion "indice" de listaSoles (el
    // indice lo resuelve ManejadorEventos proyectando cada sol a
    // pantalla y comparando con el clic). Suma SOL_VALOR a "soles"
    // y elimina el sol de la lista.
    void recolectarSol(int indice);
};

// Instancia global única del motor lógico que se compartirá con el Módulo 3
extern LogicaJuego* g_Logica;

#endif
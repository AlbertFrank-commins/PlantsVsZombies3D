#ifndef LOGICAJUEGO_H
#define LOGICAJUEGO_H

#include <vector>
#include "Estructuras.h"

class LogicaJuego {
private:
    Planta* tablero[5][9];
    float temporizadorOleada;
    void gestionarOleadas(float deltaTime);

public:
    std::vector<Zombie> listaZombies;
    std::vector<Proyectil> listaProyectiles;
    int soles;
    int oleadaActual;

    LogicaJuego();
    ~LogicaJuego();

    bool plantar(int fila, int columna, TipoPlanta tipo);
    void aparecerZombie(int fila, TipoZombie tipo);
    Planta* obtenerPlanta(int fila, int columna);
    void actualizar(float deltaTime);
};

// Instancia global única del motor lógico que se compartirá con el Módulo 3
extern LogicaJuego* g_Logica;

#endif
#ifndef LOGICAJUEGO_H
#define LOGICAJUEGO_H

#include <vector>
#include "Estructuras.h"

class LogicaJuego {
private:
    Planta* tablero[5][9]; // Matriz lógica del juego (5 filas por 9 columnas)
    float temporizadorOleada;

    void gestionarOleadas(float deltaTime); // Control secuencial de oleadas automáticas

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
    void actualizar(float deltaTime); // Ciclo general de simulación matemática
};

#endif

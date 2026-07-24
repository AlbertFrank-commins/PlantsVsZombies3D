#ifndef LOGICAJUEGO_H
#define LOGICAJUEGO_H

#include <vector>
#include <string>
#include "Estructuras.h"

class LogicaJuego {
private:
    Planta* tablero[5][9];
    float temporizadorOleada;
    float temporizadorSol;

    void gestionarOleadas(float deltaTime);
    void gestionarSolesCaidos(float deltaTime);
    void gestionarColisionesPlantaZombie(float deltaTime); // NUEVO
    void gestionarCarretillas(float deltaTime); // NUEVO

public:
    std::vector<Zombie> listaZombies;
    std::vector<Proyectil> listaProyectiles;
    std::vector<SolCaido> listaSoles;
    std::vector<Carretilla> listaCarretillas; // NUEVO: una por fila
    int soles;
    int oleadaActual;
    // NUEVO: tiempo de recarga restante por tipo de semilla (indexado
    // por el enum TipoPlanta). Mientras sea > 0, no se puede plantar
    // esa semilla de nuevo.
    float cooldownSemillas[5];

    // NUEVO: estado global del juego.
    EstadoJuego estadoJuego;

    // NUEVO: cola simple de eventos de sonido. LogicaJuego NO conoce
    // GestorAudio; solo encola nombres de evento como "plantar",
    // "disparo", "mordida", "sol". ManejadorEventos los lee cada
    // frame y llama a GestorAudio, luego vacia la cola.
    std::vector<std::string> eventosSonido;
    void VaciarEventosSonido() { eventosSonido.clear(); }

    LogicaJuego();
    ~LogicaJuego();

    void ReiniciarJuego(); // NUEVO

    bool plantar(int fila, int columna, TipoPlanta tipo);
    bool quitarPlanta(int fila, int columna);
    void aparecerZombie(int fila, TipoZombie tipo);
    Planta* obtenerPlanta(int fila, int columna);
    void actualizar(float deltaTime);
    void recolectarSol(int indice);
};

extern LogicaJuego* g_Logica;

#endif
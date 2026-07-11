#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

enum TipoPlanta { NINGUNA, LANZAGUISANTES, GIRASOL, NUEZ, CEREZA_EXPLOSIVA };
enum TipoZombie { NORMAL, CON_CONO, CUBETA };
enum EstadoRender { ESTADO_ACTIVO, ESTADO_DESTRUIDO }; // Para sincronizar con Render.h

// ============================================================
// Dimensiones y centro del tablero (5 filas x 9 columnas, celdas
// de 1.0f en Render.cpp). Se centralizan aqui porque tanto la
// camara del Modulo 2 (Render.cpp) como el sistema de clics del
// Modulo 3 (ManejadorEventos.cpp) necesitan el mismo valor: si
// se definieran por separado, un cambio en uno rompe al otro.
// ============================================================
const int   FILAS_TABLERO = 5;
const int   COLUMNAS_TABLERO = 9;
const float CENTRO_TABLERO_X = COLUMNAS_TABLERO / 2.0f; // 4.5f
const float CENTRO_TABLERO_Z = FILAS_TABLERO / 2.0f;    // 2.5f

// Costo en soles de cada planta. Centralizado aqui para que
// LogicaJuego::plantar() y el HUD de Render.cpp (que dibuja el
// precio sobre cada semilla) usen siempre el mismo numero.
inline int CostoPlanta(TipoPlanta tipo) {
    switch (tipo) {
    case GIRASOL:          return 50;
    case NUEZ:              return 50;
    case CEREZA_EXPLOSIVA:  return 150;
    case LANZAGUISANTES:    return 100;
    default:                return 0;
    }
}

// ============================================================
// Soles caidos: parametros de aparicion, caida y recoleccion.
// Centralizados aqui por la misma razon que CostoPlanta: los usa
// tanto LogicaJuego (simulacion) como Render.cpp / ManejadorEventos
// (dibujo y deteccion de clic).
// ============================================================
const float SOL_ALTURA_INICIAL = 8.0f;  // altura desde la que "cae" el sol
const float SOL_VELOCIDAD_CAIDA = 2.0f;  // unidades/segundo
const float SOL_ALTURA_SUELO = 0.35f; // altura de reposo sobre el cesped
const float SOL_TIEMPO_VIDA_SUELO = 8.0f;  // segundos que dura en el suelo antes de desvanecerse
const float SOL_INTERVALO_APARICION = 6.0f;  // segundos entre soles nuevos
const int   SOL_VALOR = 25;    // soles que otorga al recolectarlo

struct Planta {
    TipoPlanta tipo;
    int vida;
    int fila;
    int columna;
    float cooldownAtaque;
    EstadoRender estado;

    Planta() : tipo(NINGUNA), vida(0), fila(0), columna(0), cooldownAtaque(0.0f), estado(ESTADO_DESTRUIDO) {}
    Planta(TipoPlanta t, int f, int c) : tipo(t), fila(f), columna(c), cooldownAtaque(0.0f), estado(ESTADO_ACTIVO) {
        if (t == GIRASOL) vida = 150;
        else if (t == NUEZ) vida = 600; // Muro con mucha vida
        else if (t == CEREZA_EXPLOSIVA) vida = 50;
        else vida = 200; // Lanzaguisantes
    }
};

struct Zombie {
    TipoZombie tipo;
    int vida;
    int fila;
    float posicionX;
    float velocidad;
    int danio;
    bool estaComiendo;
    EstadoRender estado;

    Zombie(TipoZombie t, int f) : tipo(t), fila(f), posicionX(18.0f), estaComiendo(false), danio(20), estado(ESTADO_ACTIVO) {
        if (t == NORMAL) { vida = 200; velocidad = 0.5f; }
        else if (t == CON_CONO) { vida = 400; velocidad = 0.5f; }
        else { vida = 600; velocidad = 0.4f; } // Cubeta
    }
};

struct Proyectil {
    int fila;
    float posicionX;
    float velocidad;
    int danio;
    float escala;
    bool debeEliminarse;
    EstadoRender estado;

    Proyectil(int f, float inicioX) : fila(f), posicionX(inicioX), velocidad(4.0f), danio(20), escala(1.0f), debeEliminarse(false), estado(ESTADO_ACTIVO) {}
};

// Sol caido: aparece sobre una celda aleatoria del tablero, cae
// desde el cielo y descansa sobre el cesped un tiempo limitado
// antes de desvanecerse si el jugador no hace clic sobre el.
// x, z usan el mismo sistema de coordenadas que las plantas
// (celdas de 1.0f), asi que se puede dibujar con un simple
// glTranslatef(sol.x, sol.y, sol.z), sin conversiones extra.
struct SolCaido {
    float x;
    float z;
    float y;
    bool enSuelo;
    float tiempoEnSuelo;
    bool debeEliminarse;
    EstadoRender estado;

    SolCaido(float posX, float posZ)
        : x(posX), z(posZ), y(SOL_ALTURA_INICIAL), enSuelo(false),
        tiempoEnSuelo(0.0f), debeEliminarse(false), estado(ESTADO_ACTIVO) {
    }
};

#endif
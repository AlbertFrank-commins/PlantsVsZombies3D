#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

enum TipoPlanta { NINGUNA, LANZAGUISANTES, GIRASOL, NUEZ, CEREZA_EXPLOSIVA };
enum TipoZombie { NORMAL, CON_CONO, CUBETA };
enum EstadoRender { ESTADO_ACTIVO, ESTADO_DESTRUIDO }; // Para sincronizar con Render.h

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

#endif
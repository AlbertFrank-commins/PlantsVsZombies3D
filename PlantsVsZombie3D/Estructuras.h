#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

// Tipos requeridos para la lógica del tablero
enum TipoPlanta { NINGUNA, LANZAGUISANTES, GIRASOL };
enum TipoZombie { NORMAL, CON_CONO };

struct Planta {
    TipoPlanta tipo;
    int vida;
    int fila;
    int columna;
    float cooldownAtaque;

    Planta() : tipo(NINGUNA), vida(0), fila(0), columna(0), cooldownAtaque(0.0f) {}
    Planta(TipoPlanta t, int f, int c) : tipo(t), fila(f), columna(c), cooldownAtaque(0.0f) {
        vida = (t == GIRASOL) ? 150 : 200;
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

    Zombie(TipoZombie t, int f) : tipo(t), fila(f), posicionX(9.0f), estaComiendo(false), danio(20) {
        vida = (t == NORMAL) ? 200 : 400;
        velocidad = 0.5f; // Avance constante en el eje X
    }
};

struct Proyectil {
    int fila;
    float posicionX;
    float velocidad;
    int danio;
    float escala;       // Para controlar la reducción visual a cero especificada por gráficos
    bool debeEliminarse;

    Proyectil(int f, float inicioX) : fila(f), posicionX(inicioX), velocidad(4.0f), danio(20), escala(1.0f), debeEliminarse(false) {}
};

#endif

#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

enum TipoPlanta { NINGUNA, LANZAGUISANTES, GIRASOL, NUEZ, CEREZA_EXPLOSIVA };
enum TipoZombie { NORMAL, CON_CONO, CUBETA };
enum EstadoRender { ESTADO_ACTIVO, ESTADO_DESTRUIDO }; // Para sincronizar con Render.h

// ============================================================
// NUEVO: Estado de vida visual de una planta, calculado como
// porcentaje de vida restante. Lo calcula LogicaJuego (Modulo 1)
// y lo consume Render/GestorSprites (Modulo 2) para elegir que
// sprite mostrar. Ninguno de los dos modulos necesita conocer
// como trabaja el otro por dentro.
// ============================================================
enum EstadoVida { VIDA_SANA, VIDA_DANADA, VIDA_CRITICA };

inline EstadoVida CalcularEstadoVida(int vidaActual, int vidaMaxima) {
    if (vidaMaxima <= 0) return VIDA_CRITICA;
    float porcentaje = (float)vidaActual / (float)vidaMaxima;
    if (porcentaje > 0.60f) return VIDA_SANA;
    if (porcentaje > 0.25f) return VIDA_DANADA;
    return VIDA_CRITICA;
}

// ============================================================
// NUEVO: Estado global del juego (maquina de estados). Controla
// que dibuja Render.cpp en callbackDisplay y que interpreta
// ManejadorEventos.cpp en callbackMouse/Teclado.
// ============================================================
enum EstadoJuego { JUEGO_MENU, JUEGO_JUGANDO, JUEGO_PAUSA, JUEGO_GAME_OVER };

// ============================================================
// Dimensiones y centro del tablero (5 filas x 9 columnas, celdas
// de 1.0f en Render.cpp).
// ============================================================
const int   FILAS_TABLERO = 5;
const int   COLUMNAS_TABLERO = 9;
const float CENTRO_TABLERO_X = COLUMNAS_TABLERO / 2.0f; // 4.5f (ya no se usa para camara, se deja por compatibilidad)
const float CENTRO_TABLERO_Z = FILAS_TABLERO / 2.0f;    // 2.5f

// ============================================================
// NUEVO: Tablero 100% en 2D (coordenadas de PANTALLA, en pixeles).
// -----------------------------------------------------------
// Ya no hay camara ni matrices 3D: cada celda del cesped ocupa un
// rectangulo fijo en pixeles y todas las entidades (plantas,
// zombies, proyectiles, soles) se ubican con simples sumas y
// multiplicaciones sobre estos origenes, igual que en el juego
// original. LogicaJuego.cpp NO cambia: sigue trabajando con
// fila/columna y con "posicionX" en unidades logicas (2.0f por
// celda para zombies/proyectiles, 1.0f por celda para soles); las
// funciones de abajo son las unicas responsables de traducir esas
// unidades logicas a pixeles para dibujar.
// ============================================================
const int   TABLERO_ORIGEN_X = 175;   // borde izquierdo del cesped jugable
const int   TABLERO_ORIGEN_Y = 95;    // borde superior del cesped jugable
const int   CELDA_ANCHO_PX = 88;
const int   CELDA_ALTO_PX = 100;
const float ANCHO_CELDA_LOGICO = 2.0f; // unidades logicas por celda (ver posicionX en LogicaJuego.cpp)

inline float ColumnaAPixelX(float columna) { return TABLERO_ORIGEN_X + columna * CELDA_ANCHO_PX; }
inline float FilaAPixelY(float fila) { return TABLERO_ORIGEN_Y + fila * CELDA_ALTO_PX; }

// Centro horizontal de una celda (para sembrar una planta ahi).
inline float CentroColumnaPixelX(int columna) { return ColumnaAPixelX((float)columna + 0.5f); }
// Linea base (los "pies") de una fila, un poco arriba del borde inferior de la celda.
inline float BasePixelY(int fila) { return FilaAPixelY((float)fila) + CELDA_ALTO_PX - 8.0f; }

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
// ============================================================
const float SOL_ALTURA_INICIAL = 8.0f;
const float SOL_VELOCIDAD_CAIDA = 2.0f;
const float SOL_ALTURA_SUELO = 0.35f;
const float SOL_TIEMPO_VIDA_SUELO = 8.0f;
const float SOL_INTERVALO_APARICION = 6.0f;
const int   SOL_VALOR = 25;

// NUEVO: da�o por segundo que un zombie hace a una planta cuando
// esta "comiendola" (colision real planta-zombie).
const int   ZOMBIE_DANIO_POR_SEGUNDO = 40;

struct Planta {
    TipoPlanta tipo;
    int vida;
    int vidaMaxima;      // NUEVO: para calcular el porcentaje de EstadoVida
    int fila;
    int columna;
    float cooldownAtaque;
    EstadoRender estado;
    EstadoVida estadoVida; // NUEVO: se recalcula cada frame en LogicaJuego::actualizar

    Planta() : tipo(NINGUNA), vida(0), vidaMaxima(0), fila(0), columna(0),
        cooldownAtaque(0.0f), estado(ESTADO_DESTRUIDO), estadoVida(VIDA_SANA) {
    }

    Planta(TipoPlanta t, int f, int c) : tipo(t), fila(f), columna(c),
        cooldownAtaque(0.0f), estado(ESTADO_ACTIVO), estadoVida(VIDA_SANA) {
        if (t == GIRASOL) vida = 150;
        else if (t == NUEZ) vida = 600; // Muro con mucha vida
        else if (t == CEREZA_EXPLOSIVA) vida = 50;
        else vida = 200; // Lanzaguisantes
        vidaMaxima = vida;
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

    Zombie(TipoZombie t, int f) : tipo(t), fila(f), posicionX(18.0f),
        estaComiendo(false), danio(20), estado(ESTADO_ACTIVO) {
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
    Proyectil(int f, float inicioX) : fila(f), posicionX(inicioX), velocidad(4.0f),
        danio(20), escala(1.0f), debeEliminarse(false), estado(ESTADO_ACTIVO) {
    }
};

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

// ============================================================
// NUEVO: Carretilla (podadora). Hay una por fila, esperando quieta
// en el borde izquierdo del cesped. Si un zombie llega a la casa en
// su fila y la carretilla de esa fila todavia no se uso, se activa:
// sale corriendo hacia la derecha y elimina a todos los zombies que
// toque en el camino. Cada carretilla solo se puede usar UNA vez por
// partida; si un segundo zombie llega a la casa en una fila cuya
// carretilla ya se gasto, ahi si se pierde la partida.
// ============================================================
const float VELOCIDAD_CARRETILLA = 9.0f; // unidades logicas/seg (cruza el tablero en ~2s)

struct Carretilla {
    int fila;
    float posicionX;  // misma escala logica que zombies/proyectiles (0..18)
    bool activa;       // ya se disparo y esta corriendo ahora mismo
    bool disponible;   // todavia no se uso en esta partida
    Carretilla(int f) : fila(f), posicionX(0.0f), activa(false), disponible(true) {}
};

#endif
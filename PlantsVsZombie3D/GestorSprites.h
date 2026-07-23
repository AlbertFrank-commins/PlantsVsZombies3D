#pragma once
#include <string>
#include <unordered_map>

// ============================================================
// GestorSprites.h
// Modulo 2: Graficos y Estetica
// -----------------------------------------------------------
// Carga imagenes PNG (via stb_image) como texturas de OpenGL y
// las dibuja como quads 2D (GL_QUADS) en vez de las primitivas
// solidas (glutSolidSphere/glutSolidCube) que se usaban antes.
//
// Tiene cache interno: si se pide la misma ruta dos veces, no
// se vuelve a leer el archivo ni se genera una textura nueva.
// ============================================================
namespace GestorSprites {

    // Debe llamarse UNA vez al iniciar el programa (en main.cpp),
    // antes de cargar cualquier sprite.
    void Inicializar();

    // Carga (o recupera de cache) la textura ubicada en "rutaPng"
    // (ruta relativa a la carpeta Assets/Sprites). Devuelve el ID
    // de textura de OpenGL, o 0 si no se pudo cargar.
    unsigned int Cargar(const std::string& rutaPng);

    // Dibuja un quad texturizado centrado en el origen local
    // (0,0,0) del sistema de coordenadas actual (asumiendo que ya
    // se hizo glTranslatef a la celda correspondiente, igual que
    // hacian antes las funciones ModeloX() de Render.cpp).
    // ancho/alto estan en las mismas unidades que el tablero
    // (1.0f = una celda).
    void DibujarQuad(unsigned int idTextura, float ancho, float alto);

    // Variante para el HUD/menu: dibuja un quad en coordenadas de
    // PANTALLA (pixeles), pensado para usarse dentro del bloque
    // ortografico de DibujarInterfaz()/DibujarMenu().
    void DibujarQuadPantalla(unsigned int idTextura, float x, float y, float ancho, float alto);

    // ============================================================
    // NUEVO: dibujo de entidades del tablero en 2D PURO (pixeles),
    // sin ninguna matriz 3D ni rotacion. El sprite se dibuja "de
    // pie", mirando de frente a la camara (como en el juego
    // original), no acostado sobre el cesped.
    // ============================================================

    // Ancla el sprite por sus "pies": (cx, piesY) es el punto medio
    // del borde inferior del sprite. Ideal para plantas y zombies,
    // que estan parados sobre una celda del cesped.
    void DibujarSpriteMundo(unsigned int idTextura, float cx, float piesY, float ancho, float alto);

    // Ancla el sprite por su CENTRO. Ideal para cosas que flotan o
    // vuelan (el sol cayendo, los guisantes en el aire).
    void DibujarSpriteMundoCentrado(unsigned int idTextura, float cx, float cy, float ancho, float alto);

    // Libera todas las texturas cargadas (llamar al cerrar el programa).
    void Liberar();
}
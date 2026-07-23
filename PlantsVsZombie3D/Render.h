#pragma once
#include "Estructuras.h"

// ============================================================
// Render.h
// Modulo 2: Graficos y Estetica (2_Graficos_Estetica)
// -----------------------------------------------------------
// A partir de esta version, las entidades (Planta, Zombie,
// Proyectil, SolCaido) ya NO se dibujan con primitivas solidas
// (glutSolidSphere/glutSolidCube): se dibujan como sprites 2D
// texturizados a traves de GestorSprites. El tablero y el
// escenario de fondo (casa, arbustos, etc.) se mantienen con
// primitivas por ahora; se pueden ir reemplazando por sprites
// mas adelante siguiendo el mismo patron.
// ============================================================

// NUEVO: ya no configura luces (el juego es 2D puro); se deja el
// nombre por compatibilidad, ahora solo prepara el estado GL 2D.
void ConfigurarIluminacion();
void DibujarTablero();
// NUEVO: "tiempo" (segundos acumulados) se usa para animar nubes,
// el pasto, etc. Pasarle 0.0f tambien funciona (queda estatico).
void DibujarEscenario(float tiempo);
// NUEVO: pinta el cielo de fondo (todo el ancho/alto de la ventana);
// se llama antes que nada, incluso antes que el tablero.
void DibujarFondoCielo(int anchoVentana, int altoVentana);

// ---------- Interfaz 2D (HUD) ----------
void DibujarInterfaz(int anchoVentana, int altoVentana, int soles, TipoPlanta tipoSeleccionado);
TipoPlanta SemillaEnPosicion(int x, int y, int anchoVentana);

// ---------- Despachadores por tipo (dibujan un sprite segun el estado) ----------
// NUEVO: reciben directamente su posicion en PIXELES de pantalla
// (ya calculada por quien los llama con las funciones de
// Estructuras.h: CentroColumnaPixelX/BasePixelY/etc.). Ya no usan
// glTranslatef ni ninguna matriz: es dibujo 2D puro.
void DibujarPlanta(const Planta& planta, float cx, float piesY);
void DibujarZombie(const Zombie& zombie, float cx, float piesY);
void DibujarProyectil(const Proyectil& proyectil, float cx, float cy);
void DibujarSol(const SolCaido& sol, float cx, float cy);
// NUEVO: carretilla (podadora), en reposo o corriendo por su fila.
void DibujarCarretilla(const Carretilla& carretilla, float cx, float cy);

// ============================================================
// NUEVO: Pantalla de inicio / menu.
// Dibuja el fondo del menu y los botones Jugar/Salir en
// coordenadas de PANTALLA (pixeles). Devuelve, mediante los
// parametros por referencia, el rectangulo de cada boton para
// que ManejadorEventos pueda resolver el clic sin duplicar
// las coordenadas en dos archivos distintos.
// ============================================================
void DibujarMenu(int anchoVentana, int altoVentana, float tiempo);
void RectanguloBotonJugar(int anchoVentana, int altoVentana, int& x0, int& y0, int& x1, int& y1);
void RectanguloBotonSalir(int anchoVentana, int altoVentana, int& x0, int& y0, int& x1, int& y1);

// NUEVO: pantalla simple de Game Over
void DibujarGameOver(int anchoVentana, int altoVentana);
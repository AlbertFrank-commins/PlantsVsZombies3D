#pragma once
#include "Estructuras.h"

// ============================================================
// Render.h
// Modulo 2: Graficos y Estetica (2_Graficos_Estetica)
// -----------------------------------------------------------
// Responsabilidad: transformar el estado logico (structs de
// Estructuras.h) en geometria 3D mediante primitivas de FreeGLUT,
// aplicando iluminacion y propiedades de materiales.
//
// Esta capa NO decide posiciones globales de mundo ni procesa
// entrada de teclado/tiempo: eso corresponde al Modulo 3
// (Control_Escena / ManejadorEventos), que hace glTranslatef
// hasta la celda correspondiente antes de invocar estas funciones.
//
// NOTA DE DISENO (2D con iluminacion 3D):
// El Modulo 3 configura una proyeccion ORTOGRAFICA (glOrtho) en
// lugar de gluPerspective. Eso quita la distorsion de perspectiva
// y da la lectura "plana" de un juego 2D clasico, pero como las
// entidades se siguen dibujando con primitivas solidas y GL_LIGHTING
// activo, conservan volumen y sombreado 3D real. Este archivo no
// necesita saber que la proyeccion cambio: sigue dibujando igual.
// ============================================================

// ---------- Configuracion global de iluminacion y materiales ----------
// Se llama UNA sola vez, en la inicializacion de OpenGL (main.cpp),
// antes de entrar al bucle principal de FreeGLUT.
void ConfigurarIluminacion();

// ---------- Dibujo del entorno ----------
// Dibuja la cuadricula 5x9 del patio (cesped a cuadros, sin texturas).
void DibujarTablero();

// Dibuja la ambientacion alrededor del tablero: la casa, el camino
// de piedra, las podadoras de emergencia y el seto de arbustos,
// replicando la escena clasica del juego original.
void DibujarEscenario();

// ---------- Interfaz 2D (HUD) ----------
// Cambia temporalmente a una proyeccion ortografica en coordenadas
// de pantalla (pixeles), dibuja el panel de soles y las semillas
// seleccionables, y restaura la proyeccion 3D del tablero al salir.
// Debe llamarse al final de callbackDisplay, antes de glutSwapBuffers.
void DibujarInterfaz(int anchoVentana, int altoVentana, int soles, TipoPlanta tipoSeleccionado);

// Dibuja un sol caido en su posicion actual de caida/reposo (ya
// trasladado por ManejadorEventos a sol.x, sol.y, sol.z).
void DibujarSol(const SolCaido& sol);

// Devuelve el TipoPlanta cuya semilla ocupa la posicion de pantalla
// (x, y) en pixeles (origen arriba-izquierda, igual que los eventos
// de mouse de GLUT), o NINGUNA si el clic no cayo sobre ninguna
// semilla. La usa ManejadorEventos::callbackMouse para resolver clics.
TipoPlanta SemillaEnPosicion(int x, int y, int anchoVentana);

// ---------- Despachadores por tipo (API publica del modulo) ----------
// Cada funcion asume que el sistema de coordenadas ya fue
// trasladado (por el Modulo 3) al centro de la celda correspondiente;
// dibuja la entidad centrada en el origen local (0,0,0).
void DibujarPlanta(const Planta& planta);
void DibujarZombie(const Zombie& zombie);
void DibujarProyectil(const Proyectil& proyectil);

// ---------- Variantes especificas de modelado procedural ----------
// Expuestas por si se necesitan pruebas aisladas o vistas previas;
// en el flujo normal se llega a ellas a traves de los despachadores.
void ModeloLanzaguisantes();
void ModeloGirasol();
void ModeloNuez();
void ModeloCerezaExplosiva();
void ModeloZombieNormal();
void ModeloZombieCono();
void ModeloZombieCubeta();
void ModeloGuisante();
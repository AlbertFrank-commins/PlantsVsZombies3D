#pragma once
#include "Estructuras.h"

// ============================================================
//  Render.h
//  Modulo 2: Graficos y Estetica (2_Graficos_Estetica)
//  -----------------------------------------------------------
//  Responsabilidad: transformar el estado logico (structs de
//  Estructuras.h) en geometria 3D mediante primitivas de FreeGLUT,
//  aplicando iluminacion y propiedades de materiales.
//
//  Esta capa NO decide posiciones globales de mundo ni procesa
//  entrada de teclado/tiempo: eso corresponde al Modulo 3
//  (Control_Escena / ManejadorEventos), que hace glTranslatef
//  hasta la celda correspondiente antes de invocar estas funciones.
// ============================================================

// ---------- Configuracion global de iluminacion y materiales ----------
// Se llama UNA sola vez, en la inicializacion de OpenGL (main.cpp),
// antes de entrar al bucle principal de FreeGLUT.
void ConfigurarIluminacion();

// ---------- Dibujo del entorno ----------
// Dibuja la cuadricula 5x9 del patio (cesped a cuadros, sin texturas).
void DibujarTablero();

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

void ModeloZombieNormal();
void ModeloZombieCono();
void ModeloZombieCubeta();

void ModeloGuisante();
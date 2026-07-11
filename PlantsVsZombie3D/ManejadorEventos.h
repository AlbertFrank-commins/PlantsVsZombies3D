#ifndef MANEJADOREVENTOS_H
#define MANEJADOREVENTOS_H

// Callbacks esenciales de la ventana de OpenGL
void callbackDisplay();
void callbackReshape(int w, int h);
void callbackTimer(int v);
void callbackTeclado(unsigned char key, int x, int y);

// Clic del mouse: primero resuelve si cayo sobre una semilla del HUD
// (selecciona el tipo de planta) y, si no, intenta plantar la semilla
// ya seleccionada en la celda del tablero bajo el cursor.
void callbackMouse(int button, int state, int x, int y);

#endif
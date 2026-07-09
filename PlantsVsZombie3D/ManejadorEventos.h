#ifndef MANEJADOREVENTOS_H
#define MANEJADOREVENTOS_H

// Callbacks esenciales de la ventana de OpenGL
void callbackDisplay();
void callbackReshape(int w, int h);
void callbackTimer(int v);
void callbackTeclado(unsigned char key, int x, int y);

#endif
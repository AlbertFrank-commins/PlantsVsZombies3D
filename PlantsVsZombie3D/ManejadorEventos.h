#ifndef MANEJADOREVENTOS_H
#define MANEJADOREVENTOS_H

void callbackDisplay();
void callbackReshape(int w, int h);
void callbackTimer(int v);
void callbackTeclado(unsigned char key, int x, int y);
void callbackMouse(int button, int state, int x, int y);

#endif
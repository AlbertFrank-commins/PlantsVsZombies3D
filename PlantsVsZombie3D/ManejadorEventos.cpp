#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "LogicaJuego.h"
#include "Render.h"

void callbackDisplay() {
    if (g_Logica == nullptr) return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Ángulo de cámara elevado fijado por tus especificaciones del Módulo 3
    gluLookAt(9.0f, 10.0f, 16.0f,
        9.0f, 0.0f, 5.0f,
        0.0f, 1.0f, 0.0f);

    // 1. Renderizar el fondo (Llama a la función exacta de tu compañero 2)
    DibujarTablero();

    // 2. Renderizar Plantas en sus celdas mediante traducción de matrices
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            Planta* planta = g_Logica->obtenerPlanta(f, c);
            if (planta != nullptr) {
                glPushMatrix();
                // Ajustamos el escalado: cada baldosa mide 1.0f de ancho en Render.cpp, centramos en la celda
                glTranslatef(c * 1.0f + 0.5f, 0.0f, f * 1.0f + 0.5f);
                DibujarPlanta(*planta);
                glPopMatrix();
            }
        }
    }

    // 3. Renderizar Vector de Zombies
    for (const auto& zombie : g_Logica->listaZombies) {
        glPushMatrix();
        // Escalamos la posición X lógica a la relación visual de baldosas de 1.0f
        float visualX = (zombie.posicionX / 2.0f) * 1.0f;
        glTranslatef(visualX, 0.0f, zombie.fila * 1.0f + 0.5f);
        DibujarZombie(zombie);
        glPopMatrix();
    }

    // 4. Renderizar Vector de Proyectiles
    for (const auto& proyectil : g_Logica->listaProyectiles) {
        glPushMatrix();
        float visualProjX = (proyectil.posicionX / 2.0f) * 1.0f;
        glTranslatef(visualProjX, 0.0f, proyectil.fila * 1.0f + 0.5f);

        // Adaptación de escala visual solicitada por el módulo de gráficos
        glScalef(proyectil.escala, proyectil.escala, proyectil.escala);
        ModeloGuisante();
        glPopMatrix();
    }

    glutSwapBuffers();
}

void callbackTimer(int v) {
    // Calculamos un deltaTime aproximado constante de simulación física a 60 FPS (16ms = ~0.016s)
    if (g_Logica != nullptr) {
        g_Logica->actualizar(0.016f);
    }
    glutPostRedisplay();
    glutTimerFunc(16, callbackTimer, 0);
}

void callbackReshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
}

void callbackTeclado(unsigned char key, int x, int y) {
    if (key == 27) exit(0);
}
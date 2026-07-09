#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "../1_Core_Logica_h/Estructuras.h"
#include "../2_Graficos_Estetica_h/Render.h"

// Enlazamos los arreglos globales que controlará el Integrante 1 (Lógica)
extern Zombie listaZombies[10];
extern Guisante listaGuisantes[20];

// Enlazamos las funciones de actualización del backend lógico
void actualizarPosiciones();
void verificarColisionesLogicas();

void callbackDisplay() {
    // 1. Limpiamos los buffers de color y el Z-Buffer de profundidad (Clave del Módulo 3)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Activamos la matriz de vista del modelo para posicionar la cámara
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 3. Configuración de la cámara fija (Estilo perspectiva isométrica elevada)
    gluLookAt(9.0f, 10.0f, 16.0f,  // Ubicación de la cámara en el aire
        9.0f, 0.0f, 5.0f,    // Hacia dónde apunta (Centro del jardín)
        0.0f, 1.0f, 0.0f);   // El eje Y representa la vertical hacia arriba

    // 4. Pintamos el fondo estático (El patio de la Persona 2)
    dibujarPatio();

    // 5. RENDERIZADO JERÁRQUICO DE ZOMBIES (Tu responsabilidad principal)
    for (int i = 0; i < 10; i++) {
        if (listaZombies[i].activo) {
            glPushMatrix(); // <-- GUARDAMOS EL ESTADO DEL MUNDO LIMPIO
            // Trasladamos el origen al X lógico y calculamos el Z multiplicando la fila por 2.0
            glTranslatef(listaZombies[i].posX, 0.6f, (listaZombies[i].fila * 2.0f) + 1.0f);

            // Si el zombie está caminando, puedes agregar una pequeña rotación estética
            if (listaZombies[i].estado == CAMINANDO) {
                // Simula un balanceo usando el tiempo o una pequeña inclinación estática
                glRotatef(5.0f, 0.0f, 0.0f, 1.0f);
            }

            // Llamamos a la función de modelado del Integrante 2
            dibujarZombie();
            glPopMatrix(); // <-- RESTAURAMOS EL MUNDO PARA EL SIGUIENTE OBJETO
        }
    }

    // 6. RENDERIZADO DE PROYECTILES (Guisantes)
    for (int i = 0; i < 20; i++) {
        if (listaGuisantes[i].activo) {
            glPushMatrix();
            glTranslatef(listaGuisantes[i].posX, 0.6f, (listaGuisantes[i].fila * 2.0f) + 1.0f);
            dibujarGuisante();
            glPopMatrix();
        }
    }

    // Intercambiamos los buffers de dibujo para una animación fluida
    glutSwapBuffers();
}

void callbackTimer(int v) {
    // El reloj síncrono del juego: se ejecuta de forma continua cada 16 milisegundos (~60 FPS)

    // Llamamos al Integrante 1 para que mueva las variables lógicas y procese los daños
    actualizarPosiciones();
    verificarColisionesLogicas();

    // Forzamos a OpenGL a redibujar la pantalla con las nuevas posiciones
    glutPostRedisplay();

    // Reenganchamos el temporizador de FreeGLUT
    glutTimerFunc(16, callbackTimer, 0);
}

void callbackReshape(int w, int h) {
    // Si la ventana cambia de tamaño, ajustamos la visualización y la lente de proyección
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 1.0, 100.0);
}

void callbackTeclado(unsigned char key, int x, int y) {
    // Captura de eventos del teclado
    if (key == 27) { // 27 es el código ASCII de la tecla ESCAPE
        exit(0);     // Cierra la aplicación de forma limpia
    }
}
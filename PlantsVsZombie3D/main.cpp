#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "Render.h"
#include "LogicaJuego.h"

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Plants vs Zombies 2D - Unificado");

    // Instanciamos el motor de la logica orientada a objetos en la memoria global
    g_Logica = new LogicaJuego();

    // Inicializaciones de hardware requeridas en el Modulo 2
    ConfigurarIluminacion();

    // Asignacion de callbacks del Modulo 3
    glutDisplayFunc(callbackDisplay);
    glutReshapeFunc(callbackReshape);
    glutKeyboardFunc(callbackTeclado);
    glutMouseFunc(callbackMouse); // clic para seleccionar semilla / plantar
    glutTimerFunc(16, callbackTimer, 0);

    // Cielo claro de patio trasero en vez del fondo oscuro original:
    // encaja mejor con la ambientacion 2D de casa + cesped + arbustos.
    glClearColor(0.55f, 0.75f, 0.95f, 1.0f);

    glutMainLoop();

    delete g_Logica; // Liberacion limpia de memoria al cerrar
    return 0;
}
#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "Render.h"
#include "LogicaJuego.h"

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Plants vs Zombies 3D - Unificado");

    // Instanciamos el motor de la lógica orientada a objetos en la memoria global
    g_Logica = new LogicaJuego();

    // Inicializaciones de hardware requeridas en el Módulo 2[cite: 1]
    ConfigurarIluminacion();

    // Asignación de callbacks del Módulo 3[cite: 1]
    glutDisplayFunc(callbackDisplay);
    glutReshapeFunc(callbackReshape);
    glutKeyboardFunc(callbackTeclado);
    glutTimerFunc(16, callbackTimer, 0);

    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    glutMainLoop();

    delete g_Logica; // Liberación limpia de memoria al cerrar
    return 0;
}
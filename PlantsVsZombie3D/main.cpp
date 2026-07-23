#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "Render.h"
#include "LogicaJuego.h"
#include "GestorSprites.h"
#include "GestorAudio.h"

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Plants vs Zombies 2D - Unificado");

    g_Logica = new LogicaJuego(); // arranca en JUEGO_MENU (ver constructor)

    ConfigurarIluminacion();

    // NUEVO: inicializar los gestores de sprites y audio antes de
    // que cualquier callback intente usarlos.
    GestorSprites::Inicializar();
    if (!GestorAudio::Inicializar()) {
        // Si el audio no pudo inicializarse (por ejemplo, no hay
        // dispositivo de sonido en la maquina) el juego sigue
        // funcionando igual, solo que en silencio.
    }

    glutDisplayFunc(callbackDisplay);
    glutReshapeFunc(callbackReshape);
    glutKeyboardFunc(callbackTeclado);
    glutMouseFunc(callbackMouse);
    glutTimerFunc(16, callbackTimer, 0);

    glClearColor(0.55f, 0.75f, 0.95f, 1.0f);
    glutMainLoop();

    GestorSprites::Liberar();
    GestorAudio::Liberar();
    delete g_Logica;
    return 0;
}
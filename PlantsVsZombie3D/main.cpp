#include <GL/glut.h>
#include "3_Control_Escena_h/ManejadorEventos.h
#include "2_Graficos_Estetica_h/Render.h"

// Enlazamos el inicializador lógico del Integrante 1
void inicializarJuego();

int main(int argc, char** argv) {
    // Inicialización del sistema base de FreeGLUT
    glutInit(&argc, argv);

    // Configuración de Buffers: Doble buffer, Modo de color RGB y Z-Buffer (Profundidad tridimensional)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Plants vs Zombies 3D - Modulo 3 Control");

    // Encendemos las capacidades de la GPU requeridas en la guía
    glEnable(GL_DEPTH_TEST);            // Activa el algoritmo del buffer de profundidad
    configurarIluminacionyMateriales(); // Llama a la configuración lumínica del Integrante 2
    inicializarJuego();                 // Prepara la memoria del Integrante 1 limpia a ceros

    // Registro formal de tus Callbacks en el bucle del sistema operativo
    glutDisplayFunc(callbackDisplay);
    glutReshapeFunc(callbackReshape);
    glutKeyboardFunc(callbackTeclado);
    glutTimerFunc(16, callbackTimer, 0); // Dispara el primer ciclo del reloj continuo

    // Color de limpieza de pantalla: Un cielo azul oscuro nocturno
    glClearColor(0.02f, 0.02f, 0.08f, 1.0f);

    // Entramos al bucle infinito de renderizado de FreeGLUT
    glutMainLoop();
    return 0;
}
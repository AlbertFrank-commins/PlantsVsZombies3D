#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "LogicaJuego.h"
#include "Render.h"
#include <cmath>

// ============================================================
// ManejadorEventos.cpp
// Modulo 3: Control de Escena (3_Control_Escena)
// -----------------------------------------------------------
// Ademas de los callbacks de ventana/tiempo/teclado, este archivo
// resuelve la interaccion con el mouse: semillas del HUD, soles
// caidos y celdas del tablero. La logica de negocio (soles,
// validaciones) sigue viviendo exclusivamente en LogicaJuego; aqui
// solo traducimos coordenadas de pantalla a datos del mundo 3D.
// ============================================================

// Guardamos el tamano de ventana actual para poder construir la
// proyeccion ortografica del HUD (Render.cpp) con las dimensiones
// correctas en cada frame.
static int g_AnchoVentana = 1280;
static int g_AltoVentana = 720;

// Tipo de planta que el jugador tiene seleccionado en la mano;
// NINGUNA significa que no hay nada seleccionado todavia.
static TipoPlanta g_TipoSeleccionado = NINGUNA;

void callbackDisplay() {
    if (g_Logica == nullptr) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camara elevada y fija, combinada con la proyeccion ortografica
    // de callbackReshape: logra la lectura "plana" de un juego 2D
    // sin perder el volumen ni la iluminacion de los modelos 3D de
    // plantas y zombies (ver nota de diseno en Render.h).
    gluLookAt(CENTRO_TABLERO_X, 12.0f, CENTRO_TABLERO_Z + 11.0f,
        CENTRO_TABLERO_X, 0.0f, CENTRO_TABLERO_Z,
        0.0f, 1.0f, 0.0f);

    // 1. Entorno: cesped + casa + arbustos + podadoras (Modulo 2)
    DibujarTablero();
    DibujarEscenario();

    // 2. Renderizar Plantas en sus celdas mediante traduccion de matrices
    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 9; c++) {
            Planta* planta = g_Logica->obtenerPlanta(f, c);
            if (planta != nullptr) {
                glPushMatrix();
                // Cada baldosa mide 1.0f de ancho en Render.cpp; centramos en la celda
                glTranslatef(c * 1.0f + 0.5f, 0.0f, f * 1.0f + 0.5f);
                DibujarPlanta(*planta);
                glPopMatrix();
            }
        }
    }

    // 3. Renderizar Vector de Zombies
    for (const auto& zombie : g_Logica->listaZombies) {
        glPushMatrix();
        // Escalamos la posicion X logica a la relacion visual de baldosas de 1.0f
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
        // Adaptacion de escala visual solicitada por el modulo de graficos
        glScalef(proyectil.escala, proyectil.escala, proyectil.escala);
        ModeloGuisante();
        glPopMatrix();
    }

    // 5. Renderizar soles caidos (x,z ya estan en el mismo sistema de
    // coordenadas que las celdas, por eso no necesitan conversion)
    for (const auto& sol : g_Logica->listaSoles) {
        glPushMatrix();
        glTranslatef(sol.x, sol.y, sol.z);
        DibujarSol(sol);
        glPopMatrix();
    }

    // 6. Interfaz 2D superpuesta: panel de soles + semillas seleccionables
    DibujarInterfaz(g_AnchoVentana, g_AltoVentana, g_Logica->soles, g_TipoSeleccionado);

    glutSwapBuffers();
}

void callbackTimer(int v) {
    // Calculamos un deltaTime aproximado constante de simulacion fisica a 60 FPS (16ms = ~0.016s)
    if (g_Logica != nullptr) {
        g_Logica->actualizar(0.016f);
    }
    glutPostRedisplay();
    glutTimerFunc(16, callbackTimer, 0);
}

void callbackReshape(int w, int h) {
    if (h == 0) h = 1;
    g_AnchoVentana = w;
    g_AltoVentana = h;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Proyeccion ORTOGRAFICA en lugar de gluPerspective: quita la
    // distorsion de perspectiva y da la lectura "plana" de un juego 2D.
    //
    // mitadAltura controla el "zoom": mientras mas chico, mas grande
    // se ve el escenario. Se calculo para que la casa (borde
    // izquierdo) y los arbustos (borde derecho) sigan cabiendo
    // siempre dentro de la ventana, sin recortarse.
    float aspecto = (float)w / (float)h;
    const float mitadAltura = 5.0f;
    float mitadAncho = mitadAltura * aspecto;

    // Desplazamos el encuadre verticalmente para aprovechar el
    // espacio vacio que quedaba debajo del tablero (la vista
    // original dejaba mucho cielo abajo y poco arriba).
    const float desplazamientoVertical = 1.3f;
    glOrtho(-mitadAncho, mitadAncho,
        -mitadAltura + desplazamientoVertical, mitadAltura + desplazamientoVertical,
        -50.0, 50.0);

    glMatrixMode(GL_MODELVIEW);
}

void callbackTeclado(unsigned char key, int x, int y) {
    switch (key) {
    case 27: exit(0); break;                              // ESC
    case '1': g_TipoSeleccionado = LANZAGUISANTES; break;  // atajos de teclado opcionales,
    case '2': g_TipoSeleccionado = GIRASOL; break;         // equivalentes a hacer clic
    case '3': g_TipoSeleccionado = NUEZ; break;            // sobre la semilla del HUD
    case '4': g_TipoSeleccionado = CEREZA_EXPLOSIVA; break;
    default: break;
    }
}

// Convierte un clic de pantalla (x, y en pixeles) en una celda del
// tablero (fila, columna), intersectando el rayo de camara con el
// plano y = 0 sobre el que se apoya el cesped. Funciona igual con
// proyeccion ortografica que con perspectiva: gluUnProject solo
// necesita las matrices actuales, sin importar cual sea.
static bool ResolverCeldaClic(int screenX, int screenY, int& filaOut, int& columnaOut)
{
    GLint viewport[4];
    GLdouble modelview[16], projection[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    // GLUT entrega y con origen arriba; OpenGL espera origen abajo
    double winY = (double)(viewport[3] - screenY - 1);

    double x0, y0, z0, x1, y1, z1;
    gluUnProject(screenX, winY, 0.0, modelview, projection, viewport, &x0, &y0, &z0);
    gluUnProject(screenX, winY, 1.0, modelview, projection, viewport, &x1, &y1, &z1);

    double dx = x1 - x0, dy = y1 - y0, dz = z1 - z0;
    if (fabs(dy) < 1e-6) return false; // rayo paralelo al plano del tablero

    double t = -y0 / dy;
    double mundoX = x0 + t * dx;
    double mundoZ = z0 + t * dz;

    int columna = (int)floor(mundoX / 1.0);
    int fila = (int)floor(mundoZ / 1.0);
    if (fila < 0 || fila >= FILAS_TABLERO || columna < 0 || columna >= COLUMNAS_TABLERO) return false;

    filaOut = fila;
    columnaOut = columna;
    return true;
}

// Busca si el clic (x, y en pixeles) cayo sobre algun sol caido,
// proyectando la posicion 3D de cada sol a coordenadas de pantalla
// con gluProject y comparando la distancia contra un radio de
// tolerancia en pixeles. A diferencia de ResolverCeldaClic, aqui no
// alcanza con intersectar el plano y = 0 porque el sol puede estar
// flotando a varios metros de altura mientras cae.
static bool ResolverClicSol(int screenX, int screenY, int& indiceOut)
{
    if (g_Logica == nullptr || g_Logica->listaSoles.empty()) return false;

    GLint viewport[4];
    GLdouble modelview[16], projection[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    double winYClic = (double)(viewport[3] - screenY - 1);
    const double RADIO_CLIC_PX = 26.0;

    for (int i = 0; i < (int)g_Logica->listaSoles.size(); i++) {
        const SolCaido& sol = g_Logica->listaSoles[i];
        double sx, sy, sz;
        gluProject(sol.x, sol.y, sol.z, modelview, projection, viewport, &sx, &sy, &sz);
        double dx = sx - screenX;
        double dy = sy - winYClic;
        if (dx * dx + dy * dy <= RADIO_CLIC_PX * RADIO_CLIC_PX) {
            indiceOut = i;
            return true;
        }
    }
    return false;
}

void callbackMouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;
    if (g_Logica == nullptr) return;

    // Clic derecho: "desplantar" la celda bajo el cursor. Reutiliza
    // el mismo picking que el plantado (ResolverCeldaClic) y devuelve
    // al jugador exactamente lo que le costo esa planta.
    if (button == GLUT_RIGHT_BUTTON) {
        int fila, columna;
        if (ResolverCeldaClic(x, y, fila, columna)) {
            g_Logica->quitarPlanta(fila, columna);
        }
        return;
    }

    if (button != GLUT_LEFT_BUTTON) return;

    // 1) ¿El clic cayo sobre una semilla del HUD? -> se selecciona esa planta.
    TipoPlanta elegido = SemillaEnPosicion(x, y, g_AnchoVentana);
    if (elegido != NINGUNA) {
        g_TipoSeleccionado = elegido;
        return;
    }

    // 2) ¿El clic cayo sobre un sol caido? -> se recolecta, sin
    // importar si hay o no una semilla seleccionada en la mano.
    int indiceSol;
    if (ResolverClicSol(x, y, indiceSol)) {
        g_Logica->recolectarSol(indiceSol);
        return;
    }

    // 3) Sin semilla en mano no hay nada mas que hacer con el clic.
    if (g_TipoSeleccionado == NINGUNA) return;

    // 4) Clic sobre el tablero: intentamos plantar en la celda correspondiente.
    int fila, columna;
    if (ResolverCeldaClic(x, y, fila, columna)) {
        if (g_Logica->plantar(fila, columna, g_TipoSeleccionado)) {
            g_TipoSeleccionado = NINGUNA; // la semilla se consume tras plantar
        }
    }
}
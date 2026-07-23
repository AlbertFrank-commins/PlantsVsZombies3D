#include <GL/glut.h>
#include "ManejadorEventos.h"
#include "LogicaJuego.h"
#include "Render.h"
#include "GestorAudio.h"
#include <cmath>
#include "GestorSprites.h"

// ============================================================
// ManejadorEventos.cpp
// Modulo 3: Control de Escena
// -----------------------------------------------------------
// Ademas de los callbacks de ventana/tiempo/teclado, este
// archivo ahora:
//  1) Atiende clics del MENU (Jugar/Salir) cuando
//     g_Logica->estadoJuego == JUEGO_MENU.
//  2) Lee la cola de eventos de sonido de LogicaJuego cada
//     frame y llama a GestorAudio (LogicaJuego no conoce audio).
//  3) Atiende el clic para volver al menu en Game Over.
// ============================================================

static int g_AnchoVentana = 1280;
static int g_AltoVentana = 720;
static TipoPlanta g_TipoSeleccionado = NINGUNA;
static float g_TiempoAnimacion = 0.0f; // NUEVO: reloj para animar menu/escenario

// ---------------- Resolver clics del tablero/soles ----------------
// NUEVO: version 2D pura. Como toda la escena se dibuja en una
// proyeccion ortografica de pixeles (glOrtho(0,w,h,0,-1,1) con
// origen arriba-izquierda), las coordenadas de mouse que entrega
// GLUT (tambien origen arriba-izquierda) coinciden 1:1 con las
// coordenadas de pantalla que usamos para dibujar. Ya no hace
// falta gluUnProject/gluProject ni ninguna matriz.
static bool ResolverCeldaClic(int screenX, int screenY, int& filaOut, int& columnaOut)
{
    if (screenX < TABLERO_ORIGEN_X || screenY < TABLERO_ORIGEN_Y) return false;
    int columna = (screenX - TABLERO_ORIGEN_X) / CELDA_ANCHO_PX;
    int fila = (screenY - TABLERO_ORIGEN_Y) / CELDA_ALTO_PX;
    if (fila < 0 || fila >= FILAS_TABLERO || columna < 0 || columna >= COLUMNAS_TABLERO) return false;
    filaOut = fila;
    columnaOut = columna;
    return true;
}

static bool ResolverClicSol(int screenX, int screenY, int& indiceOut)
{
    if (g_Logica == nullptr || g_Logica->listaSoles.empty()) return false;
    const float RADIO_CLIC_PX = 30.0f;
    for (int i = 0; i < (int)g_Logica->listaSoles.size(); i++) {
        const SolCaido& sol = g_Logica->listaSoles[i];
        // Misma formula que usa DibujarSol en callbackDisplay: el sol
        // cae desde arriba (sol.y alto) hasta quedar en el suelo.
        float cx = ColumnaAPixelX(sol.x);
        float cy = FilaAPixelY(sol.z) + CELDA_ALTO_PX * 0.5f - (sol.y - SOL_ALTURA_SUELO) * 40.0f;
        float dx = cx - (float)screenX;
        float dy = cy - (float)screenY;
        if (dx * dx + dy * dy <= RADIO_CLIC_PX * RADIO_CLIC_PX) {
            indiceOut = i;
            return true;
        }
    }
    return false;
}

// NUEVO: helper para saber si (x,y) cae dentro de un rectangulo de pantalla
static bool DentroDeRectangulo(int x, int y, int x0, int y0, int x1, int y1)
{
    return (x >= x0 && x <= x1 && y >= y0 && y <= y1);
}

void callbackDisplay() {
    if (g_Logica == nullptr) return;
    glClear(GL_COLOR_BUFFER_BIT);

    // ---------------- MENU: no se dibuja el tablero, solo el menu ----------------
    if (g_Logica->estadoJuego == JUEGO_MENU) {
        DibujarMenu(g_AnchoVentana, g_AltoVentana, g_TiempoAnimacion);
        glutSwapBuffers();
        return;
    }

    // NUEVO: todo en 2D puro. Ya no hay camara ni glTranslatef en
    // 3D: cada entidad calcula su propia posicion en PIXELES a
    // partir de fila/columna (o posicionX/x/z de LogicaJuego) y se
    // la pasa directo a la funcion DibujarX correspondiente.
    DibujarFondoCielo(g_AnchoVentana, g_AltoVentana);
    DibujarEscenario(g_TiempoAnimacion);
    DibujarTablero();

    // Las plantas se dibujan fila por fila (de arriba hacia abajo)
    // para que, si alguna vez se superponen visualmente, la fila de
    // mas abajo quede encima (orden "pintor" clasico en 2D).
    for (int f = 0; f < FILAS_TABLERO; f++) {
        for (int c = 0; c < COLUMNAS_TABLERO; c++) {
            Planta* planta = g_Logica->obtenerPlanta(f, c);
            if (planta != nullptr) {
                float cx = CentroColumnaPixelX(c);
                float piesY = BasePixelY(f);
                DibujarPlanta(*planta, cx, piesY);
            }
        }
    }
    for (const auto& zombie : g_Logica->listaZombies) {
        float columnaEquivalente = zombie.posicionX / ANCHO_CELDA_LOGICO;
        float cx = ColumnaAPixelX(columnaEquivalente);
        float piesY = BasePixelY(zombie.fila);
        DibujarZombie(zombie, cx, piesY);
    }
    for (const auto& proyectil : g_Logica->listaProyectiles) {
        float columnaEquivalente = proyectil.posicionX / ANCHO_CELDA_LOGICO;
        float cx = ColumnaAPixelX(columnaEquivalente);
        float cy = FilaAPixelY(proyectil.fila) + CELDA_ALTO_PX * 0.5f;
        DibujarProyectil(proyectil, cx, cy);
    }
    for (const auto& carretilla : g_Logica->listaCarretillas) {
        float cx = ColumnaAPixelX(carretilla.posicionX / ANCHO_CELDA_LOGICO);
        float piesY = BasePixelY(carretilla.fila);
        DibujarCarretilla(carretilla, cx, piesY);
    }
    for (const auto& sol : g_Logica->listaSoles) {
        float cx = ColumnaAPixelX(sol.x);
        // sol.y baja desde SOL_ALTURA_INICIAL hasta SOL_ALTURA_SUELO:
        // lo usamos para que el sol se vea cayendo desde el cielo.
        float cy = FilaAPixelY(sol.z) + CELDA_ALTO_PX * 0.5f - (sol.y - SOL_ALTURA_SUELO) * 40.0f;
        DibujarSol(sol, cx, cy);
    }

    DibujarInterfaz(g_AnchoVentana, g_AltoVentana, g_Logica->soles, g_TipoSeleccionado);

    // ---------------- GAME OVER: se dibuja encima, como overlay ----------------
    if (g_Logica->estadoJuego == JUEGO_GAME_OVER) {
        DibujarGameOver(g_AnchoVentana, g_AltoVentana);
    }

    glutSwapBuffers();
}

void callbackTimer(int v) {
    g_TiempoAnimacion += 0.016f; // NUEVO: reloj de animacion (menu/escenario)
    if (g_Logica != nullptr) {
        g_Logica->actualizar(0.016f);

        // NUEVO: procesar la cola de eventos de sonido que encolo
        // LogicaJuego durante este frame (plantar, disparo, etc.)
        for (const auto& evento : g_Logica->eventosSonido) {
            GestorAudio::ReproducirEventoLogica(evento);
        }
        g_Logica->VaciarEventosSonido();
    }
    glutPostRedisplay();
    glutTimerFunc(16, callbackTimer, 0);
}

void callbackReshape(int w, int h) {
    if (h == 0) h = 1;
    g_AnchoVentana = w;
    g_AltoVentana = h;
    glViewport(0, 0, w, h);
    // NUEVO: proyeccion ortografica en PIXELES de pantalla, con
    // origen arriba-izquierda (igual que las coordenadas de mouse
    // de GLUT). Es la unica proyeccion de todo el juego: no hay
    // perspectiva ni camara, por eso ya no depende del aspect ratio.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)w, (double)h, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void callbackTeclado(unsigned char key, int x, int y) {
    if (g_Logica != nullptr && g_Logica->estadoJuego == JUEGO_JUGANDO) {
        switch (key) {
        case 27: exit(0); break;
        case '1': g_TipoSeleccionado = LANZAGUISANTES; break;
        case '2': g_TipoSeleccionado = GIRASOL; break;
        case '3': g_TipoSeleccionado = NUEZ; break;
        case '4': g_TipoSeleccionado = CEREZA_EXPLOSIVA; break;
        case 'p': case 'P':
            g_Logica->estadoJuego = JUEGO_PAUSA; // NUEVO: pausa rapida
            break;
        default: break;
        }
    }
    else if (key == 27) {
        exit(0);
    }
}

void callbackMouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;
    if (g_Logica == nullptr) return;
    if (button != GLUT_LEFT_BUTTON && button != GLUT_RIGHT_BUTTON) return;

    // ---------------- MENU: solo interesan los botones ----------------
    if (g_Logica->estadoJuego == JUEGO_MENU) {
        if (button != GLUT_LEFT_BUTTON) return;
        int x0, y0, x1, y1;
        RectanguloBotonJugar(g_AnchoVentana, g_AltoVentana, x0, y0, x1, y1);
        if (DentroDeRectangulo(x, y, x0, y0, x1, y1)) {
            g_Logica->ReiniciarJuego(); // deja estadoJuego = JUEGO_JUGANDO
            GestorAudio::ReproducirMusica("musica_juego.wav");
            return;
        }
        RectanguloBotonSalir(g_AnchoVentana, g_AltoVentana, x0, y0, x1, y1);
        if (DentroDeRectangulo(x, y, x0, y0, x1, y1)) {
            exit(0);
        }
        return;
    }

    // ---------------- GAME OVER: cualquier clic vuelve al menu ----------------
    if (g_Logica->estadoJuego == JUEGO_GAME_OVER) {
        if (button == GLUT_LEFT_BUTTON) {
            g_Logica->estadoJuego = JUEGO_MENU;
            g_TipoSeleccionado = NINGUNA;
        }
        return;
    }

    if (g_Logica->estadoJuego != JUEGO_JUGANDO) return;

    // ---------------- Igual que antes: clic derecho desplanta ----------------
    if (button == GLUT_RIGHT_BUTTON) {
        int fila, columna;
        if (ResolverCeldaClic(x, y, fila, columna)) {
            g_Logica->quitarPlanta(fila, columna);
        }
        return;
    }

    TipoPlanta elegido = SemillaEnPosicion(x, y, g_AnchoVentana);
    if (elegido != NINGUNA) {
        g_TipoSeleccionado = elegido;
        return;
    }

    int indiceSol;
    if (ResolverClicSol(x, y, indiceSol)) {
        g_Logica->recolectarSol(indiceSol);
        return;
    }

    if (g_TipoSeleccionado == NINGUNA) return;

    int fila, columna;
    if (ResolverCeldaClic(x, y, fila, columna)) {
        if (g_Logica->plantar(fila, columna, g_TipoSeleccionado)) {
            g_TipoSeleccionado = NINGUNA;
        }
    }
}
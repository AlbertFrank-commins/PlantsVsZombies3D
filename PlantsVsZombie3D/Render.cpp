#include <GL/glut.h>
#include "Render.h"
#include "GestorSprites.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

// ============================================================
// Render.cpp
// -----------------------------------------------------------
// VERSION 2D PURA. Ya no existe camara (gluLookAt), ni
// proyeccion en perspectiva, ni iluminacion, ni primitivas
// solidas (glutSolidCube/glutSolidSphere): todo el juego se ve
// y se dibuja como una escena plana, con la ventana entera en
// una unica proyeccion ortografica de pixeles (glOrtho(0, w, h,
// 0, -1, 1), configurada una sola vez en ManejadorEventos.cpp).
//
// El escenario de fondo (cesped, casa, camino, valla, arbustos,
// nubes) se dibuja con rectangulos/circulos 2D ubicados a mano
// para imitar el layout del PvZ original (casa a la izquierda,
// cesped en el centro, seto a la derecha). Las entidades
// jugables (plantas, zombies, proyectiles, sol) se dibujan como
// sprites PNG a traves de GestorSprites, ancladas por los pies
// (plantas/zombies) o por el centro (proyectiles/sol).
// ============================================================

// ---------- Helpers 2D basicos (usados por todo el archivo) ----------
static void DibujarRectangulo(float x0, float y0, float x1, float y1, const float color[3])
{
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}
// NUEVO: igual que DibujarRectangulo pero con degradado vertical
// (colorArriba en y0, colorAbajo en y1). Se usa para el cielo del
// menu, imitando el degradado vivo del arte original de PvZ.
static void DibujarRectanguloGradiente(float x0, float y0, float x1, float y1,
    const float colorArriba[3], const float colorAbajo[3])
{
    glBegin(GL_QUADS);
    glColor3f(colorArriba[0], colorArriba[1], colorArriba[2]); glVertex2f(x0, y0);
    glColor3f(colorArriba[0], colorArriba[1], colorArriba[2]); glVertex2f(x1, y0);
    glColor3f(colorAbajo[0], colorAbajo[1], colorAbajo[2]); glVertex2f(x1, y1);
    glColor3f(colorAbajo[0], colorAbajo[1], colorAbajo[2]); glVertex2f(x0, y1);
    glEnd();
}
// NUEVO: contorno marcado (estilo retro/pixel-art), grosor configurable.
static void DibujarContorno(float x0, float y0, float x1, float y1, float grosor, const float color[3])
{
    glColor3f(color[0], color[1], color[2]);
    glLineWidth(grosor);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x0 + 0.5f, y0 + 0.5f);
    glVertex2f(x1 - 0.5f, y0 + 0.5f);
    glVertex2f(x1 - 0.5f, y1 - 0.5f);
    glVertex2f(x0 + 0.5f, y1 - 0.5f);
    glEnd();
    glLineWidth(1.0f);
}
static void DibujarTriangulo(float x0, float y0, float x1, float y1, float x2, float y2, const float color[3])
{
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_TRIANGLES);
    glVertex2f(x0, y0); glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}
static void DibujarCirculo2D(float cx, float cy, float radio, const float color[3])
{
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 20; i++) {
        float ang = (float)i / 20.0f * 6.2831853f;
        glVertex2f(cx + cosf(ang) * radio, cy + sinf(ang) * radio);
    }
    glEnd();
}
static void DibujarTexto(float x, float y, const char* texto, void* fuente)
{
    glRasterPos2f(x, y);
    for (const char* c = texto; *c != '\0'; c++) glutBitmapCharacter(fuente, *c);
}

// Ya no hay luces que configurar: solo nos aseguramos de que el
// estado de OpenGL quede en modo "2D puro" (nada de depth test ni
// lighting, que eran cosas del render 3D anterior).
void ConfigurarIluminacion()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

// ============================================================
// Cesped jugable: 5 filas x 9 columnas, cada celda un rectangulo
// fijo en pixeles (ver TABLERO_ORIGEN_X/Y y CELDA_ANCHO/ALTO_PX
// en Estructuras.h).
// ============================================================
void DibujarTablero()
{
    float verdeClaro[] = { 0.42f, 0.72f, 0.30f };
    float verdeOscuro[] = { 0.34f, 0.62f, 0.24f };
    for (int f = 0; f < FILAS_TABLERO; f++) {
        for (int c = 0; c < COLUMNAS_TABLERO; c++) {
            float x0 = ColumnaAPixelX((float)c);
            float y0 = FilaAPixelY((float)f);
            bool esClaro = ((f + c) % 2 == 0);
            DibujarRectangulo(x0, y0, x0 + CELDA_ANCHO_PX, y0 + CELDA_ALTO_PX,
                esClaro ? verdeClaro : verdeOscuro);
            float lineaSutil[] = { 0.0f, 0.0f, 0.0f };
            glColor4f(lineaSutil[0], lineaSutil[1], lineaSutil[2], 0.12f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x0, y0); glVertex2f(x0 + CELDA_ANCHO_PX, y0);
            glVertex2f(x0 + CELDA_ANCHO_PX, y0 + CELDA_ALTO_PX); glVertex2f(x0, y0 + CELDA_ALTO_PX);
            glEnd();
            glDisable(GL_BLEND);
        }
    }
}

// ---------- Escenario de fondo, ahora en 2D (rectangulos/circulos) ----------
static const float CESPED_X0 = (float)TABLERO_ORIGEN_X;
static const float CESPED_Y0 = (float)TABLERO_ORIGEN_Y;
static const float CESPED_X1 = (float)(TABLERO_ORIGEN_X + COLUMNAS_TABLERO * CELDA_ANCHO_PX);
static const float CESPED_Y1 = (float)(TABLERO_ORIGEN_Y + FILAS_TABLERO * CELDA_ALTO_PX);

static void DibujarCielo(int anchoVentana, int altoVentana)
{
    float cieloArriba[] = { 0.35f, 0.65f, 0.90f };
    float cieloAbajo[] = { 0.72f, 0.85f, 0.92f };
    DibujarRectanguloGradiente(0, 0, (float)anchoVentana, (float)altoVentana, cieloArriba, cieloAbajo);
}

static void DibujarCasa()
{
    float ladrilloClaro[] = { 0.72f, 0.38f, 0.28f };
    float ladrilloOscuro[] = { 0.62f, 0.30f, 0.22f };
    float mortero[] = { 0.80f, 0.72f, 0.60f };
    float tejaRoja[] = { 0.58f, 0.22f, 0.14f };
    float tejaRojaOscura[] = { 0.46f, 0.16f, 0.10f };
    float maderaOscura[] = { 0.32f, 0.19f, 0.09f };
    float maderaClara[] = { 0.45f, 0.28f, 0.14f };
    float vidrioClaro[] = { 0.65f, 0.82f, 0.92f };
    float marcoBlanco[] = { 0.95f, 0.95f, 0.92f };
    float ladrilloChimenea[] = { 0.50f, 0.28f, 0.20f };
    float perillaDorada[] = { 0.85f, 0.70f, 0.25f };
    float piedraPorche[] = { 0.75f, 0.73f, 0.68f };
    float negroContorno[] = { 0.08f, 0.06f, 0.05f };
    float verdeEnredadera[] = { 0.22f, 0.42f, 0.16f };

    float x0 = 10.0f, x1 = CESPED_X0 - 15.0f;
    float y0 = CESPED_Y0 - 60.0f, y1 = CESPED_Y1 + 20.0f;
    float anchoTotal = x1 - x0;
    float cxCasa = (x0 + x1) / 2.0f;

    // Sombra suave al pie de la casa (la asienta sobre el cesped)
    float sombra[] = { 0.30f, 0.45f, 0.20f };
    DibujarRectangulo(x0 + 4.0f, y1 - 6.0f, x1 + 10.0f, y1 + 6.0f, sombra);

    // Muro de MORTERO de base + hiladas de LADRILLO desfasadas (retro brick)
    DibujarRectangulo(x0, y0, x1, y1, mortero);
    float altoLadrillo = 11.0f, anchoLadrillo = 22.0f;
    int fila = 0;
    for (float ly = y0; ly < y1; ly += altoLadrillo, fila++) {
        float desfase = (fila % 2 == 0) ? 0.0f : anchoLadrillo * 0.5f;
        for (float lx = x0 - anchoLadrillo + desfase; lx < x1; lx += anchoLadrillo) {
            float lx0 = lx + 1.0f, lx1 = lx + anchoLadrillo - 1.0f;
            if (lx1 <= x0 || lx0 >= x1) continue;
            if (lx0 < x0) lx0 = x0;
            if (lx1 > x1) lx1 = x1;
            float ly1 = ly + altoLadrillo - 1.0f;
            if (ly1 > y1) ly1 = y1;
            DibujarRectangulo(lx0, ly + 1.0f, lx1, ly1, (fila % 3 == 0) ? ladrilloOscuro : ladrilloClaro);
        }
    }
    // Zocalo mas oscuro
    float zocalo[] = { 0.35f, 0.18f, 0.14f };
    DibujarRectangulo(x0, y1 - 14.0f, x1, y1, zocalo);
    DibujarContorno(x0, y0, x1, y1, 2.0f, negroContorno);

    // Techo a dos aguas (dos triangulos que se solapan) + tejas (lineas)
    DibujarTriangulo(x0 - 14.0f, y0 + 4.0f, cxCasa, y0 - 75.0f, cxCasa, y0 + 4.0f, tejaRojaOscura);
    DibujarTriangulo(cxCasa, y0 + 4.0f, cxCasa, y0 - 75.0f, x1 + 14.0f, y0 + 4.0f, tejaRoja);
    for (int i = 1; i <= 5; i++) {
        float t = i / 6.0f;
        DibujarTriangulo(x0 - 14.0f + t * (anchoTotal / 2.0f + 14.0f), y0 + 4.0f - t * 5.0f,
            cxCasa, y0 - 75.0f + t * 8.0f,
            cxCasa, y0 + 4.0f - t * 5.0f, tejaRojaOscura);
    }

    // Chimenea
    DibujarRectangulo(cxCasa + anchoTotal * 0.18f, y0 - 55.0f, cxCasa + anchoTotal * 0.18f + 16.0f, y0 - 10.0f, ladrilloChimenea);
    DibujarRectangulo(cxCasa + anchoTotal * 0.18f - 3.0f, y0 - 58.0f, cxCasa + anchoTotal * 0.18f + 19.0f, y0 - 50.0f, maderaOscura);

    // Escalon/porche de piedra frente a la puerta
    float puertaAncho = 46.0f, puertaAlto = 82.0f;
    float puertaX0 = cxCasa - puertaAncho / 2.0f;
    DibujarRectangulo(puertaX0 - 16.0f, y1 - 2.0f, puertaX0 + puertaAncho + 16.0f, y1 + 14.0f, piedraPorche);
    DibujarContorno(puertaX0 - 16.0f, y1 - 2.0f, puertaX0 + puertaAncho + 16.0f, y1 + 14.0f, 1.5f, negroContorno);

    // Puerta en ARCO: marco recto + remate curvo arriba (abanico de triangulos)
    DibujarRectangulo(puertaX0 - 5.0f, y1 - puertaAlto - 5.0f, puertaX0 + puertaAncho + 5.0f, y1 + 2.0f, marcoBlanco);
    float archCX = puertaX0 + puertaAncho / 2.0f;
    float archY = y1 - puertaAlto - 5.0f;
    float archR = puertaAncho / 2.0f + 5.0f;
    for (int i = 0; i < 10; i++) {
        float a0 = 3.14159f * (float)i / 10.0f;
        float a1 = 3.14159f * (float)(i + 1) / 10.0f;
        DibujarTriangulo(archCX, archY, archCX + cosf(3.14159f - a0) * archR, archY - sinf(a0) * archR,
            archCX + cosf(3.14159f - a1) * archR, archY - sinf(a1) * archR, marcoBlanco);
    }
    DibujarRectangulo(puertaX0, y1 - puertaAlto, puertaX0 + puertaAncho, y1, maderaOscura);
    DibujarRectangulo(puertaX0 + 6.0f, y1 - puertaAlto + 10.0f, puertaX0 + puertaAncho - 6.0f, y1 - puertaAlto * 0.5f, maderaClara);
    DibujarCirculo2D(puertaX0 + puertaAncho - 10.0f, y1 - puertaAlto * 0.45f, 3.0f, perillaDorada);
    DibujarContorno(puertaX0, y1 - puertaAlto, puertaX0 + puertaAncho, y1, 1.5f, negroContorno);

    // Dos ventanas con marco
    float ventanaAncho = 34.0f, ventanaAlto = 34.0f;
    float ventanaY = y0 + 45.0f;
    float vX1 = x0 + anchoTotal * 0.20f - ventanaAncho / 2.0f;
    float vX2 = x1 - anchoTotal * 0.20f - ventanaAncho / 2.0f;
    for (float vx : { vX1, vX2 }) {
        DibujarRectangulo(vx - 4.0f, ventanaY - 4.0f, vx + ventanaAncho + 4.0f, ventanaY + ventanaAlto + 4.0f, marcoBlanco);
        DibujarRectangulo(vx, ventanaY, vx + ventanaAncho, ventanaY + ventanaAlto, vidrioClaro);
        DibujarRectangulo(vx + ventanaAncho / 2.0f - 1.5f, ventanaY, vx + ventanaAncho / 2.0f + 1.5f, ventanaY + ventanaAlto, marcoBlanco);
        DibujarRectangulo(vx, ventanaY + ventanaAlto / 2.0f - 1.5f, vx + ventanaAncho, ventanaY + ventanaAlto / 2.0f + 1.5f, marcoBlanco);
        DibujarContorno(vx - 4.0f, ventanaY - 4.0f, vx + ventanaAncho + 4.0f, ventanaY + ventanaAlto + 4.0f, 1.5f, negroContorno);
    }

    // Enredadera trepando la esquina (manchas verdes irregulares)
    for (int i = 0; i < 6; i++) {
        float ey = y1 - 20.0f - i * 22.0f;
        DibujarCirculo2D(x0 + 6.0f + (i % 2) * 4.0f, ey, 7.0f - (i * 0.4f), verdeEnredadera);
    }

    // Farol de porche junto a la puerta
    float farolX = puertaX0 + puertaAncho + 14.0f;
    DibujarRectangulo(farolX - 2.0f, y1 - 60.0f, farolX + 2.0f, y1 - 6.0f, maderaOscura);
    float luzFarol[] = { 0.98f, 0.85f, 0.45f };
    DibujarCirculo2D(farolX, y1 - 62.0f, 7.0f, luzFarol);
}

static void DibujarCamino()
{
    float grisPiedra[] = { 0.72f, 0.70f, 0.64f };
    float grisJunta[] = { 0.55f, 0.53f, 0.48f };
    float musgo[] = { 0.30f, 0.42f, 0.20f };
    float negroContorno[] = { 0.08f, 0.06f, 0.05f };
    float y0 = CESPED_Y1;
    float y1 = y0 + 35.0f;
    DibujarRectangulo(0.0f, y0, CESPED_X0, y1, grisJunta);
    float anchoBaldosa = 30.0f;
    for (float bx = 0.0f; bx < CESPED_X0; bx += anchoBaldosa) {
        float bx1 = bx + anchoBaldosa - 3.0f;
        if (bx1 > CESPED_X0 - 2.0f) bx1 = CESPED_X0 - 2.0f;
        DibujarRectangulo(bx + 2.0f, y0 + 2.0f, bx1, y1 - 2.0f, grisPiedra);
        DibujarContorno(bx + 2.0f, y0 + 2.0f, bx1, y1 - 2.0f, 1.0f, negroContorno);
    }
    // Un par de manchas de musgo entre baldosas
    DibujarCirculo2D(CESPED_X0 * 0.3f, y0 + 8.0f, 5.0f, musgo);
    DibujarCirculo2D(CESPED_X0 * 0.65f, y1 - 8.0f, 4.0f, musgo);
}

static void DibujarArbustos()
{
    float verdeArbusto1[] = { 0.18f, 0.42f, 0.14f };
    float verdeArbusto2[] = { 0.24f, 0.52f, 0.17f };
    float verdeBrillo[] = { 0.34f, 0.64f, 0.24f };
    float negroContorno[] = { 0.08f, 0.06f, 0.05f };
    float florRoja[] = { 0.80f, 0.20f, 0.20f };
    float florAmarilla[] = { 0.90f, 0.80f, 0.20f };
    float cxSeto = CESPED_X1 + 40.0f;
    for (int f = 0; f < FILAS_TABLERO; f++) {
        float cy = FilaAPixelY((float)f) + CELDA_ALTO_PX * 0.5f;
        const float* base = (f % 2 == 0) ? verdeArbusto1 : verdeArbusto2;
        // varios circulos superpuestos para dar densidad/volumen
        DibujarCirculo2D(cxSeto, cy + 6.0f, 40.0f, base);
        DibujarCirculo2D(cxSeto + 26.0f, cy - 12.0f, 26.0f, base);
        DibujarCirculo2D(cxSeto - 22.0f, cy - 10.0f, 24.0f, base);
        DibujarCirculo2D(cxSeto + 8.0f, cy - 24.0f, 20.0f, base);
        // brillo arriba, para dar sensacion de volumen
        DibujarCirculo2D(cxSeto - 6.0f, cy - 20.0f, 16.0f, verdeBrillo);
        DibujarContorno(cxSeto - 42.0f, cy - 42.0f, cxSeto + 44.0f, cy + 40.0f, 1.5f, negroContorno);
        // alguna florcita decorativa
        if (f % 2 == 0) DibujarCirculo2D(cxSeto - 12.0f, cy + 14.0f, 3.5f, florRoja);
        else DibujarCirculo2D(cxSeto + 16.0f, cy + 16.0f, 3.5f, florAmarilla);
    }
}

static void DibujarValla()
{
    float blancoValla[] = { 0.92f, 0.92f, 0.88f };
    float sombraValla[] = { 0.40f, 0.55f, 0.28f };
    float negroContorno[] = { 0.08f, 0.06f, 0.05f };
    const float Y_VALLA_BASE = CESPED_Y0 - 4.0f;
    const float ALTO_POSTE = 42.0f;
    // Sombra proyectada sobre el cesped, para que la valla no "flote"
    DibujarRectangulo(CESPED_X0 - 8.0f, Y_VALLA_BASE, CESPED_X1 + 8.0f, Y_VALLA_BASE + 6.0f, sombraValla);
    for (float x = CESPED_X0 - 8.0f; x <= CESPED_X1 + 8.0f; x += 34.0f) {
        DibujarRectangulo(x - 4.0f, Y_VALLA_BASE - ALTO_POSTE, x + 4.0f, Y_VALLA_BASE, blancoValla);
        DibujarContorno(x - 4.0f, Y_VALLA_BASE - ALTO_POSTE, x + 4.0f, Y_VALLA_BASE, 1.0f, negroContorno);
        DibujarTriangulo(x - 6.0f, Y_VALLA_BASE - ALTO_POSTE,
            x + 6.0f, Y_VALLA_BASE - ALTO_POSTE,
            x, Y_VALLA_BASE - ALTO_POSTE - 10.0f, blancoValla);
    }
    DibujarRectangulo(CESPED_X0 - 8.0f, Y_VALLA_BASE - 30.0f, CESPED_X1 + 8.0f, Y_VALLA_BASE - 24.0f, blancoValla);
    DibujarRectangulo(CESPED_X0 - 8.0f, Y_VALLA_BASE - 14.0f, CESPED_X1 + 8.0f, Y_VALLA_BASE - 8.0f, blancoValla);
}

static void DibujarNube(float x, float y)
{
    float blancoNube[] = { 0.98f, 0.98f, 1.00f };
    DibujarCirculo2D(x, y, 26.0f, blancoNube);
    DibujarCirculo2D(x + 28.0f, y + 4.0f, 20.0f, blancoNube);
    DibujarCirculo2D(x - 26.0f, y + 6.0f, 18.0f, blancoNube);
}

void DibujarEscenario(float tiempo)
{
    // Las nubes derivan lentamente hacia la derecha y reaparecen del
    // otro lado (efecto de looping simple con fmod).
    float ancho = CESPED_X1 + 300.0f;
    DibujarNube(fmodf(120.0f + tiempo * 6.0f, ancho), 40.0f);
    DibujarNube(fmodf(650.0f + tiempo * 4.0f, ancho), 55.0f);
    DibujarNube(fmodf(1050.0f + tiempo * 5.0f, ancho), 35.0f);
    DibujarCasa();
    DibujarCamino();
    DibujarValla();
    DibujarArbustos();
}

void DibujarFondoCielo(int anchoVentana, int altoVentana)
{
    DibujarCielo(anchoVentana, altoVentana);
}

// ============================================================
// HUD (interfaz 2D superpuesta): panel de soles y semillas
// ============================================================
struct SemillaHUD { TipoPlanta tipo; };
static const SemillaHUD SEMILLAS[] = {
    { LANZAGUISANTES }, { GIRASOL }, { NUEZ }, { CEREZA_EXPLOSIVA }
};
static const int NUM_SEMILLAS = 4;
static const int HUD_MARGEN = 12;
static const int HUD_PANEL_Y = 8;
static const int HUD_PANEL_ALTO = 78;
static const int HUD_SOL_ANCHO = 78;
static const int HUD_SEMILLA_ANCHO = 66;
static const int HUD_SEMILLA_SEPARACION = 6;

static void RectanguloSemilla(int indice, int& x0, int& y0, int& x1, int& y1)
{
    x0 = HUD_MARGEN + HUD_SOL_ANCHO + 10 + indice * (HUD_SEMILLA_ANCHO + HUD_SEMILLA_SEPARACION);
    y0 = HUD_PANEL_Y;
    x1 = x0 + HUD_SEMILLA_ANCHO;
    y1 = y0 + HUD_PANEL_ALTO;
}
static void DibujarIconoSemilla(TipoPlanta tipo, float cx, float cy)
{
    float verde[] = { 0.20f, 0.65f, 0.20f };
    float verdeOsc[] = { 0.10f, 0.45f, 0.12f };
    float amarillo[] = { 0.95f, 0.80f, 0.15f };
    float marron[] = { 0.45f, 0.30f, 0.15f };
    float rojo[] = { 0.75f, 0.15f, 0.12f };
    switch (tipo) {
    case LANZAGUISANTES:
        DibujarCirculo2D(cx, cy, 15, verde);
        DibujarCirculo2D(cx + 11, cy, 6, verdeOsc);
        break;
    case GIRASOL:
        for (int i = 0; i < 8; i++) {
            float ang = (float)i / 8.0f * 6.2831853f;
            DibujarCirculo2D(cx + cosf(ang) * 13, cy + sinf(ang) * 13, 6, amarillo);
        }
        DibujarCirculo2D(cx, cy, 10, amarillo);
        break;
    case NUEZ:
        DibujarCirculo2D(cx, cy, 15, marron);
        break;
    case CEREZA_EXPLOSIVA:
        DibujarCirculo2D(cx - 6, cy, 10, rojo);
        DibujarCirculo2D(cx + 6, cy, 10, rojo);
        break;
    default: break;
    }
}
void DibujarInterfaz(int anchoVentana, int altoVentana, int soles, TipoPlanta tipoSeleccionado, const float cooldowns[5])
{
    (void)altoVentana;
    float panelMadera[] = { 0.55f, 0.38f, 0.22f };
    DibujarRectangulo((float)HUD_MARGEN, (float)HUD_PANEL_Y,
        (float)(HUD_MARGEN + HUD_SOL_ANCHO), (float)(HUD_PANEL_Y + HUD_PANEL_ALTO), panelMadera);
    float amarilloSol[] = { 1.0f, 0.85f, 0.15f };
    DibujarCirculo2D((float)HUD_MARGEN + HUD_SOL_ANCHO / 2.0f, (float)HUD_PANEL_Y + 26, 17, amarilloSol);
    char textoSoles[16];
    snprintf(textoSoles, sizeof(textoSoles), "%d", soles);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto((float)HUD_MARGEN + 18, (float)HUD_PANEL_Y + 64, textoSoles, GLUT_BITMAP_HELVETICA_18);

    for (int i = 0; i < NUM_SEMILLAS; i++) {
        int x0, y0, x1, y1;
        RectanguloSemilla(i, x0, y0, x1, y1);
        TipoPlanta tipo = SEMILLAS[i].tipo;
        int costo = CostoPlanta(tipo);
        bool seleccionada = (tipo == tipoSeleccionado);
        bool alcanzaSoles = (soles >= costo);
        float enRecarga = cooldowns[tipo]; // NUEVO
        bool listaParaPlantar = alcanzaSoles && enRecarga <= 0.0f;
        float fondo[3];
        if (seleccionada) { fondo[0] = 0.95f; fondo[1] = 0.85f; fondo[2] = 0.30f; }
        else if (listaParaPlantar) { fondo[0] = 0.45f; fondo[1] = 0.35f; fondo[2] = 0.20f; }
        else { fondo[0] = 0.30f; fondo[1] = 0.30f; fondo[2] = 0.30f; }
        DibujarRectangulo((float)x0, (float)y0, (float)x1, (float)y1, fondo);
        float cx = (x0 + x1) / 2.0f;
        float cy = (float)y0 + 28.0f;
        DibujarIconoSemilla(tipo, cx, cy);

        // NUEVO: velo gris semi-transparente que "sube" a medida que
        // se completa la recarga (igual que el PvZ original).
        float tiempoTotal = TiempoRecargaPlanta(tipo);
        if (enRecarga > 0.0f && tiempoTotal > 0.0f) {
            float fraccion = enRecarga / tiempoTotal; // 1.0 = recien plantada, 0.0 = lista
            float veloY1 = (float)y1;
            float veloY0 = (float)y0 + (float)(y1 - y0) * (1.0f - fraccion);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.05f, 0.05f, 0.05f, 0.6f);
            glBegin(GL_QUADS);
            glVertex2f((float)x0, veloY0); glVertex2f((float)x1, veloY0);
            glVertex2f((float)x1, veloY1); glVertex2f((float)x0, veloY1);
            glEnd();
            glDisable(GL_BLEND);
        }

        char textoCosto[8];
        snprintf(textoCosto, sizeof(textoCosto), "%d", costo);
        glColor3f(1.0f, 1.0f, 1.0f);
        DibujarTexto((float)x0 + 4, (float)y1 - 6, textoCosto, GLUT_BITMAP_HELVETICA_12);
    }
}
TipoPlanta SemillaEnPosicion(int x, int y, int anchoVentana)
{
    (void)anchoVentana;
    for (int i = 0; i < NUM_SEMILLAS; i++) {
        int x0, y0, x1, y1;
        RectanguloSemilla(i, x0, y0, x1, y1);
        if (x >= x0 && x <= x1 && y >= y0 && y <= y1) return SEMILLAS[i].tipo;
    }
    return NINGUNA;
}

// ============================================================
// Entidades como sprites 2D, ancladas en PIXELES (sin matrices,
// sin rotaciones, sin "acostar" el quad: se ven de pie/de frente,
// igual que en el juego original).
// ============================================================
void DibujarSol(const SolCaido& sol, float cx, float cy)
{
    (void)sol;
    unsigned int tex = GestorSprites::Cargar("sol.png");
    GestorSprites::DibujarSpriteMundoCentrado(tex, cx, cy, 48.0f, 48.0f);
}

// Elige el archivo de sprite segun tipo de planta + EstadoVida.
static std::string RutaSpritePlanta(const Planta& planta)
{
    std::string nombre;
    switch (planta.tipo) {
    case GIRASOL:          nombre = "planta_girasol"; break;
    case LANZAGUISANTES:   nombre = "planta_lanzaguisantes"; break;
    case NUEZ:             nombre = "planta_nuez"; break;
    case CEREZA_EXPLOSIVA: nombre = "planta_cereza"; break;
    default:               return "";
    }
    switch (planta.estadoVida) {
    case VIDA_SANA:    nombre += "_sana.png"; break;
    case VIDA_DANADA:  nombre += "_danada.png"; break;
    case VIDA_CRITICA: nombre += "_critica.png"; break;
    }
    return nombre;
}

void DibujarPlanta(const Planta& planta, float cx, float piesY)
{
    std::string ruta = RutaSpritePlanta(planta);
    if (ruta.empty()) return;
    unsigned int tex = GestorSprites::Cargar(ruta);
    GestorSprites::DibujarSpriteMundo(tex, cx, piesY, 72.0f, 82.0f);
}

void DibujarZombie(const Zombie& zombie, float cx, float piesY)
{
    std::string ruta;
    switch (zombie.tipo) {
    case NORMAL:   ruta = "zombie_normal.png"; break;
    case CON_CONO: ruta = "zombie_cono.png"; break;
    case CUBETA:   ruta = "zombie_cubeta.png"; break;
    }
    unsigned int tex = GestorSprites::Cargar(ruta);
    GestorSprites::DibujarSpriteMundo(tex, cx, piesY, 76.0f, 96.0f);
}

void DibujarProyectil(const Proyectil& proyectil, float cx, float cy)
{
    if (proyectil.estado == ESTADO_DESTRUIDO) return;
    unsigned int tex = GestorSprites::Cargar("proyectil_guisante.png");
    float tam = 26.0f * proyectil.escala;
    GestorSprites::DibujarSpriteMundoCentrado(tex, cx, cy, tam, tam);
}

// NUEVO: carretilla dibujada con primitivas 2D (todavia no tenemos
// un sprite para esto). Se ve como un carrito bajo con dos ruedas y
// una manija, parecido a la podadora clasica del juego original.
void DibujarCarretilla(const Carretilla& carretilla, float cx, float piesY)
{
    if (!carretilla.disponible && !carretilla.activa) return; // ya se gasto: no se dibuja mas
    float rojoCarretilla[] = { 0.75f, 0.15f, 0.10f };
    float metalGris[] = { 0.55f, 0.55f, 0.58f };
    float ruedaNegra[] = { 0.12f, 0.12f, 0.12f };
    float ruedaRin[] = { 0.75f, 0.75f, 0.78f };

    float ancho = 46.0f, alto = 34.0f;
    float x0 = cx - ancho / 2.0f, y1 = piesY - 6.0f, y0 = y1 - alto;

    DibujarRectangulo(x0, y0 + 8.0f, x0 + ancho, y1 - 8.0f, rojoCarretilla);
    DibujarRectangulo(x0 + 4.0f, y0, x0 + ancho - 10.0f, y0 + 10.0f, metalGris);
    // Manija
    DibujarRectangulo(x0 + ancho - 8.0f, y0 - 10.0f, x0 + ancho, y0 + 6.0f, metalGris);
    // Ruedas
    DibujarCirculo2D(x0 + 10.0f, y1, 9.0f, ruedaNegra);
    DibujarCirculo2D(x0 + 10.0f, y1, 4.0f, ruedaRin);
    DibujarCirculo2D(x0 + ancho - 12.0f, y1, 9.0f, ruedaNegra);
    DibujarCirculo2D(x0 + ancho - 12.0f, y1, 4.0f, ruedaRin);
}

// ============================================================
// Pantalla de inicio / menu y Game Over (ya eran 2D, sin cambios)
// ============================================================
void RectanguloBotonJugar(int anchoVentana, int altoVentana, int& x0, int& y0, int& x1, int& y1)
{
    int anchoBoton = 320, altoBoton = 90;
    x0 = anchoVentana / 2 - anchoBoton / 2;
    y0 = altoVentana / 2 - 20;
    x1 = x0 + anchoBoton;
    y1 = y0 + altoBoton;
}
void RectanguloBotonSalir(int anchoVentana, int altoVentana, int& x0, int& y0, int& x1, int& y1)
{
    int anchoBoton = 320, altoBoton = 90;
    x0 = anchoVentana / 2 - anchoBoton / 2;
    y0 = altoVentana / 2 + 90;
    x1 = x0 + anchoBoton;
    y1 = y0 + altoBoton;
}

// NUEVO: boton con pinta de cartel de madera de jardin (para que
// combine con la tematica del fondo), en vez de un rectangulo liso.
static void DibujarBotonMadera(float x0, float y0, float x1, float y1, const char* texto, bool tonoVerde)
{
    float maderaClara[3], maderaOscura[3], maderaBorde[3];
    if (tonoVerde) {
        maderaClara[0] = 0.36f; maderaClara[1] = 0.58f; maderaClara[2] = 0.26f;
        maderaOscura[0] = 0.26f; maderaOscura[1] = 0.44f; maderaOscura[2] = 0.18f;
        maderaBorde[0] = 0.14f; maderaBorde[1] = 0.26f; maderaBorde[2] = 0.10f;
    }
    else {
        maderaClara[0] = 0.62f; maderaClara[1] = 0.24f; maderaClara[2] = 0.16f;
        maderaOscura[0] = 0.48f; maderaOscura[1] = 0.16f; maderaOscura[2] = 0.11f;
        maderaBorde[0] = 0.30f; maderaBorde[1] = 0.08f; maderaBorde[2] = 0.06f;
    }
    // Borde/sombra
    DibujarRectangulo(x0 - 4.0f, y0 - 4.0f, x1 + 4.0f, y1 + 4.0f, maderaBorde);
    // Tabla principal con degradado (luz arriba, sombra abajo)
    DibujarRectanguloGradiente(x0, y0, x1, y1, maderaClara, maderaOscura);
    // Vetas de madera (lineas horizontales sutiles)
    float veta[] = { maderaOscura[0] * 0.85f, maderaOscura[1] * 0.85f, maderaOscura[2] * 0.85f };
    float alto = y1 - y0;
    for (int i = 1; i <= 3; i++) {
        float vy = y0 + alto * (i / 4.0f);
        DibujarRectangulo(x0 + 8.0f, vy - 1.5f, x1 - 8.0f, vy + 1.5f, veta);
    }
    // Clavos en las esquinas
    float clavo[] = { 0.80f, 0.78f, 0.65f };
    DibujarCirculo2D(x0 + 12.0f, y0 + 12.0f, 4.0f, clavo);
    DibujarCirculo2D(x1 - 12.0f, y0 + 12.0f, 4.0f, clavo);
    DibujarCirculo2D(x0 + 12.0f, y1 - 12.0f, 4.0f, clavo);
    DibujarCirculo2D(x1 - 12.0f, y1 - 12.0f, 4.0f, clavo);
    // Texto con sombra
    float cx = (x0 + x1) / 2.0f - (float)strlen(texto) * 4.7f;
    float cy = (y0 + y1) / 2.0f + 5.0f;
    glColor3f(0.0f, 0.0f, 0.0f);
    DibujarTexto(cx + 1, cy + 1, texto, GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto(cx, cy, texto, GLUT_BITMAP_HELVETICA_18);
}

void DibujarMenu(int anchoVentana, int altoVentana, float tiempo)
{
    // ---------------- Cielo con degradado vivo (estilo arte de PvZ) ----------------
    float cieloArriba[] = { 0.20f, 0.55f, 0.85f };
    float cieloAbajo[] = { 0.95f, 0.75f, 0.55f };
    float franjaY = altoVentana * 0.68f;
    DibujarRectanguloGradiente(0, 0, (float)anchoVentana, franjaY, cieloArriba, cieloAbajo);

    float ancho = (float)anchoVentana + 300.0f;
    DibujarNube(fmodf(150.0f + tiempo * 8.0f, ancho), altoVentana * 0.12f);
    DibujarNube(fmodf(500.0f + tiempo * 5.0f, ancho), altoVentana * 0.08f);
    DibujarNube(fmodf(850.0f + tiempo * 6.5f, ancho), altoVentana * 0.16f);

    float radioSol = 46.0f + 4.0f * sinf(tiempo * 1.5f);
    float amarilloSol[] = { 1.0f, 0.90f, 0.35f };
    float amarilloSolClaro[] = { 1.0f, 0.97f, 0.75f };
    DibujarCirculo2D(anchoVentana - 90.0f, 90.0f, radioSol + 10.0f, amarilloSolClaro);
    DibujarCirculo2D(anchoVentana - 90.0f, 90.0f, radioSol, amarilloSol);

    // Siluetas de casitas lejanas, para dar profundidad
    float siluetaCasa[] = { 0.55f, 0.62f, 0.72f };
    for (int i = 0; i < 4; i++) {
        float bx = 40.0f + i * (anchoVentana / 4.3f);
        DibujarRectangulo(bx, franjaY - 55.0f, bx + 60.0f, franjaY, siluetaCasa);
        DibujarTriangulo(bx - 6.0f, franjaY - 55.0f, bx + 66.0f, franjaY - 55.0f, bx + 30.0f, franjaY - 85.0f, siluetaCasa);
    }

    // ---------------- Suelo: cesped a la izquierda, camino a la derecha ----------------
    float verdeClaro[] = { 0.42f, 0.72f, 0.30f };
    float verdeOscuro[] = { 0.34f, 0.62f, 0.24f };
    float caminoGris[] = { 0.55f, 0.53f, 0.50f };
    float divisionX = anchoVentana * 0.62f;
    int franjas = 10;
    for (int i = 0; i < franjas; i++) {
        float fx = i * (divisionX / franjas);
        DibujarRectangulo(fx, franjaY, fx + divisionX / franjas, (float)altoVentana,
            (i % 2 == 0) ? verdeClaro : verdeOscuro);
    }
    DibujarRectangulo(divisionX, franjaY, (float)anchoVentana, (float)altoVentana, caminoGris);

    // Valla metalica separando cesped/camino (como en la referencia)
    float vallaColor[] = { 0.25f, 0.25f, 0.28f };
    for (float vx = divisionX - 4.0f; vx < divisionX + 8.0f; vx += 2.5f) {
        DibujarRectangulo(vx, franjaY - 30.0f, vx + 1.2f, franjaY + 40.0f, vallaColor);
    }

    // ---------------- Plantas reales en primer plano (abajo-izquierda) ----------------
    float baseY = (float)altoVentana - 10.0f;
    unsigned int texGirasol = GestorSprites::Cargar("planta_girasol_sana.png");
    unsigned int texPea = GestorSprites::Cargar("planta_lanzaguisantes_sana.png");
    unsigned int texNuez = GestorSprites::Cargar("planta_nuez_sana.png");
    unsigned int texCereza = GestorSprites::Cargar("planta_cereza_sana.png");
    float bob = sinf(tiempo * 2.2f) * 3.0f; // leve balanceo, como si el viento las moviera
    GestorSprites::DibujarSpriteMundo(texGirasol, anchoVentana * 0.07f, baseY + bob, 90.0f, 100.0f);
    GestorSprites::DibujarSpriteMundo(texNuez, anchoVentana * 0.16f, baseY, 80.0f, 90.0f);
    GestorSprites::DibujarSpriteMundo(texPea, anchoVentana * 0.24f, baseY - bob, 85.0f, 96.0f);
    GestorSprites::DibujarSpriteMundo(texCereza, anchoVentana * 0.32f, baseY + bob * 0.6f, 70.0f, 78.0f);

    // ---------------- Zombies reales acercandose por el camino (derecha) ----------------
    unsigned int texZombieNormal = GestorSprites::Cargar("zombie_normal.png");
    unsigned int texZombieCono = GestorSprites::Cargar("zombie_cono.png");
    float caminar = sinf(tiempo * 1.8f) * 6.0f; // leve vaiven al caminar
    GestorSprites::DibujarSpriteMundo(texZombieCono, anchoVentana * 0.78f + caminar, baseY, 78.0f, 100.0f);
    GestorSprites::DibujarSpriteMundo(texZombieNormal, anchoVentana * 0.90f - caminar, baseY, 74.0f, 96.0f);

    // ---------------- Titulo con rebote y sombra ----------------
    float tituloY = 150.0f + 4.0f * sinf(tiempo * 2.0f);
    glColor3f(0.05f, 0.05f, 0.05f);
    DibujarTexto((float)anchoVentana / 2 - 149, tituloY + 1, "PLANTS VS ZOMBIES 2D", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto((float)anchoVentana / 2 - 150, tituloY, "PLANTS VS ZOMBIES 2D", GLUT_BITMAP_HELVETICA_18);

    // ---------------- Botones tipo cartel de madera, con pulso ----------------
    float pulso = 1.0f + 0.035f * sinf(tiempo * 3.0f);
    int x0, y0, x1, y1;
    RectanguloBotonJugar(anchoVentana, altoVentana, x0, y0, x1, y1);
    {
        float cx = (x0 + x1) / 2.0f, cy = (y0 + y1) / 2.0f;
        float w = (x1 - x0) * pulso, h = (y1 - y0) * pulso;
        DibujarBotonMadera(cx - w / 2.0f, cy - h / 2.0f, cx + w / 2.0f, cy + h / 2.0f, "JUGAR", true);
    }
    RectanguloBotonSalir(anchoVentana, altoVentana, x0, y0, x1, y1);
    DibujarBotonMadera((float)x0, (float)y0, (float)x1, (float)y1, "SALIR", false);
}

void DibujarGameOver(int anchoVentana, int altoVentana)
{
    float negro[] = { 0.05f, 0.05f, 0.05f };
    DibujarRectangulo(0, 0, (float)anchoVentana, (float)altoVentana, negro);
    glColor3f(1.0f, 0.3f, 0.3f);
    DibujarTexto((float)anchoVentana / 2 - 90, (float)altoVentana / 2, "GAME OVER", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto((float)anchoVentana / 2 - 150, (float)altoVentana / 2 + 40,
        "Clic izquierdo para volver al menu", GLUT_BITMAP_HELVETICA_12);
}

// NUEVO: pantalla de victoria (se sobrevivieron todas las oleadas).
void DibujarVictoria(int anchoVentana, int altoVentana)
{
    float verdeOscuro[] = { 0.05f, 0.20f, 0.06f };
    DibujarRectangulo(0, 0, (float)anchoVentana, (float)altoVentana, verdeOscuro);
    glColor3f(1.0f, 0.85f, 0.2f);
    DibujarTexto((float)anchoVentana / 2 - 130, (float)altoVentana / 2 - 20, "GANASTE LA PARTIDA!", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 1.0f, 1.0f);
    char textoOleadas[64];
    snprintf(textoOleadas, sizeof(textoOleadas), "Sobreviviste las %d oleadas", OLEADAS_PARA_GANAR);
    DibujarTexto((float)anchoVentana / 2 - 120, (float)altoVentana / 2 + 20, textoOleadas, GLUT_BITMAP_HELVETICA_12);
    DibujarTexto((float)anchoVentana / 2 - 150, (float)altoVentana / 2 + 50,
        "Clic izquierdo para volver al menu", GLUT_BITMAP_HELVETICA_12);
}
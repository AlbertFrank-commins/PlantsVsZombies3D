#include <GL/glut.h>
#include "Render.h"
#include "GestorSprites.h"
#include <cmath>
#include <cstdio>
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
    float azulCielo[] = { 0.55f, 0.75f, 0.95f };
    DibujarRectangulo(0, 0, (float)anchoVentana, (float)altoVentana, azulCielo);
}

static void DibujarCasa()
{
    float bejeMuro[] = { 0.84f, 0.76f, 0.58f };
    float bejeMuroSombra[] = { 0.76f, 0.68f, 0.50f };
    float tejaRoja[] = { 0.58f, 0.22f, 0.14f };
    float tejaRojaOscura[] = { 0.46f, 0.16f, 0.10f };
    float maderaOscura[] = { 0.32f, 0.19f, 0.09f };
    float maderaClara[] = { 0.45f, 0.28f, 0.14f };
    float vidrioClaro[] = { 0.65f, 0.82f, 0.92f };
    float marcoBlanco[] = { 0.95f, 0.95f, 0.92f };
    float ladrilloChimenea[] = { 0.50f, 0.28f, 0.20f };
    float perillaDorada[] = { 0.85f, 0.70f, 0.25f };

    float x0 = 10.0f, x1 = CESPED_X0 - 15.0f;
    float y0 = CESPED_Y0 - 60.0f, y1 = CESPED_Y1 + 20.0f;
    float anchoTotal = x1 - x0;
    float cxCasa = (x0 + x1) / 2.0f;

    // Sombra suave al pie de la casa (la asienta sobre el cesped)
    float sombra[] = { 0.30f, 0.45f, 0.20f };
    DibujarRectangulo(x0 + 4.0f, y1 - 6.0f, x1 + 10.0f, y1 + 6.0f, sombra);

    // Muro principal, con una franja mas oscura abajo (zocalo)
    DibujarRectangulo(x0, y0, x1, y1, bejeMuro);
    DibujarRectangulo(x0, y1 - 16.0f, x1, y1, bejeMuroSombra);

    // Techo a dos aguas (dos triangulos que se solapan) + tejas (lineas)
    DibujarTriangulo(x0 - 14.0f, y0 + 4.0f, cxCasa, y0 - 75.0f, cxCasa, y0 + 4.0f, tejaRojaOscura);
    DibujarTriangulo(cxCasa, y0 + 4.0f, cxCasa, y0 - 75.0f, x1 + 14.0f, y0 + 4.0f, tejaRoja);
    for (int i = 1; i <= 4; i++) {
        float t = i / 5.0f;
        DibujarTriangulo(x0 - 14.0f + t * (anchoTotal / 2.0f + 14.0f), y0 + 4.0f - t * 5.0f,
            cxCasa, y0 - 75.0f + t * 8.0f,
            cxCasa, y0 + 4.0f - t * 5.0f, tejaRojaOscura);
    }

    // Chimenea
    DibujarRectangulo(cxCasa + anchoTotal * 0.18f, y0 - 55.0f, cxCasa + anchoTotal * 0.18f + 16.0f, y0 - 10.0f, ladrilloChimenea);
    DibujarRectangulo(cxCasa + anchoTotal * 0.18f - 3.0f, y0 - 58.0f, cxCasa + anchoTotal * 0.18f + 19.0f, y0 - 50.0f, maderaOscura);

    // Puerta con marco y perilla
    float puertaAncho = 46.0f, puertaAlto = 82.0f;
    float puertaX0 = cxCasa - puertaAncho / 2.0f;
    DibujarRectangulo(puertaX0 - 5.0f, y1 - puertaAlto - 5.0f, puertaX0 + puertaAncho + 5.0f, y1 + 2.0f, marcoBlanco);
    DibujarRectangulo(puertaX0, y1 - puertaAlto, puertaX0 + puertaAncho, y1, maderaOscura);
    DibujarRectangulo(puertaX0 + 6.0f, y1 - puertaAlto + 10.0f, puertaX0 + puertaAncho - 6.0f, y1 - puertaAlto * 0.5f, maderaClara);
    DibujarCirculo2D(puertaX0 + puertaAncho - 10.0f, y1 - puertaAlto * 0.45f, 3.0f, perillaDorada);

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
    }

    // Farol de porche junto a la puerta
    float farolX = puertaX0 + puertaAncho + 14.0f;
    DibujarRectangulo(farolX - 2.0f, y1 - 60.0f, farolX + 2.0f, y1 - 6.0f, maderaOscura);
    float luzFarol[] = { 0.98f, 0.85f, 0.45f };
    DibujarCirculo2D(farolX, y1 - 62.0f, 7.0f, luzFarol);
}

static void DibujarCamino()
{
    float grisPiedra[] = { 0.70f, 0.68f, 0.62f };
    float y0 = CESPED_Y1;
    float y1 = y0 + 35.0f;
    DibujarRectangulo(0.0f, y0, CESPED_X0, y1, grisPiedra);
}

static void DibujarArbustos()
{
    float verdeArbusto1[] = { 0.20f, 0.45f, 0.15f };
    float verdeArbusto2[] = { 0.25f, 0.55f, 0.18f };
    float cxSeto = CESPED_X1 + 40.0f;
    for (int f = 0; f < FILAS_TABLERO; f++) {
        float cy = FilaAPixelY((float)f) + CELDA_ALTO_PX * 0.5f;
        DibujarCirculo2D(cxSeto, cy, 42.0f, (f % 2 == 0) ? verdeArbusto1 : verdeArbusto2);
        DibujarCirculo2D(cxSeto + 30.0f, cy - 15.0f, 24.0f, (f % 2 == 0) ? verdeArbusto1 : verdeArbusto2);
    }
}

static void DibujarValla()
{
    float blancoValla[] = { 0.92f, 0.92f, 0.88f };
    const float Y_VALLA_BASE = CESPED_Y0 - 4.0f;
    const float ALTO_POSTE = 42.0f;
    for (float x = CESPED_X0 - 8.0f; x <= CESPED_X1 + 8.0f; x += 34.0f) {
        DibujarRectangulo(x - 4.0f, Y_VALLA_BASE - ALTO_POSTE, x + 4.0f, Y_VALLA_BASE, blancoValla);
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
void DibujarInterfaz(int anchoVentana, int altoVentana, int soles, TipoPlanta tipoSeleccionado)
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
        int costo = CostoPlanta(SEMILLAS[i].tipo);
        bool seleccionada = (SEMILLAS[i].tipo == tipoSeleccionado);
        bool alcanzaSoles = (soles >= costo);
        float fondo[3];
        if (seleccionada) { fondo[0] = 0.95f; fondo[1] = 0.85f; fondo[2] = 0.30f; }
        else if (alcanzaSoles) { fondo[0] = 0.45f; fondo[1] = 0.35f; fondo[2] = 0.20f; }
        else { fondo[0] = 0.30f; fondo[1] = 0.30f; fondo[2] = 0.30f; }
        DibujarRectangulo((float)x0, (float)y0, (float)x1, (float)y1, fondo);
        float cx = (x0 + x1) / 2.0f;
        float cy = (float)y0 + 28.0f;
        DibujarIconoSemilla(SEMILLAS[i].tipo, cx, cy);
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

void DibujarMenu(int anchoVentana, int altoVentana, float tiempo)
{
    // Fondo: cielo con nubes animadas + franja de cesped abajo (en vez
    // de estirar el sprite chico a toda la pantalla, que se veia
    // repetido/rayado).
    float azulCielo[] = { 0.55f, 0.75f, 0.95f };
    DibujarRectangulo(0, 0, (float)anchoVentana, (float)altoVentana, azulCielo);

    float ancho = (float)anchoVentana + 300.0f;
    DibujarNube(fmodf(150.0f + tiempo * 8.0f, ancho), altoVentana * 0.15f);
    DibujarNube(fmodf(500.0f + tiempo * 5.0f, ancho), altoVentana * 0.10f);
    DibujarNube(fmodf(850.0f + tiempo * 6.5f, ancho), altoVentana * 0.20f);

    // Sol con un leve pulso de brillo
    float radioSol = 46.0f + 4.0f * sinf(tiempo * 1.5f);
    float amarilloSol[] = { 1.0f, 0.90f, 0.35f };
    float amarilloSolClaro[] = { 1.0f, 0.97f, 0.75f };
    DibujarCirculo2D(anchoVentana - 90.0f, 90.0f, radioSol + 10.0f, amarilloSolClaro);
    DibujarCirculo2D(anchoVentana - 90.0f, 90.0f, radioSol, amarilloSol);

    float verdeClaro[] = { 0.42f, 0.72f, 0.30f };
    float verdeOscuro[] = { 0.34f, 0.62f, 0.24f };
    float franjaY = altoVentana * 0.62f;
    for (int i = 0; i < 14; i++) {
        float fx = i * (anchoVentana / 14.0f);
        DibujarRectangulo(fx, franjaY, fx + anchoVentana / 14.0f, (float)altoVentana,
            (i % 2 == 0) ? verdeClaro : verdeOscuro);
    }

    // Titulo con un leve rebote vertical
    float tituloY = 150.0f + 4.0f * sinf(tiempo * 2.0f);
    glColor3f(0.05f, 0.05f, 0.05f);
    DibujarTexto((float)anchoVentana / 2 - 149, tituloY + 1, "PLANTS VS ZOMBIES 2D", GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto((float)anchoVentana / 2 - 150, tituloY, "PLANTS VS ZOMBIES 2D", GLUT_BITMAP_HELVETICA_18);

    // Botones con un leve pulso de escala (el rectangulo de clic en
    // ManejadorEventos.cpp queda igual, solo el dibujo "respira").
    float pulso = 1.0f + 0.035f * sinf(tiempo * 3.0f);
    int x0, y0, x1, y1;
    RectanguloBotonJugar(anchoVentana, altoVentana, x0, y0, x1, y1);
    {
        float cx = (x0 + x1) / 2.0f, cy = (y0 + y1) / 2.0f;
        float w = (x1 - x0) * pulso, h = (y1 - y0) * pulso;
        unsigned int botonJugar = GestorSprites::Cargar("boton_jugar.png");
        GestorSprites::DibujarQuadPantalla(botonJugar, cx - w / 2.0f, cy - h / 2.0f, w, h);
    }
    RectanguloBotonSalir(anchoVentana, altoVentana, x0, y0, x1, y1);
    {
        unsigned int botonSalir = GestorSprites::Cargar("boton_salir.png");
        GestorSprites::DibujarQuadPantalla(botonSalir, (float)x0, (float)y0, (float)(x1 - x0), (float)(y1 - y0));
    }
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
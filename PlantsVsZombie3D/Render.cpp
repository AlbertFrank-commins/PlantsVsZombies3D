#include <GL/glut.h>
#include "Render.h"
#include <cmath>
#include <cstdio>

// ============================================================
// Render.cpp
// Implementacion del Modulo Visual y Estetico.
// Todas las entidades se construyen combinando primitivas
// volumetricas de FreeGLUT (esfera, cubo, cono). No se usan
// mapas de bits ni texturas: la diferenciacion visual proviene
// unicamente de materiales y de la iluminacion (glMaterialfv +
// GL_LIGHT0), y las normales por cara las genera FreeGLUT
// automaticamente en cada primitiva solida.
//
// FILAS_TABLERO, COLUMNAS_TABLERO, CENTRO_TABLERO_X/Z viven ahora
// en Estructuras.h para que la camara (aqui) y el picking del
// mouse (ManejadorEventos.cpp) nunca queden desincronizados.
// ============================================================

// ---------- Utilidad interna: aplicar material a la primitiva ----------
static void AplicarMaterial(const float ambiente[4], const float difuso[4],
    const float especular[4], float brillo)
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difuso);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMaterialf(GL_FRONT, GL_SHININESS, brillo);
}

// ---------- Iluminacion global ----------
void ConfigurarIluminacion()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // corrige normales al escalar con glScalef
    glEnable(GL_DEPTH_TEST);

    // Luz cenital ambiental simulada, ubicada por encima del tablero
    GLfloat posicionLuz[] = { 0.0f, 15.0f, 5.0f, 1.0f };
    GLfloat luzAmbiente[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat luzDifusa[] = { 0.8f, 0.8f, 0.75f, 1.0f };
    GLfloat luzEspecular[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
}

// ---------- Tablero (cesped a cuadros, sin texturas) ----------
void DibujarTablero()
{
    const float ANCHO_CELDA = 1.0f;
    float verdeClaro[] = { 0.25f, 0.55f, 0.2f, 1.0f };
    float verdeOscuro[] = { 0.15f, 0.4f, 0.12f, 1.0f };
    float sinBrillo[] = { 0.05f, 0.05f, 0.05f, 1.0f };

    for (int f = 0; f < FILAS_TABLERO; f++) {
        for (int c = 0; c < COLUMNAS_TABLERO; c++) {
            glPushMatrix();
            glTranslatef(c * ANCHO_CELDA, 0.0f, f * ANCHO_CELDA);
            bool esClaro = ((f + c) % 2 == 0);
            AplicarMaterial(esClaro ? verdeClaro : verdeOscuro,
                esClaro ? verdeClaro : verdeOscuro,
                sinBrillo, 5.0f);
            // Baldosa plana: cubo aplastado en el eje Y
            glPushMatrix();
            glScalef(ANCHO_CELDA, 0.05f, ANCHO_CELDA);
            glutSolidCube(1.0);
            glPopMatrix();
            glPopMatrix();
        }
    }
}

// ============================================================
// ESCENARIO: ambientacion alrededor del tablero
// Reproduce, con primitivas solidas, los elementos clasicos que
// enmarcan el cesped: la casa a la izquierda, el camino de piedra,
// las podadoras de emergencia sobre el pasto, y el seto de
// arbustos que cierra el patio por la derecha (ver captura de
// referencia del proyecto).
// ============================================================

static void DibujarCasa()
{
    float bejeMuro[] = { 0.80f, 0.72f, 0.55f, 1.0f };
    float tejaRoja[] = { 0.55f, 0.20f, 0.12f, 1.0f };
    float maderaOscura[] = { 0.30f, 0.18f, 0.08f, 1.0f };
    float vidrioClaro[] = { 0.75f, 0.85f, 0.90f, 1.0f };
    float especularCasa[] = { 0.15f, 0.15f, 0.15f, 1.0f };

    glPushMatrix();
    // La casa se ubica a la izquierda del tablero (X negativo),
    // centrada en la profundidad del patio.
    glTranslatef(-2.4f, 0.0f, CENTRO_TABLERO_Z);

    // Muro principal
    AplicarMaterial(bejeMuro, bejeMuro, especularCasa, 8.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(1.8f, 2.0f, 4.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Techo a dos aguas (dos prismas aproximados con cubos rotados)
    AplicarMaterial(tejaRoja, tejaRoja, especularCasa, 5.0f);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(lado * 0.55f, 2.15f, 0.0f);
        glRotatef(lado * 35.0f, 0.0f, 0.0f, 1.0f);
        glScalef(1.1f, 0.15f, 4.8f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Puerta
    AplicarMaterial(maderaOscura, maderaOscura, especularCasa, 15.0f);
    glPushMatrix();
    glTranslatef(0.91f, 0.6f, 1.6f);
    glScalef(0.1f, 1.2f, 0.7f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Ventana
    AplicarMaterial(vidrioClaro, vidrioClaro, especularCasa, 40.0f);
    glPushMatrix();
    glTranslatef(0.91f, 1.3f, -1.2f);
    glScalef(0.08f, 0.6f, 0.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

static void DibujarCamino()
{
    float grisPiedra[] = { 0.65f, 0.63f, 0.58f, 1.0f };
    float sinBrillo[] = { 0.05f, 0.05f, 0.05f, 1.0f };
    AplicarMaterial(grisPiedra, grisPiedra, sinBrillo, 3.0f);

    // Losas de piedra irregulares entre la puerta de la casa y el tablero
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(-0.65f - i * 0.55f, 0.02f, CENTRO_TABLERO_Z + (i % 2 == 0 ? 0.3f : -0.3f));
        glScalef(0.45f, 0.04f, 0.45f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

static void DibujarPodadora(float x, float z)
{
    float rojoPodadora[] = { 0.75f, 0.10f, 0.08f, 1.0f };
    float negroRueda[] = { 0.05f, 0.05f, 0.05f, 1.0f };
    float especular[] = { 0.30f, 0.30f, 0.30f, 1.0f };

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Cuerpo
    AplicarMaterial(rojoPodadora, rojoPodadora, especular, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    glScalef(0.4f, 0.3f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Ruedas
    AplicarMaterial(negroRueda, negroRueda, especular, 5.0f);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0.15f, 0.1f, lado * 0.18f);
        glutSolidSphere(0.1, 10, 10);
        glPopMatrix();
    }

    // Mango
    glPushMatrix();
    glTranslatef(-0.25f, 0.35f, 0.0f);
    glRotatef(-30.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.35f, 0.05f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

static void DibujarArbustos()
{
    float verdeArbusto1[] = { 0.20f, 0.45f, 0.15f, 1.0f };
    float verdeArbusto2[] = { 0.25f, 0.55f, 0.18f, 1.0f };
    float especular[] = { 0.10f, 0.20f, 0.10f, 1.0f };

    float bordeX = COLUMNAS_TABLERO + 0.8f;
    for (int f = -1; f <= FILAS_TABLERO; f++) {
        AplicarMaterial((f % 2 == 0) ? verdeArbusto1 : verdeArbusto2,
            (f % 2 == 0) ? verdeArbusto1 : verdeArbusto2,
            especular, 8.0f);
        glPushMatrix();
        glTranslatef(bordeX, 0.5f, (float)f);
        glScalef(1.1f, 1.0f, 1.1f);
        glutSolidSphere(0.55, 14, 14);
        glPopMatrix();

        // Segunda esfera mas pequena encima para dar volumen irregular
        glPushMatrix();
        glTranslatef(bordeX + 0.18f, 0.85f, (float)f + 0.2f);
        glutSolidSphere(0.3, 10, 10);
        glPopMatrix();
    }
}

static void DibujarValla()
{
    float blancoValla[] = { 0.92f, 0.92f, 0.88f, 1.0f };
    float especular[] = { 0.30f, 0.30f, 0.30f, 1.0f };
    AplicarMaterial(blancoValla, blancoValla, especular, 20.0f);

    // La valla corre a lo largo del borde "trasero" del patio (Z
    // negativo), igual que en la imagen de referencia: separa el
    // cesped jugable del cielo/fondo.
    const float Z_VALLA = -0.45f;
    const float INICIO_X = -0.3f;
    const float FIN_X = (float)COLUMNAS_TABLERO + 0.3f;
    const float SEPARACION_POSTE = 0.5f;

    for (float x = INICIO_X; x <= FIN_X; x += SEPARACION_POSTE) {
        // Poste
        glPushMatrix();
        glTranslatef(x, 0.35f, Z_VALLA);
        glScalef(0.08f, 0.7f, 0.08f);
        glutSolidCube(1.0);
        glPopMatrix();

        // Punta triangular del poste
        glPushMatrix();
        glTranslatef(x, 0.75f, Z_VALLA);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.09, 0.15, 8, 8);
        glPopMatrix();
    }

    // Dos travesanos horizontales que conectan todos los postes
    float largoValla = FIN_X - INICIO_X;
    float alturasListon[2] = { 0.25f, 0.55f };
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(INICIO_X + largoValla / 2.0f, alturasListon[i], Z_VALLA);
        glScalef(largoValla, 0.06f, 0.05f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

static void DibujarAcera()
{
    float grisAcera[] = { 0.72f, 0.72f, 0.70f, 1.0f };
    float lineaAcera[] = { 0.55f, 0.55f, 0.53f, 1.0f };
    float sinBrillo[] = { 0.05f, 0.05f, 0.05f, 1.0f };

    // Acera al frente del patio (Z positivo, mas cerca de la camara),
    // como continuacion del camino de piedra de la casa.
    const float Z_ACERA = (float)FILAS_TABLERO + 0.55f;
    const float ANCHO_ACERA = 1.0f;
    const float INICIO_X = -0.3f;
    const float FIN_X = (float)COLUMNAS_TABLERO + 0.3f;
    float largo = FIN_X - INICIO_X;

    AplicarMaterial(grisAcera, grisAcera, sinBrillo, 3.0f);
    glPushMatrix();
    glTranslatef(INICIO_X + largo / 2.0f, 0.02f, Z_ACERA);
    glScalef(largo, 0.04f, ANCHO_ACERA);
    glutSolidCube(1.0);
    glPopMatrix();

    // Juntas de las losas de concreto (lineas oscuras cada cierto tramo)
    AplicarMaterial(lineaAcera, lineaAcera, sinBrillo, 2.0f);
    for (float x = INICIO_X + 1.0f; x < FIN_X; x += 1.2f) {
        glPushMatrix();
        glTranslatef(x, 0.045f, Z_ACERA);
        glScalef(0.03f, 0.01f, ANCHO_ACERA);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}

static void DibujarBuzon()
{
    float grisMetal[] = { 0.55f, 0.55f, 0.58f, 1.0f };
    float negroPoste[] = { 0.15f, 0.12f, 0.10f, 1.0f };
    float especular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glPushMatrix();
    glTranslatef(-1.6f, 0.0f, (float)FILAS_TABLERO + 0.3f);

    // Poste
    AplicarMaterial(negroPoste, negroPoste, especular, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glScalef(0.07f, 0.7f, 0.07f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Caja del buzon: cuerpo cubico + tapa redondeada (esfera achatada)
    AplicarMaterial(grisMetal, grisMetal, especular, 40.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glScalef(0.35f, 0.2f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 0.85f, 0.0f);
    glScalef(0.35f, 0.12f, 0.2f);
    glutSolidSphere(1.0, 12, 12);
    glPopMatrix();

    glPopMatrix();
}

static void DibujarFloresPequenas()
{
    // Pequenas matas de flores junto al camino de la casa, puro
    // detalle decorativo sin logica asociada.
    float colores[3][4] = {
        { 0.85f, 0.15f, 0.15f, 1.0f },
        { 0.90f, 0.85f, 0.15f, 1.0f },
        { 0.85f, 0.85f, 0.85f, 1.0f }
    };
    float especular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    float tallosVerdes[] = { 0.15f, 0.45f, 0.15f, 1.0f };
    float posiciones[4][2] = {
        { -3.0f, 1.0f }, { -3.0f, 4.0f }, { -1.0f, 0.3f }, { -1.0f, 4.7f }
    };

    for (int i = 0; i < 4; i++) {
        AplicarMaterial(tallosVerdes, tallosVerdes, especular, 5.0f);
        glPushMatrix();
        glTranslatef(posiciones[i][0], 0.1f, posiciones[i][1]);
        glScalef(0.05f, 0.2f, 0.05f);
        glutSolidCube(1.0);
        glPopMatrix();

        AplicarMaterial(colores[i % 3], colores[i % 3], especular, 15.0f);
        glPushMatrix();
        glTranslatef(posiciones[i][0], 0.22f, posiciones[i][1]);
        glutSolidSphere(0.08, 8, 8);
        glPopMatrix();
    }
}

static void DibujarNube(float x, float y, float z)
{
    float blancoNube[] = { 0.98f, 0.98f, 1.00f, 1.0f };
    float sinBrillo[] = { 0.05f, 0.05f, 0.05f, 1.0f };
    AplicarMaterial(blancoNube, blancoNube, sinBrillo, 2.0f);

    glPushMatrix();
    glTranslatef(x, y, z);
    glutSolidSphere(0.6, 12, 12);
    glPushMatrix();
    glTranslatef(0.6f, 0.05f, 0.1f);
    glutSolidSphere(0.45, 12, 12);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.55f, 0.0f, -0.1f);
    glutSolidSphere(0.4, 12, 12);
    glPopMatrix();
    glPopMatrix();
}

static void DibujarNubes()
{
    // Nubes fijas de fondo, detras de la valla, para dar profundidad al cielo
    DibujarNube(-1.0f, 6.0f, -3.0f);
    DibujarNube(6.0f, 6.5f, -3.5f);
    DibujarNube(11.5f, 5.8f, -2.5f);
}

void DibujarEscenario()
{
    DibujarNubes();
    DibujarCasa();
    DibujarCamino();
    DibujarValla();
    DibujarAcera();
    DibujarBuzon();
    DibujarFloresPequenas();
    DibujarArbustos();
    for (int f = 0; f < FILAS_TABLERO; f++) {
        DibujarPodadora(-0.6f, (float)f + 0.5f);
    }
}

// ============================================================
// HUD (interfaz 2D superpuesta): panel de soles y semillas
// ============================================================

struct SemillaHUD {
    TipoPlanta tipo;
};

// Orden en el que se muestran las semillas en pantalla
static const SemillaHUD SEMILLAS[] = {
    { LANZAGUISANTES },
    { GIRASOL },
    { NUEZ },
    { CEREZA_EXPLOSIVA }
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

static void DibujarRectangulo(float x0, float y0, float x1, float y1, const float color[3])
{
    glColor3f(color[0], color[1], color[2]);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
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
    for (const char* c = texto; *c != '\0'; c++) {
        glutBitmapCharacter(fuente, *c);
    }
}

// Miniatura de cada semilla: formas simples que evocan a la planta real
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
    default:
        break;
    }
}

void DibujarInterfaz(int anchoVentana, int altoVentana, int soles, TipoPlanta tipoSeleccionado)
{
    // Guardamos las matrices 3D y cambiamos a una proyeccion
    // ortografica en coordenadas de pantalla (pixeles, origen
    // arriba-izquierda, igual que los eventos de mouse de GLUT).
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, anchoVentana, altoVentana, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // --- Panel de soles ---
    float panelMadera[] = { 0.55f, 0.38f, 0.22f };
    DibujarRectangulo((float)HUD_MARGEN, (float)HUD_PANEL_Y,
        (float)(HUD_MARGEN + HUD_SOL_ANCHO), (float)(HUD_PANEL_Y + HUD_PANEL_ALTO), panelMadera);

    float amarilloSol[] = { 1.0f, 0.85f, 0.15f };
    DibujarCirculo2D((float)HUD_MARGEN + HUD_SOL_ANCHO / 2.0f, (float)HUD_PANEL_Y + 26, 17, amarilloSol);

    char textoSoles[16];
    snprintf(textoSoles, sizeof(textoSoles), "%d", soles);
    glColor3f(1.0f, 1.0f, 1.0f);
    DibujarTexto((float)HUD_MARGEN + 18, (float)HUD_PANEL_Y + 64, textoSoles, GLUT_BITMAP_HELVETICA_18);

    // --- Semillas seleccionables ---
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

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

TipoPlanta SemillaEnPosicion(int x, int y, int anchoVentana)
{
    (void)anchoVentana; // reservado por si el layout pasa a ser proporcional al ancho
    for (int i = 0; i < NUM_SEMILLAS; i++) {
        int x0, y0, x1, y1;
        RectanguloSemilla(i, x0, y0, x1, y1);
        if (x >= x0 && x <= x1 && y >= y0 && y <= y1) return SEMILLAS[i].tipo;
    }
    return NINGUNA;
}

// ============================================================
// SOL CAIDO
// ============================================================
// Esfera amarilla brillante + un halo semitransparente-simulado
// (segunda esfera mas grande y mas tenue) para que se note que es
// un objeto recolectable, distinto de las plantas/zombies.
void DibujarSol(const SolCaido& sol)
{
    (void)sol; // el color/tamano no dependen del estado del sol por ahora
    float amarilloSol[] = { 1.00f, 0.85f, 0.15f, 1.0f };
    float halo[] = { 0.95f, 0.80f, 0.30f, 1.0f };
    float especular[] = { 0.80f, 0.75f, 0.40f, 1.0f };

    // Halo: esfera mas grande, mas plana, sin brillo, da sensacion de resplandor
    AplicarMaterial(halo, halo, especular, 5.0f);
    glPushMatrix();
    glScalef(1.0f, 0.6f, 1.0f);
    glutSolidSphere(0.45, 16, 16);
    glPopMatrix();

    // Nucleo solido del sol
    AplicarMaterial(amarilloSol, amarilloSol, especular, 60.0f);
    glutSolidSphere(0.3, 18, 18);
}

// ---------- Despachadores publicos ----------
void DibujarPlanta(const Planta& planta)
{
    switch (planta.tipo) {
    case LANZAGUISANTES: ModeloLanzaguisantes(); break;
    case GIRASOL: ModeloGirasol(); break;
    case NUEZ: ModeloNuez(); break;
    case CEREZA_EXPLOSIVA: ModeloCerezaExplosiva(); break;
    default: break;
    }
}

void DibujarZombie(const Zombie& zombie)
{
    glPushMatrix();
    // Los modelos de CuerpoBaseZombie (brazos, cabeza) estan
    // construidos mirando hacia +Z, pero los zombies avanzan en -X
    // (ver Zombie::posicionX en LogicaJuego::actualizar). Sin esta
    // rotacion se ven caminando de lado; con ella quedan de frente,
    // avanzando con los brazos extendidos hacia las plantas.
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    switch (zombie.tipo) {
    case NORMAL: ModeloZombieNormal(); break;
    case CON_CONO: ModeloZombieCono(); break;
    case CUBETA: ModeloZombieCubeta(); break;
    }
    glPopMatrix();
}

void DibujarProyectil(const Proyectil& proyectil) // <- Cambiado a Proyectil
{
    if (proyectil.estado == ESTADO_DESTRUIDO) return;
    ModeloGuisante();
}

// ============================================================
// PLANTAS
// ============================================================
// Lanzaguisantes: tallo verde oscuro + cabeza esferica + boca conica
void ModeloLanzaguisantes()
{
    float verdeTallo[] = { 0.10f, 0.50f, 0.15f, 1.0f };
    float verdeCabeza[] = { 0.15f, 0.65f, 0.20f, 1.0f };
    float naranjaOscuro[] = { 0.40f, 0.15f, 0.05f, 1.0f };
    float especularSuave[] = { 0.30f, 0.30f, 0.30f, 1.0f };

    // Tallo
    AplicarMaterial(verdeTallo, verdeTallo, especularSuave, 15.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    glScalef(0.3f, 0.6f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Cabeza
    AplicarMaterial(verdeCabeza, verdeCabeza, especularSuave, 25.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glutSolidSphere(0.35, 20, 20);
    glPopMatrix();

    // Boca: cono orientado hacia el frente en horizontal (eje +X,
    // que es el eje sobre el que se mueven tus zombies segun
    // LogicaJuego::actualizar -> zombie.posicionX). Antes apuntaba
    // en +Z (profundidad de fila), lo cual no coincidia con el eje
    // real de avance de los zombies.
    AplicarMaterial(naranjaOscuro, naranjaOscuro, especularSuave, 10.0f);
    glPushMatrix();
    glTranslatef(0.3f, 0.75f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCone(0.15, 0.25, 12, 12);
    glPopMatrix();
}

// Girasol: tallo + centro esferico amarillo + 8 petalos alrededor
void ModeloGirasol()
{
    float verdeTallo[] = { 0.10f, 0.50f, 0.15f, 1.0f };
    float amarilloCentro[] = { 0.90f, 0.75f, 0.10f, 1.0f };
    float amarilloPetalo[] = { 1.00f, 0.85f, 0.20f, 1.0f };
    float especular[] = { 0.40f, 0.40f, 0.20f, 1.0f };

    // Tallo
    AplicarMaterial(verdeTallo, verdeTallo, especular, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    glScalef(0.25f, 0.5f, 0.25f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Centro de la flor
    AplicarMaterial(amarilloCentro, amarilloCentro, especular, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.65f, 0.0f);
    glutSolidSphere(0.3, 20, 20);
    glPopMatrix();

    // Petalos distribuidos en circulo (esferas aplastadas en abanico)
    AplicarMaterial(amarilloPetalo, amarilloPetalo, especular, 15.0f);
    const int NUM_PETALOS = 8;
    for (int i = 0; i < NUM_PETALOS; i++) {
        float angulo = (360.0f / NUM_PETALOS) * i;
        glPushMatrix();
        glTranslatef(0.0f, 0.65f, 0.0f);
        glRotatef(angulo, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.4f, 0.0f, 0.0f);
        glScalef(0.35f, 0.12f, 0.2f);
        glutSolidSphere(1.0, 10, 10);
        glPopMatrix();
    }
}

// Nuez (muro defensivo): esfera achatada marron + ojos
void ModeloNuez()
{
    float marron[] = { 0.45f, 0.30f, 0.15f, 1.0f };
    float marronOscuro[] = { 0.25f, 0.15f, 0.08f, 1.0f };
    float especular[] = { 0.50f, 0.45f, 0.40f, 1.0f };

    AplicarMaterial(marron, marron, especular, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    glScalef(1.1f, 0.85f, 1.0f);
    glutSolidSphere(0.4, 24, 24);
    glPopMatrix();

    AplicarMaterial(marronOscuro, marronOscuro, especular, 40.0f);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0.18f * lado, 0.5f, 0.38f);
        glutSolidSphere(0.05, 8, 8);
        glPopMatrix();
    }
}

// Cereza explosiva: dos esferas rojas gemelas sobre un tallo comun,
// con un brillo especular alto para sugerir la piel tensa de la fruta
// justo antes de explotar.
void ModeloCerezaExplosiva()
{
    float verdeTallo[] = { 0.10f, 0.45f, 0.12f, 1.0f };
    float rojoCereza[] = { 0.70f, 0.08f, 0.10f, 1.0f };
    float brilloCereza[] = { 0.35f, 0.30f, 0.30f, 1.0f };
    float especularTallo[] = { 0.20f, 0.30f, 0.20f, 1.0f };

    // Tallo comun en Y
    AplicarMaterial(verdeTallo, verdeTallo, especularTallo, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.55f, 0.0f);
    glScalef(0.06f, 0.5f, 0.06f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Dos cerezas gemelas, ligeramente separadas y a distinta altura
    AplicarMaterial(rojoCereza, rojoCereza, brilloCereza, 70.0f);
    glPushMatrix();
    glTranslatef(-0.18f, 0.35f, 0.0f);
    glutSolidSphere(0.28, 18, 18);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18f, 0.45f, 0.05f);
    glutSolidSphere(0.28, 18, 18);
    glPopMatrix();
}

// ============================================================
// ZOMBIES
// ============================================================
// Cuerpo base compartido por las 3 variantes: piernas, torso,
// brazos extendidos y cabeza. Las variantes solo agregan el
// "sombrero" distintivo encima de la cabeza.
static void CuerpoBaseZombie()
{
    float grisRopa[] = { 0.35f, 0.35f, 0.40f, 1.0f };
    float pielVerdosa[] = { 0.55f, 0.60f, 0.45f, 1.0f };
    float especular[] = { 0.25f, 0.25f, 0.25f, 1.0f };

    // Piernas
    AplicarMaterial(grisRopa, grisRopa, especular, 10.0f);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0.15f * lado, 0.35f, 0.0f);
        glScalef(0.18f, 0.7f, 0.18f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Torso
    AplicarMaterial(grisRopa, grisRopa, especular, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.9f, 0.0f);
    glScalef(0.5f, 0.5f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Brazos extendidos al frente (postura clasica de zombie)
    AplicarMaterial(pielVerdosa, pielVerdosa, especular, 20.0f);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0.3f * lado, 1.0f, 0.25f);
        glScalef(0.1f, 0.1f, 0.5f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    // Cabeza
    AplicarMaterial(pielVerdosa, pielVerdosa, especular, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.35f, 0.0f);
    glutSolidSphere(0.25, 16, 16);
    glPopMatrix();
}

void ModeloZombieNormal()
{
    CuerpoBaseZombie();
}

// Zombie con cono de trafico en la cabeza
void ModeloZombieCono()
{
    CuerpoBaseZombie();

    float naranjaCono[] = { 0.90f, 0.40f, 0.05f, 1.0f };
    float especular[] = { 0.50f, 0.30f, 0.10f, 1.0f };
    AplicarMaterial(naranjaCono, naranjaCono, especular, 25.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.55f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.22, 0.4, 16, 16);
    glPopMatrix();
}

// Zombie con cubeta (balde) metalico en la cabeza
void ModeloZombieCubeta()
{
    CuerpoBaseZombie();

    float grisCubeta[] = { 0.60f, 0.60f, 0.65f, 1.0f };
    // Brillo especular alto para sugerir superficie metalica sin usar texturas
    float especular[] = { 0.70f, 0.70f, 0.70f, 1.0f };
    AplicarMaterial(grisCubeta, grisCubeta, especular, 60.0f);
    glPushMatrix();
    glTranslatef(0.0f, 1.6f, 0.0f);
    glScalef(0.3f, 0.3f, 0.3f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// ============================================================
// PROYECTIL
// ============================================================
// Guisante: pequena esfera verde con brillo especular alto
void ModeloGuisante()
{
    float verde[] = { 0.20f, 0.70f, 0.20f, 1.0f };
    float especular[] = { 0.60f, 0.80f, 0.60f, 1.0f };
    AplicarMaterial(verde, verde, especular, 50.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glutSolidSphere(0.12, 14, 14);
    glPopMatrix();
}
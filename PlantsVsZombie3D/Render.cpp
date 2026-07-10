#include <GL/glut.h>
#include "Render.h"

// ============================================================
//  Render.cpp
//  Implementacion del Modulo Visual y Estetico.
//  Todas las entidades se construyen combinando primitivas
//  volumetricas de FreeGLUT (esfera, cubo, cono). No se usan
//  mapas de bits ni texturas: la diferenciacion visual proviene
//  unicamente de materiales y de la iluminacion (glMaterialfv +
//  GL_LIGHT0), y las normales por cara las genera FreeGLUT
//  automaticamente en cada primitiva solida.
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
    glEnable(GL_NORMALIZE);   // corrige normales al escalar con glScalef
    glEnable(GL_DEPTH_TEST);

    // Luz cenital ambiental simulada, ubicada por encima del tablero
    GLfloat posicionLuz[] = { 0.0f, 15.0f, 5.0f, 1.0f };
    GLfloat luzAmbiente[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat luzDifusa[] = { 0.8f,  0.8f,  0.75f, 1.0f };
    GLfloat luzEspecular[] = { 0.6f,  0.6f,  0.6f,  1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
}

// ---------- Tablero (cesped a cuadros, sin texturas) ----------
void DibujarTablero()
{
    const float ANCHO_CELDA = 1.0f;
    float verdeClaro[] = { 0.25f, 0.55f, 0.2f,  1.0f };
    float verdeOscuro[] = { 0.15f, 0.4f,  0.12f, 1.0f };
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

// ---------- Despachadores publicos ----------
void DibujarPlanta(const Planta& planta)
{
    if (planta.estado == ESTADO_DESTRUIDO) return;

    switch (planta.tipo) {
    case PLANTA_LANZAGUISANTES: ModeloLanzaguisantes(); break;
    case PLANTA_GIRASOL:        ModeloGirasol();        break;
    case PLANTA_NUEz:            ModeloNuez();           break;
    }
}

void DibujarZombie(const Zombie& zombie)
{
    if (zombie.estado == ESTADO_DESTRUIDO) return;

    switch (zombie.tipo) {
    case ZOMBIE_NORMAL: ModeloZombieNormal(); break;
    case ZOMBIE_CONO:   ModeloZombieCono();   break;
    case ZOMBIE_CUBETA: ModeloZombieCubeta(); break;
    }
}

void DibujarProyectil(const Guisante& guisante)
{
    if (guisante.estado == ESTADO_DESTRUIDO) return;
    ModeloGuisante();
}

// ============================================================
//  PLANTAS
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

    // Boca: cono orientado hacia el frente (eje +Z, hacia los zombies)
    AplicarMaterial(naranjaOscuro, naranjaOscuro, especularSuave, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.3f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
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

// ============================================================
//  ZOMBIES
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
//  PROYECTIL
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
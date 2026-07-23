#include "GestorSprites.h"
#include <GL/glut.h>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GL_CLAMP_TO_EDGE es de OpenGL 1.2+, pero el <GL/gl.h> clasico de
// Windows solo trae OpenGL 1.1. Se define a mano con su valor oficial.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif



namespace GestorSprites {

    // Cache: ruta -> id de textura ya generado en la GPU
    static std::unordered_map<std::string, unsigned int> g_Cache;

    // Carpeta base donde viven los PNG. Ajusten esta ruta si
    // organizan los assets distinto en su proyecto de Visual Studio
    // (por ejemplo copiando la carpeta al directorio de salida del
    // .exe mediante "Copiar si es mas reciente").
    static const std::string CARPETA_BASE = "Assets/Sprites/";

    void Inicializar() {
        // ANTES (con la camara 3D vieja) se usaba flip_vertically=true
        // porque OpenGL en un mundo 3D espera texturas con origen
        // abajo-izquierda. Ahora TODO el juego es 2D con origen
        // arriba-izquierda (glOrtho(0,w,h,0,-1,1)), igual que un PNG
        // normal se lee de arriba hacia abajo. Dejar el flip en true
        // hacia que los sprites (y los botones del menu) se vieran
        // boca abajo. Con false, una imagen se ve tal cual esta
        // guardada en el archivo.
        stbi_set_flip_vertically_on_load(false);
    }

    unsigned int Cargar(const std::string& rutaPng) {
        auto encontrado = g_Cache.find(rutaPng);
        if (encontrado != g_Cache.end()) {
            return encontrado->second; // ya estaba en cache
        }

        std::string rutaCompleta = CARPETA_BASE + rutaPng;
        int ancho, alto, canales;
        unsigned char* datos = stbi_load(rutaCompleta.c_str(), &ancho, &alto, &canales, 4);
        if (datos == nullptr) {
            printf("[GestorSprites] No se pudo cargar: %s\n", rutaCompleta.c_str());
            g_Cache[rutaPng] = 0;
            return 0;
        }

        unsigned int idTextura;
        glGenTextures(1, &idTextura);
        glBindTexture(GL_TEXTURE_2D, idTextura);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ancho, alto, 0, GL_RGBA, GL_UNSIGNED_BYTE, datos);

        stbi_image_free(datos);

        g_Cache[rutaPng] = idTextura;
        return idTextura;
    }

    void DibujarQuad(unsigned int idTextura, float ancho, float alto) {
        if (idTextura == 0) return; // textura no cargada: no dibuja nada (evita crashear)

        float mitadAncho = ancho / 2.0f;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING); // los sprites 2D ya traen su propio "shading" pintado

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, idTextura);

        // El quad se dibuja de pie, en el plano XY, centrado en X y
        // apoyado sobre Y=0 (para que "flote" sobre la celda igual
        // que antes flotaban los modelos 3D).
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // lo acuesta sobre el plano del tablero visto desde arriba
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-mitadAncho, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(mitadAncho, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(mitadAncho, alto, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-mitadAncho, alto, 0.0f);
        glEnd();
        glPopMatrix();

        glPopAttrib();
    }

    // Helper interno: dibuja el quad texturizado en coordenadas de
    // pantalla puras (x0,y0)-(x1,y1), con blending para el canal
    // alpha, sin tocar lighting/depth (ya estan deshabilitados
    // globalmente porque todo el juego es 2D).
    static void DibujarQuadInterno(unsigned int idTextura, float x0, float y0, float x1, float y1) {
        if (idTextura == 0) return;
        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, idTextura);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x0, y0);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x1, y0);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x1, y1);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x0, y1);
        glEnd();
        glPopAttrib();
    }

    void DibujarSpriteMundo(unsigned int idTextura, float cx, float piesY, float ancho, float alto) {
        float mitad = ancho / 2.0f;
        DibujarQuadInterno(idTextura, cx - mitad, piesY - alto, cx + mitad, piesY);
    }

    void DibujarSpriteMundoCentrado(unsigned int idTextura, float cx, float cy, float ancho, float alto) {
        float mitadA = ancho / 2.0f;
        float mitadH = alto / 2.0f;
        DibujarQuadInterno(idTextura, cx - mitadA, cy - mitadH, cx + mitadA, cy + mitadH);
    }

    void DibujarQuadPantalla(unsigned int idTextura, float x, float y, float ancho, float alto) {
        if (idTextura == 0) return;

        glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, idTextura);

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + ancho, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + ancho, y + alto);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + alto);
        glEnd();

        glPopAttrib();
    }

    void Liberar() {
        for (auto& par : g_Cache) {
            if (par.second != 0) {
                GLuint id = par.second;
                glDeleteTextures(1, &id);
            }
        }
        g_Cache.clear();
    }
}
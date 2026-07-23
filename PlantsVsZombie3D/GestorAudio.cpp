#include "GestorAudio.h"
#include <cstdio>
#include <unordered_map>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace GestorAudio {

    static ma_engine g_Motor;
    static bool g_Inicializado = false;
    static ma_sound g_Musica;
    static bool g_MusicaCargada = false;

    static const std::string CARPETA_BASE = "Assets/Sonidos/";

    bool Inicializar() {
        ma_result resultado = ma_engine_init(nullptr, &g_Motor);
        if (resultado != MA_SUCCESS) {
            printf("[GestorAudio] No se pudo inicializar el motor de audio.\n");
            g_Inicializado = false;
            return false;
        }
        g_Inicializado = true;
        return true;
    }

    void ReproducirMusica(const std::string& rutaArchivo) {
        if (!g_Inicializado) return;

        if (g_MusicaCargada) {
            ma_sound_stop(&g_Musica);
            ma_sound_uninit(&g_Musica);
            g_MusicaCargada = false;
        }

        std::string rutaCompleta = CARPETA_BASE + rutaArchivo;
        ma_result resultado = ma_sound_init_from_file(&g_Motor, rutaCompleta.c_str(),
            MA_SOUND_FLAG_STREAM, nullptr, nullptr, &g_Musica);
        if (resultado != MA_SUCCESS) {
            printf("[GestorAudio] No se pudo cargar musica: %s\n", rutaCompleta.c_str());
            return;
        }
        ma_sound_set_looping(&g_Musica, MA_TRUE);
        ma_sound_start(&g_Musica);
        g_MusicaCargada = true;
    }

    void DetenerMusica() {
        if (g_MusicaCargada) {
            ma_sound_stop(&g_Musica);
        }
    }

    void ReproducirEfecto(const std::string& rutaArchivo) {
        if (!g_Inicializado) return;
        std::string rutaCompleta = CARPETA_BASE + rutaArchivo;
        // ma_engine_play_sound reproduce "y se olvida": ideal para
        // efectos cortos que pueden superponerse (varios disparos
        // a la vez, por ejemplo).
        ma_engine_play_sound(&g_Motor, rutaCompleta.c_str(), nullptr);
    }

    void ReproducirEventoLogica(const std::string& nombreEvento) {
        // Mapeo centralizado evento logico -> archivo de sonido.
        // Ajusten los nombres de archivo a los que efectivamente
        // coloquen en Assets/Sonidos/.
        static const std::unordered_map<std::string, std::string> mapa = {
            { "plantar",          "plantar.wav" },
            { "disparo",          "disparo.wav" },
            { "planta_destruida", "mordida.wav" },
            { "sol",              "recolectar_sol.wav" },
            { "explosion",        "explosion.wav" },
        };
        auto it = mapa.find(nombreEvento);
        if (it != mapa.end()) {
            ReproducirEfecto(it->second);
        }
    }

    void Liberar() {
        if (g_MusicaCargada) {
            ma_sound_uninit(&g_Musica);
            g_MusicaCargada = false;
        }
        if (g_Inicializado) {
            ma_engine_uninit(&g_Motor);
            g_Inicializado = false;
        }
    }
}
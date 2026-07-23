#pragma once
#include <string>

// ============================================================
// GestorAudio.h
// Modulo 3: Control de Escena
// -----------------------------------------------------------
// Envoltorio simple sobre miniaudio.h para reproducir musica de
// fondo y efectos de sonido cortos. LogicaJuego NO llama a este
// modulo directamente: solo encola nombres de evento en
// g_Logica->eventosSonido. Es ManejadorEventos quien lee esa
// cola cada frame y llama a GestorAudio::ReproducirEfecto().
// ============================================================
namespace GestorAudio {

    // Debe llamarse UNA vez al iniciar el programa (en main.cpp).
    // Devuelve false si no se pudo inicializar el dispositivo de audio.
    bool Inicializar();

    // Musica de fondo en loop (una sola a la vez: si ya habia una
    // sonando, la reemplaza).
    void ReproducirMusica(const std::string& rutaArchivo);
    void DetenerMusica();

    // Efecto corto de un solo disparo (plantar, disparo, mordida,
    // explosion, recolectar sol, etc.). Se puede llamar muchas
    // veces por segundo sin problema.
    void ReproducirEfecto(const std::string& rutaArchivo);

    // Traduce un nombre de evento de LogicaJuego (por ejemplo
    // "plantar", "disparo", "mordida", "sol") al archivo de sonido
    // correspondiente y lo reproduce. Centraliza el mapeo evento->
    // archivo en un solo lugar.
    void ReproducirEventoLogica(const std::string& nombreEvento);

    // Liberar todos los recursos de audio (llamar al cerrar el programa).
    void Liberar();
}
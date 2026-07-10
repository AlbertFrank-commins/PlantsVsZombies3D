using System;
using System.Threading;
using PlantsVsZombies3D.CoreLogica;

namespace PlantsVsZombies3D
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("=== SIMULACIÓN DEL CORE_LÓGICA - PvZ 3D ===");

            LogicaJuego juego = new LogicaJuego();

            // 1. Plantamos un Lanzaguisantes en la Fila 2, Columna 1
            juego.Plantar(2, 1, TipoPlanta.Lanzaguisantes);

            // 2. Aparece un Zombie en la Fila 2 (X = 9.0)
            juego.AparecerZombie(2, TipoZombie.Normal);

            float tiempoSimulado = 0f;
            float deltaTime = 0.5f;

            while (tiempoSimulado < 15.0f)
            {
                juego.Actualizar(deltaTime);

                Console.Clear();
                Console.WriteLine($"=== TIEMPO: {tiempoSimulado:F1}s | SOLES: {juego.Soles} ===");

                foreach (var z in juego.ListaZombies)
                {
                    Console.WriteLine($"[Zombie] Fila: {z.Fila} | Posición X: {z.PosicionX:F2} | Vida: {z.Vida} | ¿Comiendo?: {z.EstaComiendo}");
                }

                foreach (var p in juego.ListaProyectiles)
                {
                    Console.WriteLine($"  -> [Guisante] Fila: {p.Fila} | Posición X: {p.PosicionX:F2}");
                }

                if (juego.ListaZombies.Count == 0)
                {
                    Console.WriteLine("\n¡Felicidades! Tu lógica eliminó al zombie.");
                    break;
                }

                tiempoSimulado += deltaTime;
                Thread.Sleep(500);
            }

            Console.WriteLine("\nPrueba de lógica finalizada con éxito.");
            Console.ReadLine();
        }
    }
}

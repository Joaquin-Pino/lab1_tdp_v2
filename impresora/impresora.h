// impresora.h
#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"

// Métodos estáticos de presentación. No modifica ningún objeto del juego.
class Impresora {
public:
    static void imprimirTablero(const Tablero& tablero);

    // Renderiza el tablero con las piezas en las posiciones del estado dado.
    // Recalcula las salidas dinámicas según el stepUsed del estado,
    // ya que su largo puede haber cambiado respecto al tablero estático.
    static void imprimirEstado(const Tablero& tablero, const Estado& estado);

    // Imprime la secuencia de movimientos consolidada (ej: R5,1 R5,1 → R5,2).
    // El arreglo solucion termina con nullptr como centinela.
    static void imprimirSolucion(const Tablero& tablero, Estado** solucion);

    // Imprime la solución consolidada seguida del tablero tras cada movimiento individual.
    static void imprimirSolucionPasoAPaso(const Tablero& tablero, Estado** solucion);
};
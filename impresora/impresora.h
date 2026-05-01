// impresora.h
#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"

class Impresora {
public:
    // imprime el tablero base sin piezas
    static void imprimirTablero(const Tablero& tablero);
    
    // imprime el tablero con las piezas en sus posiciones del estado
    static void imprimirEstado(const Tablero& tablero, const Estado& estado);
    
    // imprime solo el string de solución (consolidado)
    static void imprimirSolucion(const Tablero& tablero, Estado** solucion);

    // imprime la solución paso a paso (cada estado tras cada movimiento)
    static void imprimirSolucionPasoAPaso(const Tablero& tablero, Estado** solucion);
};
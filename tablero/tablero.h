#pragma once
#include "../estado/estado.h"
#include "../pieza/pieza.h"
#include "../salida/salida.h"
#include "../compuerta/compuerta.h"

enum celda{
    VACIA,
    PARED,
    COMPUERTA,
    PIEZA
};

enum direccion{
    ARRIBA,
    ABAJO,
    IZQUIERDA,
    DERECHA
};

class Tablero {
private:
    celda** matriz;
    Pieza* piezas;
    Salida* salidas;
    Compuerta* compuertas;
    int numPiezas, numSalidas, numCompuertas;
    int w, h, stepLimit;

    Tablero();
    Tablero(celda** matriz, Pieza* piezas, Salida* salidas, Compuerta* compuertas, int numPiezas, int numSalidas, int numCompuertas, int w, int h, int stepLimit);
    
    // regla de los 3
    Tablero(const Tablero& otro); // constructor de copia
    Tablero& operator=(const Tablero& otro); // operador de asignacion
    ~Tablero();

    // decidir si pasar estado como referencia o puntero
    bool piezaPuedeMoverse(int id, direccion dir, const Estado& estado);
    bool piezaPuedeSalir(int id, const Estado& estado);
    int calcularColorCompuerta(int idCompuerta, const Estado& estado);
    int calcularLargoSalida(int idSalida, const Estado& estado);
    int calcularHeuristica(const Estado& estado);
    Estado* crearEstadoInicial() const;
};
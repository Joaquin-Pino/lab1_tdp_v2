#pragma once
#include "../estado/estado.h"
#include "../pieza/pieza.h"
#include "../salida/salida.h"
#include "../compuerta/compuerta.h"

enum tipoCelda {
    VACIA,
    PARED,
    COMPUERTA,
    PIEZA,
    SALIDA
};

struct celda {
    int tipo;       // VACIA, PARED, COMPUERTA, PIEZA, SALIDA
    int id;         // id de la pieza, salida o compuerta. -1 si no aplica
};

enum direccion{
    ARRIBA,
    ABAJO,
    IZQUIERDA,
    DERECHA
};

class Tablero {
private:
    celda* matriz;
    Pieza* piezas;
    Salida* salidas;
    Compuerta* compuertas;
    int numPiezas, numSalidas, numCompuertas;
    int w, h, stepLimit;

    bool esSalidaValida(int fila, int columna, const Pieza& pieza, const Estado& estado);
public:
    Tablero();
    Tablero(celda* matriz, Pieza* piezas, Salida* salidas, Compuerta* compuertas, int numPiezas, int numSalidas, int numCompuertas, int w, int h, int stepLimit);
    
    // regla de los 3
    Tablero(const Tablero& otro); // constructor de copia
    Tablero& operator=(const Tablero& otro); // operador de asignacion
    ~Tablero();

    //getters
    celda* getMatriz() const;
    Pieza* getPiezas() const;
    Salida* getSalidas() const;
    Compuerta* getCompuertas() const;
    int getNumPiezas() const;
    int getNumSalidas() const;
    int getNumCompuertas() const;
    int getW() const;
    int getH() const;
    int getStepLimit() const;
    
    // decidir si pasar estado como referencia o puntero
    bool piezaPuedeMoverse(int id, direccion dir, const Estado& estado);
    bool piezaPuedeSalir(int id, const Estado& estado);
    int calcularColorCompuerta(int idCompuerta, const Estado& estado);
    int calcularLargoSalida(int idSalida, const Estado& estado);
    int calcularHeuristica(const Estado& estado);
    Estado* crearEstadoInicial() const;

    void imprimir() const;
};
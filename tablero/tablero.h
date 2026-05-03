#pragma once
#include "../estado/estado.h"
#include "../pieza/pieza.h"
#include "../salida/salida.h"
#include "../compuerta/compuerta.h"

// Tipos de celda para la matriz estática del tablero.
enum tipoCelda {
    VACIA,
    PARED,
    COMPUERTA,
    PIEZA,   // posición inicial de una pieza (solo en el estado inicial; el solver usa ocupacion)
    SALIDA
};

// Una celda del tablero estático.
// `id` apunta al índice en el arreglo correspondiente (piezas[], salidas[], compuertas[]).
// Vale -1 si el tipo no tiene entidad asociada (VACIA, PARED).
struct celda {
    int tipo;
    int id;
};

// Direcciones de movimiento para piezas.
enum direccion{
    ARRIBA,
    ABAJO,
    IZQUIERDA,
    DERECHA
};

// Representa los datos estáticos del tablero: paredes, salidas, compuertas y piezas.
// No cambia durante la búsqueda; el Estado guarda la parte mutable (posiciones, etc.).
// Esta clase es dueña de todos sus arreglos (matriz, piezas, salidas, compuertas).
class Tablero {
private:
    celda*     matriz;      // matriz[y*w + x]: celda estática en la posición (x,y)
    Pieza*     piezas;
    Salida*    salidas;
    Compuerta* compuertas;
    int numPiezas, numSalidas, numCompuertas;
    int w, h, stepLimit;

public:
    // Devuelve true si la celda (fila, columna) es una salida válida para la pieza dada.
    // Chequea tipo, color y que la pieza quepa en el largo actual de la salida.
    bool esSalidaValida(int fila, int columna, const Pieza& pieza, const Estado& estado);

    Tablero();
    // Toma ownership de todos los arreglos pasados como punteros.
    Tablero(celda* matriz, Pieza* piezas, Salida* salidas, Compuerta* compuertas,
            int numPiezas, int numSalidas, int numCompuertas, int w, int h, int stepLimit);
    Tablero(const Tablero& otro);
    Tablero& operator=(const Tablero& otro);
    ~Tablero();

    celda*     getMatriz() const;
    // Devuelve una copia del arreglo de celdas; el llamador es responsable de liberarla.
    celda*     getCopiaMatriz() const;
    Pieza*     getPiezas() const;
    Salida*    getSalidas() const;
    Compuerta* getCompuertas() const;
    int getNumPiezas() const;
    int getNumSalidas() const;
    int getNumCompuertas() const;
    int getW() const;
    int getH() const;
    int getStepLimit() const;

    // Verifica si la pieza `id` puede moverse 1 celda en la dirección `dir` en el estado dado.
    // Chequea límites del tablero, ocupación por otras piezas, paredes y compuertas.
    bool piezaPuedeMoverse(int id, direccion dir, const Estado& estado);

    // Verifica si la pieza `id` está adyacente a una salida válida y puede salir ahora.
    // Examina los 4 bordes de la pieza en busca de una celda de salida compatible.
    bool piezaPuedeSalir(int id, const Estado& estado);

    // Calcula el color actual de la compuerta `idCompuerta` según el stepUsed del estado.
    int calcularColorCompuerta(int idCompuerta, const Estado& estado);

    // Calcula el largo actual de la salida `idSalida` según el stepUsed del estado.
    int calcularLargoSalida(int idSalida, const Estado& estado) const;

    // Construye el Estado inicial a partir de las posiciones y valores de arranque del tablero.
    Estado* crearEstadoInicial() const;

    // Devuelve true si la pieza PODRÍA llegar a salir por esa salida eventualmente,
    // asumiendo el largo máximo posible. Usado por la heurística para filtrar salidas inviables.
    bool piezaPodriaSalir(Pieza& pieza, Salida& salida) const;

    void imprimir() const;
};

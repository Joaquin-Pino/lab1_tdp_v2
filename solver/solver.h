// solver.h
#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"
#include "../minheap/minheap.h"
#include "../tablaHash/tablaHash.h"

// Implementa el algoritmo A* para resolver el puzzle Color Block Jam.
//
// El Solver no toma ownership del Tablero (no lo libera).
// Sí toma ownership temporal de los Estado* en openSet y closedSet,
// y los libera en el destructor.
class Solver {
private:
    Tablero*   tablero;
    MinHeap*   openSet;    // estados por explorar, ordenados por f = g + h
    TablaHash* closedSet;  // estados ya explorados, para no revisitarlos
    Estado**   vecinosTemp; // buffer reutilizable para los vecinos de cada expansión
    int        maxVecinos;  // tamaño del buffer (numPiezas * (4*maxDim + 1))

    // Genera todos los vecinos válidos del estado `actual` y los almacena en vecinosTemp.
    // Devuelve la cantidad de vecinos generados.
    int generarVecinos(Estado* actual);

    // Sigue la cadena de punteros parent desde el estado meta hasta el inicial
    // y devuelve un arreglo de Estado* (copias) terminado en nullptr.
    Estado** reconstruirCamino(Estado* final);

    // Heurística informada: distancia Manhattan al borde de la pieza hacia su mejor salida,
    // más un término de bloqueos dividido entre 2. Puede sobreestimar levemente.
    int calcularHeuristica(const Estado& estado) const;

    // Cuenta cuántas piezas distintas bloquean el camino rectilíneo de la pieza `idPieza`
    // hacia su salida más cercana (versión sin salida explícita).
    int contarBloqueos(int idPieza, coordenada pos, const Estado& estado) const;

    // Cuenta bloqueos hacia una salida específica en `posSalida`.
    // Prueba los dos recorridos posibles (horizontal primero o vertical primero)
    // y devuelve el mínimo, para no sobreestimar más de lo necesario.
    int contarBloqueos(int idPieza, coordenada pos, coordenada posSalida, const Estado& estado) const;

    // Devuelve true si mover la pieza `idPieza` en dirección `dir` deshace el movimiento
    // previo registrado en el estado. Evitar este movimiento es una poda segura porque
    // el estado resultante ya fue (o será) evaluado.
    bool esMovimientoRedundante(int idPieza, char dir, const Estado* actual) const;

    // Post-procesa un vecino recién creado: actualiza elementos dinámicos (compuertas, salidas),
    // calcula h y f, y registra el parent y el movimiento que lo generó.
    void prepararVecino(Estado* vecino, Estado* actual, char mov[10]) const;

public:
    Solver(Tablero* t);
    Solver(const Solver&) = delete;
    Solver& operator=(const Solver&) = delete;
    ~Solver();

    // Ejecuta A* desde estadoInicial. Toma ownership de estadoInicial.
    // Devuelve un arreglo de Estado* (terminado en nullptr) con el camino solución,
    // o nullptr si no existe solución dentro del stepLimit.
    // El llamador es responsable de liberar el arreglo y sus elementos.
    Estado** resolver(Estado* estadoInicial);
};

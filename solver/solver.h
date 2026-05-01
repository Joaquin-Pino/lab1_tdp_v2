// solver.h
#pragma once
#include "../tablero/tablero.h"
#include "../estado/estado.h"
#include "../minheap/minheap.h"
#include "../tablaHash/tablaHash.h"

class Solver {
private:
    Tablero* tablero;
    MinHeap* openSet;
    TablaHash* closedSet;
    Estado** vecinosTemp;
    int maxVecinos;

    int generarVecinos(Estado* actual);
    Estado** reconstruirCamino(Estado* final);
    int calcularHeuristica(const Estado& estado) const;
    int contarBloqueos(int idPieza, coordenada pos, const Estado& estado) const;
    int contarBloqueos(int idPieza, coordenada pos, coordenada posSalida, const Estado& estado) const;
    bool esMovimientoRedundante(int idPieza, char dir, const Estado* actual) const;
    void prepararVecino(Estado* vecino, Estado* actual, char mov[10]) const;
public:
    Solver(Tablero* t);
    Solver(const Solver&) = delete;
    Solver& operator=(const Solver&) = delete;
    ~Solver();

    Estado** resolver(Estado* estadoInicial);
};
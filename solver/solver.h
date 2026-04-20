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

public:
    Solver(Tablero* t);
    Solver(const Solver&) = delete;
    Solver& operator=(const Solver&) = delete;
    ~Solver();

    Estado** resolver(Estado* estadoInicial);
};
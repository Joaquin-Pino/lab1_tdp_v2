// solver.cpp
#include "solver.h"
#include <cstring>
#include <iostream>

Solver::Solver(Tablero* t)
    : tablero(t), openSet(nullptr), closedSet(nullptr), vecinosTemp(nullptr) {
    // maxVecinos = 4 direcciones * numPiezas
    maxVecinos  = t->getNumPiezas() * 4;
    openSet     = new MinHeap(1024);
    closedSet   = new TablaHash(100003);  // primo grande
    vecinosTemp = new Estado*[maxVecinos];
}

Solver::~Solver() {
    while (!openSet->estaVacio()) {
        Estado* e = openSet->pop();
        delete e;
    }
    closedSet->liberarEstados();  // libera los Estado*
    delete openSet;
    delete closedSet;
    delete[] vecinosTemp;
}

int Solver::generarVecinos(Estado* actual) {
    int count = 0;
    int numPiezas = tablero->getNumPiezas();
    int w = tablero->getW();

    direccion dirs[] = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};
    const char* dirStr[] = {"U", "D", "L", "R"};
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue;

        Pieza& pieza = tablero->getPiezas()[id];

        if (tablero->piezaPuedeSalir(id, *actual)) {
            Estado* vecino = new Estado(*actual);
            vecino->sacarPieza(id, pieza, w);

            int h = tablero->calcularHeuristica(*vecino);
            vecino->setH(h);
            vecino->setF(vecino->getStepUsed() + h);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId());
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
            continue;
        }

        for (int d = 0; d < 4; d++) {
            if (!tablero->piezaPuedeMoverse(id, dirs[d], *actual)) continue;

            Estado* vecino = new Estado(*actual);
            vecino->moverPieza(id, dx[d], dy[d], pieza, w);

            for (int i = 0; i < tablero->getNumCompuertas(); i++)
                vecino->actualizarCompuerta(i,
                    tablero->calcularColorCompuerta(i, *vecino));

            for (int i = 0; i < tablero->getNumSalidas(); i++)
                vecino->actualizarSalida(i,
                    tablero->calcularLargoSalida(i, *vecino));

            int h = tablero->calcularHeuristica(*vecino);
            vecino->setH(h);
            vecino->setF(vecino->getStepUsed() + h);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "%c%d,1", dirStr[d][0], pieza.getId());
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
        }
    }
    return count;
}
Estado** Solver::reconstruirCamino(Estado* final) {
    // contar pasos
    int numPasos = 0;
    Estado* actual = final;
    while (actual != nullptr) {
        numPasos++;
        actual = actual->getParent();
    }

    Estado** camino = new Estado*[numPasos + 1];

    actual = final;
    for (int i = numPasos - 1; i >= 0; i--) {
        camino[i] = actual;
        actual = actual->getParent();
    }
    
    camino[numPasos] = nullptr;  // marcar fin del camino
    return camino;
}

Estado** Solver::resolver(Estado* estadoInicial) {
    
    // calcular heuristica inicial
    int h = tablero->calcularHeuristica(*estadoInicial);
    estadoInicial->setH(h);
    estadoInicial->setF(estadoInicial->getStepUsed() + h);
    estadoInicial->setParent(nullptr);

    openSet->push(estadoInicial);

    while (!openSet->estaVacio()) {
        Estado* actual = openSet->pop();

        // verificar si ya fue visitado
        if (closedSet->existe(actual)) {
            delete actual;
            continue;
        }

        // verificar si es solución
        if (actual->jugoTerminado(tablero->getNumPiezas())) {
            Estado** solucion = reconstruirCamino(actual);
            // limpiar estados en openSet
            // los estados del camino se mantienen vía punteros padre
            return solucion;
        }

        // verificar step limit
        if (actual->getStepUsed() >= tablero->getStepLimit()) {
            delete actual;
            continue;
        }

        closedSet->insertar(actual);

        // generar vecinos
        int numVecinos = generarVecinos(actual);
        for (int i = 0; i < numVecinos; i++) {
            if (!closedSet->existe(vecinosTemp[i]))
                openSet->push(vecinosTemp[i]);
            else
                delete vecinosTemp[i];
        }
    }

    return nullptr;  // sin solución
}
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
    delete openSet;
    delete closedSet;
    delete[] vecinosTemp;
    // no liberar tablero — no es dueño
}

int Solver::generarVecinos(Estado* actual) {
    int count = 0;
    int numPiezas = tablero->getNumPiezas();

    direccion dirs[] = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};
    const char* dirStr[] = {"U", "D", "L", "R"};

    // aplicamos esto para cada pieza 
    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue;

        // verificar si puede salir antes de mover
        if (tablero->piezaPuedeSalir(id, *actual)) {
            Estado* vecino = new Estado(*actual);

            // sacar pieza: lo marcamos en el bitmask
            vecino->setPiezasSalidas(
                actual->getPiezasSalidas() | (1u << id)
            );

            // actualizar compuertas y salidas según nuevo stepUsed
            // (stepUsed no cambia al sacar, solo al mover)
            int h = tablero->calcularHeuristica(*vecino);
            vecino->setH(h);
            vecino->setF(vecino->getStepUsed() + h);
            vecino->setParent(actual);

            // movimiento formato: "S<id>"
            char mov[10];
            mov[0] = 'S';
            mov[1] = '0' + id;
            mov[2] = '\0';
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
            continue;
        }

        // intentar mover en cada dirección
        for (int d = 0; d < 4; d++) {
            if (!tablero->piezaPuedeMoverse(id, dirs[d], *actual)) continue;

            Estado* vecino = new Estado(*actual);

            // aplicar movimiento
            coordenada* pos = vecino->getPosPiezas();
            switch (dirs[d]) {
                case ARRIBA:    pos[id].y--; break;
                case ABAJO:     pos[id].y++; break;
                case IZQUIERDA: pos[id].x--; break;
                case DERECHA:   pos[id].x++; break;
            }

            // incrementar step y actualizar compuertas/salidas
            int newStep = vecino->getStepUsed() + 1;
            vecino->setStepUsed(newStep);

            // actualizar color de compuertas
            for (int i = 0; i < tablero->getNumCompuertas(); i++)
                vecino->getColorCompuertas()[i] =
                    tablero->calcularColorCompuerta(i, *vecino);

            // actualizar largo de salidas
            for (int i = 0; i < tablero->getNumSalidas(); i++)
                vecino->getLargoSalidas()[i] =
                    tablero->calcularLargoSalida(i, *vecino);

            int h = tablero->calcularHeuristica(*vecino);
            vecino->setH(h);
            vecino->setF(newStep + h);
            vecino->setParent(actual);

            // movimiento formato: "R<id>,1"
            char mov[10];
            int idReal = tablero->getPiezas()[id].getId();  // id real del archivo
            mov[0] = dirStr[d][0];
            // formato: R<idReal>,1
            snprintf(mov, 10, "%c%d,1", dirStr[d][0], idReal);
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
        }
    }
    return count;
}

char* Solver::reconstruirCamino(Estado* final) {
    // contar pasos
    int pasos = 0;
    Estado* actual = final;
    while (actual->getParent() != nullptr) {
        pasos++;
        actual = actual->getParent();
    }

    // alojar string resultado
    // cada movimiento ocupa máximo 10 chars
    char* resultado = new char[pasos * 10 + 1];
    resultado[0] = '\0';

    // reconstruir en orden inverso usando arreglo temporal
    const char** movimientos = new const char*[pasos];
    actual = final;
    for (int i = pasos - 1; i >= 0; i--) {
        movimientos[i] = actual->getMovimiento();
        actual = actual->getParent();
    }

    for (int i = 0; i < pasos; i++)
        strcat(resultado, movimientos[i]);

    delete[] movimientos;
    return resultado;
}

char* Solver::resolver(Estado* estadoInicial) {
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
            char* solucion = reconstruirCamino(actual);
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
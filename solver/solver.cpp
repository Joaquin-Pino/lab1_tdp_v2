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

            int h = calcularHeuristica(*vecino);
            vecino->setH(h);
            vecino->setF(vecino->getStepUsed() + h);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId());
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
            continue;
        }
        
        // mover las piezas
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

            int h = calcularHeuristica(*vecino);
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
        int calcularHeuristica(const Estado& estado);
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
    int h = calcularHeuristica(*estadoInicial);
    estadoInicial->setH(h);
    estadoInicial->setF(estadoInicial->getStepUsed() + h);
    estadoInicial->setParent(nullptr);
    openSet->push(estadoInicial);

    while (!openSet->estaVacio()) {
        Estado* actual = openSet->pop();

        if (closedSet->existe(actual)) {
            delete actual;
            continue;
        }

        if (actual->jugoTerminado(tablero->getNumPiezas())) {
            return reconstruirCamino(actual);
        }

        if (actual->getStepUsed() >= tablero->getStepLimit()) {
            delete actual;
            continue;
        }

        closedSet->insertar(actual);

        int numVecinos = generarVecinos(actual);
        for (int i = 0; i < numVecinos; i++) {
            if (!closedSet->existe(vecinosTemp[i]))
                openSet->push(vecinosTemp[i]);
            else
                delete vecinosTemp[i];
        }
    }
    return nullptr;
}
int Solver::calcularHeuristica(const Estado& estado) const {
    int maxDist = 0;

    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos = estado.getPosPiezas()[i];
        Pieza& pieza   = tablero->getPiezas()[i];
        int mejorDist  = -1;

        for (int j = 0; j < tablero->getNumSalidas(); j++) {
            Salida& salida = tablero->getSalidas()[j];
            if (salida.getColor() != pieza.getColor()) continue;
            if (!tablero->piezaPodriaSalir(pieza, salida)) continue;

            int dist = abs(pos.x - salida.getPos().x) 
                     + abs(pos.y - salida.getPos().y);
            if (mejorDist == -1 || dist < mejorDist)
                mejorDist = dist;
        }

        if (mejorDist == -1){
            mejorDist = tablero->getW() + tablero->getH();
        }

        int distConBloqueos = mejorDist + contarBloqueos(i, pos, estado);
        if (distConBloqueos > maxDist) {
            maxDist = distConBloqueos;
        }
    }
    return maxDist;
}

int Solver::contarBloqueos(int idPieza, coordenada pos, 
                             const Estado& estado) const {
    Pieza& pieza   = tablero->getPiezas()[idPieza];
    coordenada posSalida = {0, 0};

    // encontrar la salida más cercana (la misma que usó calcularHeuristica)
    int mejorDist = -1;
    for (int j = 0; j < tablero->getNumSalidas(); j++) {
        Salida& salida = tablero->getSalidas()[j];
        if (salida.getColor() != pieza.getColor()) continue;
        if (!tablero->piezaPodriaSalir(pieza, salida)) continue;
        int d = abs(pos.x - salida.getPos().x) + abs(pos.y - salida.getPos().y);
        
        if (mejorDist == -1 || d < mejorDist) {
            mejorDist = d;
            posSalida = salida.getPos();
        }
    }
    if (mejorDist <= 0) return 0;

    int bloqueos = 0;
    short* ocupacion = estado.getOcupacion();
    int w = tablero->getW();

    // barrer las celdas en línea recta entre pieza y salida
    // movimiento horizontal
    if (pos.y == posSalida.y) {
        int xMin = (pos.x < posSalida.x) ? pos.x + 1 : posSalida.x;
        int xMax = (pos.x < posSalida.x) ? posSalida.x : pos.x - 1;
        for (int x = xMin; x <= xMax; x++) {
            short ocupante = ocupacion[pos.y * w + x];
            if (ocupante != -1 && ocupante != idPieza)
                bloqueos++;
        }
    }
    // movimiento vertical
    else if (pos.x == posSalida.x) {
        int yMin = (pos.y < posSalida.y) ? pos.y + 1 : posSalida.y;
        int yMax = (pos.y < posSalida.y) ? posSalida.y : pos.y - 1;
        for (int y = yMin; y <= yMax; y++) {
            short ocupante = ocupacion[y * w + pos.x];
            if (ocupante != -1 && ocupante != idPieza)
                bloqueos++;
        }
    }

    return bloqueos;
}
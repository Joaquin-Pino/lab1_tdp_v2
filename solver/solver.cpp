// solver.cpp
#include "solver.h"
#include <cstring>
#include <iostream>

Solver::Solver(Tablero* t)
    : tablero(t), openSet(nullptr), closedSet(nullptr), vecinosTemp(nullptr) {
    int maxDim = (t->getW() > t->getH()) ? t->getW() : t->getH();
    maxVecinos  = t->getNumPiezas() * (4 * maxDim + 1);
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

bool Solver::esMovimientoRedundante(int idPieza, char dir, const Estado* actual) const {
    const char* prev = actual->getMovimiento();
    if (prev[0] == '\0' || prev[0] == 'S') return false;

    char prevDir = prev[0];
    int prevPiezaId = -1;
    sscanf(prev + 1, "%d", &prevPiezaId);

    Pieza& pieza = tablero->getPiezas()[idPieza];
    if (pieza.getId() != prevPiezaId) return false;

    // poda: deshacer movimiento previo de la misma pieza es siempre redundante
    if ((dir == 'U' && prevDir == 'D') || (dir == 'D' && prevDir == 'U') ||
        (dir == 'L' && prevDir == 'R') || (dir == 'R' && prevDir == 'L')) return true;

    return false;
}

void Solver::prepararVecino(Estado* vecino, Estado* actual, char mov[10]) const {
    for (int i = 0; i < tablero->getNumCompuertas(); i++)
        vecino->actualizarCompuerta(i, tablero->calcularColorCompuerta(i, *vecino));
    for (int i = 0; i < tablero->getNumSalidas(); i++)
        vecino->actualizarSalida(i, tablero->calcularLargoSalida(i, *vecino));

    int hv = calcularHeuristica(*vecino);
    vecino->setH(hv);
    vecino->setF(vecino->getStepUsed() + hv);
    vecino->setParent(actual);
    vecino->setMovimiento(mov);
}

int Solver::generarVecinos(Estado* actual) {
    int count = 0;
    int numPiezas = tablero->getNumPiezas();
    int w = tablero->getW();
    Pieza* piezas = tablero->getPiezas();

    direccion dirs[4]   = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};
    int       dx[4]     = { 0,  0, -1,  1};
    int       dy[4]     = {-1,  1,  0,  0};
    char      dirChar[4]= {'U', 'D', 'L', 'R'};

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue;
        Pieza& pieza = piezas[id];

        if (tablero->piezaPuedeSalir(id, *actual)) {
            Estado* vecino = actual->clonarYSacar(id, pieza, w);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId());
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count;
            continue;
        }

        for (int d = 0; d < 4; d++) {
            //if (esMovimientoRedundante(id, dirChar[d], actual)) continue;
            if (!tablero->piezaPuedeMoverse(id, dirs[d], *actual)) continue;

            Estado* vecino = actual->clonarYMover(id, dx[d], dy[d], pieza, w);

            char mov[10];
            snprintf(mov, 10, "%c%d,1", dirChar[d], pieza.getId());
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count;
        }
    }
    return count;
}

Estado** Solver::reconstruirCamino(Estado* final) {
    int numPasos = 0;
    Estado* actual = final;
    while (actual != nullptr) {
        numPasos++;
        actual = actual->getParent();
    }

    Estado** camino = new Estado*[numPasos + 1];

    actual = final;
    for (int i = numPasos - 1; i >= 0; i--) {
        camino[i] = new Estado(*actual);
        camino[i]->setParent(nullptr);
        actual = actual->getParent();
    }

    camino[numPasos] = nullptr;
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
            Estado** camino = reconstruirCamino(actual);
            delete actual;
            return camino;
        }

        if (actual->getStepUsed() >= tablero->getStepLimit()) {
            delete actual;
            continue;
        }

        closedSet->insertar(actual);

        // poda por f: descarta vecinos que ya no pueden llegar a meta.
        // El margen tolera la sobreestimación de la heurística por bloqueos.
        int margen = tablero->getNumPiezas() / 2 + 1;

        int numVecinos = generarVecinos(actual);
        for (int i = 0; i < numVecinos; i++) {
            if (vecinosTemp[i]->getStepUsed() > tablero->getStepLimit()) {
                delete vecinosTemp[i];
                continue;
            }
            if (vecinosTemp[i]->getF() > tablero->getStepLimit() + margen) {
                delete vecinosTemp[i];
                continue;
            }
            if (!closedSet->existe(vecinosTemp[i]))
                openSet->push(vecinosTemp[i]);
            else
                delete vecinosTemp[i];
        }
        actual->eliminarOcupacion();
    }
    return nullptr;
}
int Solver::calcularHeuristica(const Estado& estado) const {
    int total = 0;

    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos = estado.getPosPiezas()[i];
        Pieza& pieza   = tablero->getPiezas()[i];
        int pw = pieza.getAncho();
        int ph = pieza.getAlto();

        int mejorCosto = -1;

        for (int j = 0; j < tablero->getNumSalidas(); j++) {
            Salida& salida = tablero->getSalidas()[j];
            if (salida.getColor() != pieza.getColor()) continue;
            if (!tablero->piezaPodriaSalir(pieza, salida)) continue;

            coordenada ps = salida.getPos();

            int dx = 0, dy = 0;
            if (ps.x < pos.x) dx = pos.x - ps.x;
            else if (ps.x > pos.x + pw - 1) dx = ps.x - (pos.x + pw - 1);

            if (ps.y < pos.y) dy = pos.y - ps.y;
            else if (ps.y > pos.y + ph - 1) dy = ps.y - (pos.y + ph - 1);

            int dist = dx + dy;

            int bloqueos = contarBloqueos(i, pos, ps, estado);
            int costo = dist + bloqueos / 2;

            if (mejorCosto == -1 || costo < mejorCosto)
                mejorCosto = costo;
        }

        if (mejorCosto == -1) {
            mejorCosto = 0;
        }

        total += mejorCosto;
    }
    return total;
}

int Solver::contarBloqueos(int idPieza, coordenada pos,
                             const Estado& estado) const {
    Pieza& pieza = tablero->getPiezas()[idPieza];
    coordenada mejorSalida = {0, 0};
    int mejorDist = -1;
    for (int j = 0; j < tablero->getNumSalidas(); j++) {
        Salida& salida = tablero->getSalidas()[j];
        if (salida.getColor() != pieza.getColor()) continue;
        if (!tablero->piezaPodriaSalir(pieza, salida)) continue;
        int d = abs(pos.x - salida.getPos().x) + abs(pos.y - salida.getPos().y);
        if (mejorDist == -1 || d < mejorDist) {
            mejorDist = d;
            mejorSalida = salida.getPos();
        }
    }
    if (mejorDist <= 0) return 0;
    return contarBloqueos(idPieza, pos, mejorSalida, estado);
}

int Solver::contarBloqueos(int idPieza, coordenada pos, coordenada posSalida,
                             const Estado& estado) const {
    int w = tablero->getW();
    short* ocupacion = estado.getOcupacion();
    celda* matriz = tablero->getMatriz();

    int xMin = pos.x < posSalida.x ? pos.x : posSalida.x;
    int xMax = pos.x < posSalida.x ? posSalida.x : pos.x;
    int yMin = pos.y < posSalida.y ? pos.y : posSalida.y;
    int yMax = pos.y < posSalida.y ? posSalida.y : pos.y;

    int bloqueosHorizPrimero = 0;
    {
        int xIni = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin = posSalida.x;
        int paso = pos.x < posSalida.x ? 1 : -1;
        short ultimoId = idPieza;
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[pos.y * w + x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[pos.y * w + x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; }
        }
        int yIni = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        ultimoId = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + posSalida.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + posSalida.x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; }
        }
    }

    int bloqueosVertPrimero = 0;
    {
        int yIni = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        short ultimoId = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + pos.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + pos.x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
        int xIni = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin = posSalida.x;
        int paso = pos.x < posSalida.x ? 1 : -1;
        ultimoId = idPieza;
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[posSalida.y * w + x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[posSalida.y * w + x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
    }

    return bloqueosHorizPrimero < bloqueosVertPrimero ? bloqueosHorizPrimero : bloqueosVertPrimero;
}
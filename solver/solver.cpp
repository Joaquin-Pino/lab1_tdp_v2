// solver.cpp
#include "solver.h"
#include <cstring>
#include <iostream>

Solver::Solver(Tablero* t)
    : tablero(t), openSet(nullptr), closedSet(nullptr), vecinosTemp(nullptr) {
    int maxDim = (t->getW() > t->getH()) ? t->getW() : t->getH();
    (void)maxDim;
    maxVecinos  = t->getNumPiezas() * 5;
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
    int w         = tablero->getW();
    int hTab      = tablero->getH();
    celda* matriz = tablero->getMatriz();
    Pieza* piezas = tablero->getPiezas();
    Compuerta* compuertas = tablero->getCompuertas();

    const int dx[4]       = { 0,  0, -1,  1};
    const int dy[4]       = {-1,  1,  0,  0};
    const char dirChar[4] = {'U','D','L','R'};

    short* ocupacion = actual->getOcupacion();

    const char* prev = actual->getMovimiento();
    int prevPieza = -1;
    char prevDir = 0;
    if (prev[0] != '\0' && prev[0] != 'S') {
        prevDir = prev[0];
        sscanf(prev + 1, "%d", &prevPieza);
    }

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue;

        Pieza& pieza = piezas[id];
        coordenada posOrig = actual->getPosPiezas()[id];
        int pw = pieza.getAncho();
        int ph = pieza.getAlto();

        // si la pieza puede salir
        if (tablero->piezaPuedeSalir(id, *actual)) {
            Estado* vecino = new Estado(*actual);
            vecino->sacarPieza(id, pieza, w);

            int hv = calcularHeuristica(*vecino);
            vecino->setH(hv);
            vecino->setF(vecino->getStepUsed() + hv);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId());
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count;
            continue;
        }

        for (int d = 0; d < 4; d++) {
            if (id == prevPieza) {
                if (dirChar[d] == prevDir) continue;
                if ((dirChar[d] == 'U' && prevDir == 'D') ||
                    (dirChar[d] == 'D' && prevDir == 'U') ||
                    (dirChar[d] == 'L' && prevDir == 'R') ||
                    (dirChar[d] == 'R' && prevDir == 'L')) continue;
            }

            int newX = posOrig.x + dx[d];
            int newY = posOrig.y + dy[d];

            bool bloqueado = false;
            for (int i = 0; i < ph && !bloqueado; i++) {
                for (int j = 0; j < pw && !bloqueado; j++) {
                    if (!pieza.getCelda(j, i)) continue;

                    int fila    = newY + i;
                    int columna = newX + j;

                    if (fila < 0 || fila >= hTab || columna < 0 || columna >= w) {
                        bloqueado = true; break;
                    }

                    int idx = fila * w + columna;

                    short ocup = ocupacion[idx];
                    if (ocup != -1 && ocup != id) {
                        bloqueado = true; break;
                    }

                    celda& c = matriz[idx];
                    if (c.tipo == PARED) {
                        bloqueado = true; break;
                    }

                    if (c.tipo == COMPUERTA) {
                        Compuerta& cp = compuertas[c.id];
                        int colorCompuerta;
                        if (cp.getPaso() == 0) {
                            colorCompuerta = cp.getCi();
                        } else {
                            int pasoEvaluacion = actual->getStepUsed() + 1;
                            int ciclo = cp.getCf() - cp.getCi() + 1;
                            int pc = pasoEvaluacion / cp.getPaso();
                            colorCompuerta = cp.getCi() + (pc % ciclo);
                        }
                        int tamanoBloque = (d == 0 || d == 1) ? pw : ph;
                        if (!cp.aceptaBloque(pieza.getColor(), tamanoBloque, colorCompuerta)) {
                            bloqueado = true; break;
                        }
                    }
                }
            }

            if (bloqueado) continue;

            Estado* vecino = new Estado(*actual);
            coordenada* posPiezasV = vecino->getPosPiezas();
            short* ocupV = vecino->getOcupacion();

            for (int i = 0; i < ph; i++) {
                for (int j = 0; j < pw; j++) {
                    if (!pieza.getCelda(j, i)) continue;
                    ocupV[(posOrig.y + i) * w + (posOrig.x + j)] = -1;
                }
            }

            posPiezasV[id].x = newX;
            posPiezasV[id].y = newY;

            for (int i = 0; i < ph; i++) {
                for (int j = 0; j < pw; j++) {
                    if (!pieza.getCelda(j, i)) continue;
                    ocupV[(newY + i) * w + (newX + j)] = id;
                }
            }

            vecino->setStepUsed(actual->getStepUsed() + 1);

            for (int i = 0; i < tablero->getNumCompuertas(); i++)
                vecino->actualizarCompuerta(i,
                    tablero->calcularColorCompuerta(i, *vecino));
            for (int i = 0; i < tablero->getNumSalidas(); i++)
                vecino->actualizarSalida(i,
                    tablero->calcularLargoSalida(i, *vecino));

            int hv = calcularHeuristica(*vecino);
            vecino->setH(hv);
            vecino->setF(vecino->getStepUsed() + hv);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "%c%d,1", dirChar[d], pieza.getId());
            vecino->setMovimiento(mov);

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

        int numVecinos = generarVecinos(actual);
        for (int i = 0; i < numVecinos; i++) {
            if (vecinosTemp[i]->getStepUsed() > tablero->getStepLimit()) {
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
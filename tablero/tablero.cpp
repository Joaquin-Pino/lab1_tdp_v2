#include "tablero.h"
#include <iostream>

// ─────────────────────────────────────────
// Constructores y destructor
// ─────────────────────────────────────────

Tablero::Tablero() : matriz(nullptr), piezas(nullptr), salidas(nullptr),
    compuertas(nullptr), numPiezas(0), numSalidas(0),
    numCompuertas(0), w(0), h(0), stepLimit(0) {}

// toma ownership de todos los punteros
Tablero::Tablero(celda* matriz, Pieza* piezas, Salida* salidas,
                 Compuerta* compuertas, int numPiezas, int numSalidas,
                 int numCompuertas, int w, int h, int stepLimit)
    : matriz(matriz), piezas(piezas), salidas(salidas), compuertas(compuertas),
      numPiezas(numPiezas), numSalidas(numSalidas), numCompuertas(numCompuertas),
      w(w), h(h), stepLimit(stepLimit) {}

Tablero::Tablero(const Tablero& otro)
    : numPiezas(otro.numPiezas), numSalidas(otro.numSalidas),
      numCompuertas(otro.numCompuertas), w(otro.w), h(otro.h),
      stepLimit(otro.stepLimit) {

    matriz = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        matriz[i] = otro.matriz[i];

    piezas = new Pieza[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        piezas[i] = otro.piezas[i];

    salidas = new Salida[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        salidas[i] = otro.salidas[i];

    compuertas = new Compuerta[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        compuertas[i] = otro.compuertas[i];
}

Tablero& Tablero::operator=(const Tablero& otro) {
    if (this == &otro) return *this;

    delete[] matriz;
    delete[] piezas;
    delete[] salidas;
    delete[] compuertas;

    numPiezas = otro.numPiezas;
    numSalidas = otro.numSalidas;
    numCompuertas = otro.numCompuertas;
    w = otro.w;
    h = otro.h;
    stepLimit = otro.stepLimit;

    matriz = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        matriz[i] = otro.matriz[i];

    piezas = new Pieza[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        piezas[i] = otro.piezas[i];

    salidas = new Salida[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        salidas[i] = otro.salidas[i];

    compuertas = new Compuerta[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        compuertas[i] = otro.compuertas[i];

    return *this;
}

Tablero::~Tablero() {
    delete[] matriz;
    delete[] piezas;
    delete[] salidas;
    delete[] compuertas;
}

// ─────────────────────────────────────────
// Getters
// ─────────────────────────────────────────

celda* Tablero::getMatriz() const { return matriz; }
Pieza* Tablero::getPiezas() const { return piezas; }
Salida* Tablero::getSalidas() const { return salidas; }
Compuerta* Tablero::getCompuertas() const { return compuertas; }
int Tablero::getNumPiezas() const { return numPiezas; }
int Tablero::getNumSalidas() const { return numSalidas; }
int Tablero::getNumCompuertas() const { return numCompuertas; }
int Tablero::getW() const { return w; }
int Tablero::getH() const { return h; }
int Tablero::getStepLimit() const { return stepLimit; }

// ─────────────────────────────────────────
// Lógica del juego
// ─────────────────────────────────────────

bool Tablero::piezaPuedeMoverse(int id, direccion dir, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];

    for (int i = 0; i < pieza.getAlto(); i++) {
        for (int j = 0; j < pieza.getAncho(); j++) {
            if (!pieza.getCelda(j, i)) continue;

            int fila = pos.y + i;
            int columna = pos.x + j;

            switch (dir) {
                case ARRIBA:    fila--;    break;
                case ABAJO:     fila++;    break;
                case IZQUIERDA: columna--; break;
                case DERECHA:   columna++; break;
            }

            if (fila < 0 || fila >= h || columna < 0 || columna >= w)
                return false;

            // verificar obstáculos estáticos en la matriz
            celda& c = matriz[fila * w + columna];
            if (c.tipo == PARED) return false;

            if (c.tipo == COMPUERTA) {
                int colorActual = calcularColorCompuerta(c.id, estado);
                int tamano = (dir == ARRIBA || dir == ABAJO) ? pieza.getAncho() : pieza.getAlto();
                if (!compuertas[c.id].aceptaBloque(pieza.getColor(), tamano, colorActual))
                    return false;
            }

            // verificar colisión con otras piezas desde el estado
            // TODO: ver si esta comprobacion se puede hacer en log(n)
            for (int otraId = 0; otraId < numPiezas; otraId++) {
                if (otraId == id) continue;
                if (estado.piezaYaSalio(otraId)) continue;

                Pieza& otra = piezas[otraId]; // para mirar info estatica de la pieza
                coordenada posOtra = estado.getPosPiezas()[otraId];

                for (int oi = 0; oi < otra.getAlto(); oi++) {
                    for (int oj = 0; oj < otra.getAncho(); oj++) {
                        if (!otra.getCelda(oj, oi)) continue;
                        int filaOtra    = posOtra.y + oi;
                        int columnaOtra = posOtra.x + oj;
                        if (filaOtra == fila && columnaOtra == columna)
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

bool Tablero::piezaPuedeSalir(int id, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];

    // verificar los 4 bordes de la pieza
    int dx[] = { 0,  0, -1,  1};
    int dy[] = {-1,  1,  0,  0};

    for (int dir = 0; dir < 4; dir++) {
        for (int i = 0; i < pieza.getAlto(); i++) {
            for (int j = 0; j < pieza.getAncho(); j++) {
                if (!pieza.getCelda(j, i)) continue;

                int fila = pos.y + i + dy[dir];
                int columna = pos.x + j + dx[dir];

                if (fila < 0 || fila >= h || columna < 0 || columna >= w) continue;

                celda& c = matriz[fila * w + columna];
                if (c.tipo != SALIDA) continue;

                Salida& salida = salidas[c.id];
                if (salida.getColor() != pieza.getColor()) continue;

                int largoActual = calcularLargoSalida(c.id, estado);
                int tamano = salida.getEsHorizontal() ?
                             pieza.getAlto() : pieza.getAncho();

                if (salida.aceptaBloque(tamano, largoActual)) return true;
            }
        }
    }
    return false;
}

int Tablero::calcularColorCompuerta(int idCompuerta, const Estado& estado) {
    return estado.getColorCompuertas()[idCompuerta];
}

int Tablero::calcularLargoSalida(int idSalida, const Estado& estado) {
    return estado.getLargoSalidas()[idSalida];
}

int Tablero::calcularHeuristica(const Estado& estado) {
    int total = 0;
    for (int i = 0; i < numPiezas; i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos = estado.getPosPiezas()[i];

        // buscar la salida del color de esta pieza
        for (int j = 0; j < numSalidas; j++) {
            // TODO: se debe verificar que la pieza eventualmente pueda salir por la salida, no solo que tengan el mismo color. 
            // para mejorar la heuristica
            if (salidas[j].getColor() == piezas[i].getColor()) {
                coordenada posSalida = salidas[j].getPos();
                // distancia Manhattan
                total += (pos.x > posSalida.x ? pos.x - posSalida.x : posSalida.x - pos.x)
                       + (pos.y > posSalida.y ? pos.y - posSalida.y : posSalida.y - pos.y);
                break;
            }
        }
    }
    return total;
}

Estado* Tablero::crearEstadoInicial() const {
    coordenada* posiciones = new coordenada[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        posiciones[i] = piezas[i].getPosInicial();

    // color inicial de cada compuerta viene de ci
    int* coloresCompuertas = new int[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        coloresCompuertas[i] = compuertas[i].getCi();

    // largo inicial de cada salida viene de Li
    short* largosSalidas = new short[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        largosSalidas[i] = salidas[i].getLi();

    Estado* estado = new Estado(numPiezas, numCompuertas, numSalidas,
                                posiciones, coloresCompuertas, largosSalidas,
                                0, 0, 0, nullptr, "");
    delete[] posiciones;
    delete[] coloresCompuertas;
    delete[] largosSalidas;

    return estado;
}

void Tablero::imprimir() const {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            celda& c = matriz[i * w + j];
            switch (c.tipo) {
                case VACIA:     std::cout << "."; break;
                case PARED:     std::cout << "#"; break;
                case COMPUERTA: std::cout << "C"; break;
                case PIEZA:     std::cout << "B"; break;
                case SALIDA:    std::cout << "S"; break;
                default:        std::cout << "?"; break;
            }
        }
        std::cout << "\n";
    }
}
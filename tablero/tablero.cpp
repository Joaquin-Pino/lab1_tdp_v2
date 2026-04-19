#include "tablero.h"
#include <iostream>
#include <climits>


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



// TODO: optimizar
bool Tablero::piezaPuedeMoverse(int id, direccion dir, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];

    // verificamos cada celda de la pieza que se movería para ver si el movimiento es válido
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

            // verificar limites del tablero
            if (fila <= 0 || fila >= h || columna <= 0 || columna >= w)
                return false;

            // verificar obstáculos estáticos en la matriz
            celda& c = matriz[fila * w + columna];
            if (c.tipo == PARED) return false;

            // si se quiere mover a compuerta, verificar si la pieza puede pasar por esa compuerta
            if (c.tipo == COMPUERTA) {
                int colorActual = calcularColorCompuerta(c.id, estado);
                int tamano = (dir == ARRIBA || dir == ABAJO) ? pieza.getAncho() : pieza.getAlto();
                
                // vemos si pieza puede pasar por la compuerta, verificando tamano y color del bloque
                if (!compuertas[c.id].aceptaBloque(pieza.getColor(), tamano, colorActual))
                    return false;
            }

            // verificar colisión con otras piezas desde el estado
            // TODO: ver si esta comprobacion se puede hacer en log(n)
            // vamos por todas las otras piezas y vemos si alguna ocupa la celda a la que queremos movernos
            for (int otraId = 0; otraId < numPiezas; otraId++) {
                if (otraId == id) continue;
                if (estado.piezaYaSalio(otraId)) continue;

                Pieza& otra = piezas[otraId]; // para mirar info estatica de la pieza
                coordenada posOtra = estado.getPosPiezas()[otraId];
                
                // revisamos cada celda de la pieza otra para ver si alguna coincide con la celda a la que queremos movernos
                for (int oi = 0; oi < otra.getAlto(); oi++) {
                    for (int oj = 0; oj < otra.getAncho(); oj++) {
                        // celda sin geometria
                        if (!otra.getCelda(oj, oi)) continue;
                        
                        int filaOtra = posOtra.y + oi;
                        int columnaOtra = posOtra.x + oj;

                        // se estan topando en la celda a la que queremos movernos
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

    // solo verificar celdas que están realmente en el borde de la pieza
    // borde superior: solo fila 0 de la pieza
    for (int j = 0; j < pieza.getAncho(); j++) {
        if (!pieza.getCelda(j, 0)) continue;
        int fila    = pos.y - 1;
        int columna = pos.x + j;
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // borde inferior: solo última fila de la pieza
    for (int j = 0; j < pieza.getAncho(); j++) {
        if (!pieza.getCelda(j, pieza.getAlto()-1)) continue;
        int fila    = pos.y + pieza.getAlto();
        int columna = pos.x + j;
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // borde izquierdo: solo columna 0 de la pieza
    for (int i = 0; i < pieza.getAlto(); i++) {
        if (!pieza.getCelda(0, i)) continue;
        int fila    = pos.y + i;
        int columna = pos.x - 1;
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // borde derecho: solo última columna de la pieza
    for (int i = 0; i < pieza.getAlto(); i++) {
        if (!pieza.getCelda(pieza.getAncho()-1, i)) continue;
        int fila    = pos.y + i;
        int columna = pos.x + pieza.getAncho();
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    return false;
}

// bool Tablero::piezaPuedeSalir(int id, const Estado& estado) {
//     Pieza& pieza = piezas[id];
//     coordenada pos = estado.getPosPiezas()[id];

//     // borde derecho
//     for (int i = 0; i < pieza.getAlto(); i++) {
//         if (!pieza.getCelda(pieza.getAncho()-1, i)) continue;
//         int fila    = pos.y + i;
//         int columna = pos.x + pieza.getAncho();
//         std::cout << "DEBUG borde derecho fila=" << fila 
//                   << " columna=" << columna 
//                   << " tipo=" << matriz[fila * w + columna].tipo << std::endl;
//         if (fila >= 0 && fila < h && columna >= 0 && columna < w)
//             if (esSalidaValida(fila, columna, pieza, estado)) return true;
//     }
//     return false;
// }

bool Tablero::esSalidaValida(int fila, int columna, 
                              const Pieza& pieza, const Estado& estado) {
    celda& c = matriz[fila * w + columna];
    Salida& salida = salidas[c.id];
    
    //celda& c = matriz[fila * w + columna];
    if (c.tipo != SALIDA) return false;

    //Salida& salida = salidas[c.id];
    // comprobamos color
    if (salida.getColor() != pieza.getColor()) return false;

    int largoActual = calcularLargoSalida(c.id, estado);
    int tamano = salida.getEsHorizontal() ? pieza.getAncho() : pieza.getAlto();
    
    return salida.aceptaBloque(tamano, largoActual);
}

int Tablero::calcularColorCompuerta(int idx, const Estado& estado) {
    // usar stepUsed para calcular el color actual
    Compuerta& c = compuertas[idx];
    if (c.getPaso() == 0) return c.getCi();
    int ciclo = c.getCf() - c.getCi() + 1;
    int pasosCiclo = estado.getStepUsed() / c.getPaso();
    return c.getCi() + (pasosCiclo % ciclo);
}

int Tablero::calcularLargoSalida(int idx, const Estado& estado) {
    Salida& s = salidas[idx];
    if (s.getLi() == s.getLf()) return s.getLi();
    int rango = s.getLf() - s.getLi();
    int pasosCiclo = estado.getStepUsed() / s.getPaso();
    int pos = pasosCiclo % (2 * rango);
    if (pos <= rango)
        return s.getLi() + pos;
    else
        return s.getLf() - (pos - rango);
}

int Tablero::calcularHeuristica(const Estado& estado) {
    int total = 0;
    for (int i = 0; i < numPiezas; i++) {
        if (estado.piezaYaSalio(i)) continue;

        coordenada pos = estado.getPosPiezas()[i];

        // buscar la salida del color de esta pieza
        int distMin = INT_MAX;
        for (int j = 0; j < numSalidas; j++) {
            if (salidas[j].getColor() != piezas[i].getColor()) continue;
            coordenada posSalida = salidas[j].getPos();
            int dist = (pos.x > posSalida.x ? pos.x - posSalida.x : posSalida.x - pos.x)
                    + (pos.y > posSalida.y ? pos.y - posSalida.y : posSalida.y - pos.y);
            if (dist < distMin) distMin = dist;
        }

        if (distMin != INT_MAX) total += distMin;
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
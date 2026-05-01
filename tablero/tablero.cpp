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

celda* Tablero::getCopiaMatriz() const {
    celda* copia = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        copia[i] = matriz[i];
    return copia;
}

Pieza* Tablero::getPiezas() const { return piezas; }
Salida* Tablero::getSalidas() const { return salidas; }
Compuerta* Tablero::getCompuertas() const { return compuertas; }
int Tablero::getNumPiezas() const { return numPiezas; }
int Tablero::getNumSalidas() const { return numSalidas; }
int Tablero::getNumCompuertas() const { return numCompuertas; }
int Tablero::getW() const { return w; }
int Tablero::getH() const { return h; }
int Tablero::getStepLimit() const { return stepLimit; }

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
            if (fila < 0 || fila >= h || columna < 0 || columna >= w)
                return false;
            
            // verificamos en matriz de ocupacion del estado para ver si la celda está ocupada por otra pieza
            short* ocupacion = estado.getOcupacion();
            // si la celda a la que queremos movernos está ocupada por otra pieza (id diferente y no vacía)
            if (ocupacion[fila * w + columna] != -1 && ocupacion[fila * w + columna] != id) {
                return false;
            }
                
            // verificar obstáculos estáticos en la matriz
            celda& c = matriz[fila * w + columna];
            if (c.tipo == PARED) return false;

            // si se quiere mover a compuerta, verificar si la pieza puede pasar por esa compuerta
            
            if (c.tipo == COMPUERTA) {
                // TODO: verificar orientacion de compuerta
                
                int colorActual = calcularColorCompuerta(c.id, estado);
                // std::cout << "DEBUG Intentando mover pieza " << id << " a compuerta " << c.id
                //           << " con color actual " << colorActual << "Color de la pieza: " << pieza.getColor() << std::endl;
                int tamano = (dir == ARRIBA || dir == ABAJO) ? pieza.getAncho() : pieza.getAlto();
                
                // vemos si pieza puede pasar por la compuerta, verificando tamano y color del bloque
                if (!compuertas[c.id].aceptaBloque(pieza.getColor(), tamano, colorActual))
                    return false;
            }
        }
    }

    return true;
}

bool Tablero::piezaPuedeSalir(int id, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];
    int pw = pieza.getAncho();
    int ph = pieza.getAlto();

    // --- borde superior: todas las celdas de geometria en fila 0 deben mirar
    //     a la MISMA salida, y el ancho de la pieza debe caber en su largo ---
    {
        int idSalida = -1;
        bool valido = true;
        bool algunaCelda = false;
        for (int j = 0; j < pw && valido; j++) {
            if (!pieza.getCelda(j, 0)) continue;
            algunaCelda = true;
            int fila    = pos.y - 1;
            int columna = pos.x + j;
            if (fila < 0 || fila >= h || columna < 0 || columna >= w) {
                valido = false; break;
            }
            celda& c = matriz[fila * w + columna];
            if (c.tipo != SALIDA) { valido = false; break; }
            if (idSalida == -1) idSalida = c.id;
            else if (c.id != idSalida) { valido = false; break; }
        }
        if (valido && algunaCelda && idSalida != -1) {
            Salida& s = salidas[idSalida];
            if (s.getColor() == pieza.getColor() && s.getEsHorizontal()) {
                int largoActual = calcularLargoSalida(idSalida, estado);
                if (pw <= largoActual) return true;
            }
        }
    }

    // --- borde inferior ---
    {
        int idSalida = -1;
        bool valido = true;
        bool algunaCelda = false;
        for (int j = 0; j < pw && valido; j++) {
            if (!pieza.getCelda(j, ph - 1)) continue;
            algunaCelda = true;
            int fila    = pos.y + ph;
            int columna = pos.x + j;
            if (fila < 0 || fila >= h || columna < 0 || columna >= w) {
                valido = false; break;
            }
            celda& c = matriz[fila * w + columna];
            if (c.tipo != SALIDA) { valido = false; break; }
            if (idSalida == -1) idSalida = c.id;
            else if (c.id != idSalida) { valido = false; break; }
        }
        if (valido && algunaCelda && idSalida != -1) {
            Salida& s = salidas[idSalida];
            if (s.getColor() == pieza.getColor() && s.getEsHorizontal()) {
                int largoActual = calcularLargoSalida(idSalida, estado);
                if (pw <= largoActual) return true;
            }
        }
    }

    // --- borde izquierdo ---
    {
        int idSalida = -1;
        bool valido = true;
        bool algunaCelda = false;
        for (int i = 0; i < ph && valido; i++) {
            if (!pieza.getCelda(0, i)) continue;
            algunaCelda = true;
            int fila    = pos.y + i;
            int columna = pos.x - 1;
            if (fila < 0 || fila >= h || columna < 0 || columna >= w) {
                valido = false; break;
            }
            celda& c = matriz[fila * w + columna];
            if (c.tipo != SALIDA) { valido = false; break; }
            if (idSalida == -1) idSalida = c.id;
            else if (c.id != idSalida) { valido = false; break; }
        }
        if (valido && algunaCelda && idSalida != -1) {
            Salida& s = salidas[idSalida];
            if (s.getColor() == pieza.getColor() && !s.getEsHorizontal()) {
                int largoActual = calcularLargoSalida(idSalida, estado);
                if (ph <= largoActual) return true;
            }
        }
    }

    // --- borde derecho ---
    {
        int idSalida = -1;
        bool valido = true;
        bool algunaCelda = false;
        for (int i = 0; i < ph && valido; i++) {
            if (!pieza.getCelda(pw - 1, i)) continue;
            algunaCelda = true;
            int fila    = pos.y + i;
            int columna = pos.x + pw;
            if (fila < 0 || fila >= h || columna < 0 || columna >= w) {
                valido = false; break;
            }
            celda& c = matriz[fila * w + columna];
            if (c.tipo != SALIDA) { valido = false; break; }
            if (idSalida == -1) idSalida = c.id;
            else if (c.id != idSalida) { valido = false; break; }
        }
        if (valido && algunaCelda && idSalida != -1) {
            Salida& s = salidas[idSalida];
            if (s.getColor() == pieza.getColor() && !s.getEsHorizontal()) {
                int largoActual = calcularLargoSalida(idSalida, estado);
                if (ph <= largoActual) return true;
            }
        }
    }

    return false;
}


bool Tablero::esSalidaValida(int fila, int columna, 
                              const Pieza& pieza, const Estado& estado) {
    celda& c = matriz[fila * w + columna];
    Salida& salida = salidas[c.id];
    
    if (c.tipo != SALIDA) return false;

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

int Tablero::calcularLargoSalida(int idx, const Estado& estado) const {
    Salida& s = salidas[idx];
    //salida que no cambia de largo
    if (s.getLi() == s.getLf()) return s.getLi();
    
    int rango = s.getLf() - s.getLi();
    int pasosCiclo = estado.getStepUsed() / s.getPaso();
    int pos = pasosCiclo % (2 * rango);
    if (pos <= rango)
        return s.getLi() + pos;
    else
        return s.getLf() - (pos - rango);
}

// esto se deberia encargar solver
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

    // marcar ocupacion inicial del tablero según las piezas
    short* ocupacion = new short[w * h];
    for (int i = 0; i < w * h; i++) {
        ocupacion[i] = -1;
    } // -1 significa sin pieza

    for (int id = 0; id < numPiezas; id++) {
        Pieza& pieza = piezas[id];
        coordenada pos = posiciones[id];
        for (int i = 0; i < pieza.getAlto(); i++) {
            for (int j = 0; j < pieza.getAncho(); j++) {
                if (!pieza.getCelda(j, i)) continue;
                int fila = pos.y + i;
                int columna = pos.x + j;
                ocupacion[fila * w + columna] = id;
            }
        }
    }

    Estado* estado = new Estado(numPiezas, numCompuertas, numSalidas,
                                posiciones, coloresCompuertas, largosSalidas,
                                0, 0, 0, w, h, nullptr, "", ocupacion);
    delete[] posiciones;
    delete[] coloresCompuertas;
    delete[] largosSalidas;
    delete[] ocupacion;

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

bool Tablero::piezaPodriaSalir(Pieza& pieza, Salida& salida) const {
    // esta función se encarga de verificar si una pieza podría salir por una salida dada su posición actual
    // se asume que la pieza ya está adyacente a la salida y que el color coincide (verificado antes)
    int tamanoPieza= salida.getEsHorizontal() ? pieza.getAncho() : pieza.getAlto();
    int largoEventual = (salida.getLi() > salida.getLf()) ? salida.getLi() : salida.getLf();
    return tamanoPieza <= largoEventual;
}
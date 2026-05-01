#include "verificador.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

Verificador::Verificador(Tablero* t) : tablero(t) {}

Verificador::~Verificador() {}

bool Verificador::parsearToken(const char* str, int& idx,
                                char& dir, int& pieza, int& dist, bool& esSalida) {
    while (str[idx] == ' ' || str[idx] == '\t') idx++;

    if (str[idx] == '\0') return false;

    char c = str[idx];
    if (c == 'S') {
        esSalida = true;
        idx++;
        if (str[idx] < '0' || str[idx] > '9') return false;
        pieza = 0;
        while (str[idx] >= '0' && str[idx] <= '9') {
            pieza = pieza * 10 + (str[idx] - '0');
            idx++;
        }
        dir = 'S';
        dist = 0;
        return true;
    }

    if (c != 'U' && c != 'D' && c != 'L' && c != 'R') return false;
    dir = c;
    esSalida = false;
    idx++;

    if (str[idx] < '0' || str[idx] > '9') return false;
    pieza = 0;
    while (str[idx] >= '0' && str[idx] <= '9') {
        pieza = pieza * 10 + (str[idx] - '0');
        idx++;
    }

    if (str[idx] != ',') return false;
    idx++;

    if (str[idx] < '0' || str[idx] > '9') return false;
    dist = 0;
    while (str[idx] >= '0' && str[idx] <= '9') {
        dist = dist * 10 + (str[idx] - '0');
        idx++;
    }
    if (dist <= 0) return false;

    return true;
}

bool Verificador::aplicarMovimiento(Estado* estado, char dir, int pieza, int dist) {
    int idxPieza = -1;
    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (tablero->getPiezas()[i].getId() == pieza) {
            idxPieza = i;
            break;
        }
    }
    if (idxPieza == -1) return false;
    if (estado->piezaYaSalio(idxPieza)) return false;

    direccion d;
    int dx, dy;
    switch (dir) {
        case 'U': d = ARRIBA;    dx = 0;  dy = -1; break;
        case 'D': d = ABAJO;     dx = 0;  dy = 1;  break;
        case 'L': d = IZQUIERDA; dx = -1; dy = 0;  break;
        case 'R': d = DERECHA;   dx = 1;  dy = 0;  break;
        default: return false;
    }

    Pieza& p = tablero->getPiezas()[idxPieza];
    int w = tablero->getW();

    for (int paso = 0; paso < dist; paso++) {
        if (!tablero->piezaPuedeMoverse(idxPieza, d, *estado)) return false;
        estado->moverPieza(idxPieza, dx, dy, p, w);

        for (int i = 0; i < tablero->getNumCompuertas(); i++)
            estado->actualizarCompuerta(i,
                tablero->calcularColorCompuerta(i, *estado));
        for (int i = 0; i < tablero->getNumSalidas(); i++)
            estado->actualizarSalida(i,
                tablero->calcularLargoSalida(i, *estado));
    }
    return true;
}

bool Verificador::aplicarSalida(Estado* estado, int pieza) {
    int idxPieza = -1;
    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (tablero->getPiezas()[i].getId() == pieza) {
            idxPieza = i;
            break;
        }
    }
    if (idxPieza == -1) return false;
    if (estado->piezaYaSalio(idxPieza)) return false;
    if (!tablero->piezaPuedeSalir(idxPieza, *estado)) return false;

    Pieza& p = tablero->getPiezas()[idxPieza];
    int w = tablero->getW();
    estado->sacarPieza(idxPieza, p, w);
    return true;
}

bool Verificador::verificarSolucion(const char* secuencia, const Estado* estadoInicial) {
    Estado* estado = new Estado(*estadoInicial);

    int idx = 0;
    while (secuencia[idx] != '\0') {
        char dir;
        int pieza, dist;
        bool esSalida;

        if (!parsearToken(secuencia, idx, dir, pieza, dist, esSalida)) {
            delete estado;
            return false;
        }

        if (esSalida) {
            if (!aplicarSalida(estado, pieza)) {
                delete estado;
                return false;
            }
        } else {
            if (!aplicarMovimiento(estado, dir, pieza, dist)) {
                delete estado;
                return false;
            }
        }
    }

    bool valido = estado->jugoTerminado(tablero->getNumPiezas())
               && estado->getStepUsed() <= tablero->getStepLimit();

    delete estado;
    return valido;
}
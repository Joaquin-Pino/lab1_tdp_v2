// tablahash.cpp
#include "tablaHash.h"

TablaHash::TablaHash(int capacidad)
    : capacidad(capacidad), tamano(0) {
    tabla = new Nodo*[capacidad];
    for (int i = 0; i < capacidad; i++)
        tabla[i] = nullptr;
}

TablaHash::~TablaHash() {
    for (int i = 0; i < capacidad; i++) {
        Nodo* actual = tabla[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente;
            delete actual;  // solo el nodo, no el Estado*
            actual = siguiente;
        }
    }
    delete[] tabla;
}

unsigned int TablaHash::calcularHash(const Estado* e) const {
    return e->generarHash() % (unsigned int)capacidad;
}

bool TablaHash::sonIguales(const Estado* a, const Estado* b) const {
    if (a->getNumPiezas() != b->getNumPiezas())         return false;
    if (a->getPiezasSalidas() != b->getPiezasSalidas()) return false;
    if (a->getStepUsed() != b->getStepUsed())           return false;

    for (int i = 0; i < a->getNumPiezas(); i++) {
        if (a->getPosPiezas()[i].x != b->getPosPiezas()[i].x ||
            a->getPosPiezas()[i].y != b->getPosPiezas()[i].y)
            return false;
    }

    for (int i = 0; i < a->getNumCompuertas(); i++) {
        if (a->getColorCompuertas()[i] != b->getColorCompuertas()[i])
            return false;
    }

    for (int i = 0; i < a->getNumSalidas(); i++) {
        if (a->getLargoSalidas()[i] != b->getLargoSalidas()[i])
            return false;
    }

    return true;
}

void TablaHash::insertar(Estado* e) {
    unsigned int idx = calcularHash(e);
    // no insertar si ya existe
    if (existe(e)) return;
    tabla[idx] = new Nodo(e, tabla[idx]);
    tamano++;
}

bool TablaHash::existe(const Estado* e) const {
    unsigned int idx = calcularHash(e);
    Nodo* actual = tabla[idx];
    while (actual) {
        if (sonIguales(actual->estado, e)) return true;
        actual = actual->siguiente;
    }
    return false;
}

int TablaHash::getTamano() const {
    return tamano;
}
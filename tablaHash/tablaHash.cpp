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

unsigned int TablaHash::calcularHashConCapacidad(const Estado* e, int cap) const {
    return e->generarHash() % (unsigned int)cap;
}

bool TablaHash::sonIguales(const Estado* a, const Estado* b) const {
    if (a->getNumPiezas() != b->getNumPiezas())         return false;
    if (a->getPiezasSalidas() != b->getPiezasSalidas()) return false;
    //if (a->getStepUsed() != b->getStepUsed())           return false;

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

void TablaHash::rehash() {
    int nuevaCapacidad = capacidad * 2 + 1;
    Nodo** nuevaTabla = new Nodo*[nuevaCapacidad];
    for (int i = 0; i < nuevaCapacidad; i++)
        nuevaTabla[i] = nullptr;

    for (int i = 0; i < capacidad; i++) {
        Nodo* actual = tabla[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente;
            unsigned int nuevoIdx = calcularHashConCapacidad(actual->estado, nuevaCapacidad);
            actual->siguiente = nuevaTabla[nuevoIdx];
            nuevaTabla[nuevoIdx] = actual;
            actual = siguiente;
        }
    }

    delete[] tabla;
    tabla = nuevaTabla;
    capacidad = nuevaCapacidad;
}

void TablaHash::insertar(Estado* e) {
    if ((tamano + 1) * 4 > capacidad * 3) {
        rehash();
    }
    unsigned int idx = calcularHash(e);
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

void TablaHash::liberarEstados() {
    for (int i = 0; i < capacidad; i++) {
        Nodo* actual = tabla[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente;  // guardar ANTES de borrar
            delete actual->estado;                // libera Estado*
            delete actual;                        // libera Nodo*
            actual = siguiente;
        }
        tabla[i] = nullptr;
    }
    tamano = 0;
}
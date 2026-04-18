// tablahash.h
#pragma once
#include "../estado/estado.h"

class TablaHash {
private:
    struct Nodo {
        Estado* estado;
        Nodo* siguiente;
        Nodo(Estado* e, Nodo* sig) : estado(e), siguiente(sig) {}
    };

    Nodo** tabla;
    int capacidad;
    int tamano;

    unsigned int calcularHash(const Estado* e) const;
    bool sonIguales(const Estado* a, const Estado* b) const;

public:
    TablaHash(int capacidad);
    TablaHash(const TablaHash& otro) = delete;
    TablaHash& operator=(const TablaHash& otro) = delete;
    ~TablaHash();

    void insertar(Estado* e);
    bool existe(const Estado* e) const;
    int getTamano() const;
};
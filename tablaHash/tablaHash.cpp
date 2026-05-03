// tablahash.cpp
#include "tablaHash.h"

TablaHash::TablaHash(int capacidad)
    : capacidad(capacidad), tamano(0) {
    // Inicializar todos los buckets a nullptr (lista vacía).
    tabla = new Nodo*[capacidad];
    for (int i = 0; i < capacidad; i++)
        tabla[i] = nullptr;
}

TablaHash::~TablaHash() {
    // Liberar los nodos de la lista encadenada de cada bucket.
    // Los Estado* no se liberan aquí; eso es responsabilidad de liberarEstados().
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
    // Si difieren en número de piezas, son incompatibles (no deberían existir en la misma tabla).
    if (a->getNumPiezas() != b->getNumPiezas()) return false;

    // El bitmask de piezas salidas es la primera discriminante rápida:
    // si difieren, los estados son claramente distintos sin revisar posiciones.
    if (a->getPiezasSalidas() != b->getPiezasSalidas()) return false;

    // Comparar la posición actual de cada pieza.
    // stepUsed no se compara intencionalmente: dos estados con la misma configuración
    // pero diferente costo representan el mismo nodo del grafo; A* descarta el más costoso.
    for (int i = 0; i < a->getNumPiezas(); i++) {
        if (a->getPosPiezas()[i].x != b->getPosPiezas()[i].x ||
            a->getPosPiezas()[i].y != b->getPosPiezas()[i].y)
            return false;
    }

    // Comparar el estado dinámico de las compuertas (pueden estar en colores distintos).
    for (int i = 0; i < a->getNumCompuertas(); i++) {
        if (a->getColorCompuertas()[i] != b->getColorCompuertas()[i])
            return false;
    }

    // Comparar el largo actual de cada salida (pueden haber oscilado a valores distintos).
    for (int i = 0; i < a->getNumSalidas(); i++) {
        if (a->getLargoSalidas()[i] != b->getLargoSalidas()[i])
            return false;
    }

    return true;
}

void TablaHash::rehash() {
    // Nueva capacidad = 2*vieja + 1, que tiende a ser impar (reduce colisiones con hashes pares).
    int nuevaCapacidad = capacidad * 2 + 1;
    Nodo** nuevaTabla = new Nodo*[nuevaCapacidad];
    for (int i = 0; i < nuevaCapacidad; i++)
        nuevaTabla[i] = nullptr;

    // Reubicar cada nodo existente en su nuevo bucket sin crear ni destruir nodos.
    for (int i = 0; i < capacidad; i++) {
        Nodo* actual = tabla[i];
        while (actual) {
            Nodo* siguiente = actual->siguiente; // guardar siguiente antes de redirigir

            // Calcular el nuevo bucket con la nueva capacidad y encadenar al frente.
            unsigned int nuevoIdx   = calcularHashConCapacidad(actual->estado, nuevaCapacidad);
            actual->siguiente       = nuevaTabla[nuevoIdx];
            nuevaTabla[nuevoIdx]    = actual;

            actual = siguiente;
        }
    }

    delete[] tabla;
    tabla     = nuevaTabla;
    capacidad = nuevaCapacidad;
}

void TablaHash::insertar(Estado* e) {
    // Rehash cuando el factor de carga supera 0.75 (tamano+1)/capacidad > 3/4.
    if ((tamano + 1) * 4 > capacidad * 3)
        rehash();

    // Insertar al frente de la lista del bucket correspondiente (O(1)).
    unsigned int idx = calcularHash(e);
    tabla[idx] = new Nodo(e, tabla[idx]);
    tamano++;
}

bool TablaHash::existe(const Estado* e) const {
    // Calcular el bucket y recorrer la lista encadenada buscando un estado igual.
    unsigned int idx  = calcularHash(e);
    Nodo*        actual = tabla[idx];
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
            Nodo* siguiente = actual->siguiente; // guardar ANTES de borrar el nodo
            delete actual->estado;               // liberar el Estado* (dueño)
            delete actual;                       // liberar el Nodo
            actual = siguiente;
        }
        tabla[i] = nullptr; // marcar bucket como vacío
    }
    tamano = 0;
}

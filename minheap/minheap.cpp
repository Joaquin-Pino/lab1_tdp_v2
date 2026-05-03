// minheap.cpp
#include "minheap.h"

MinHeap::MinHeap(int capacidad)
    : tamano(0), capacidad(capacidad) {
    heap = new Estado*[capacidad];
}

MinHeap::MinHeap(const MinHeap& otro)
    : tamano(otro.tamano), capacidad(otro.capacidad) {
    heap = new Estado*[capacidad];
    for (int i = 0; i < tamano; i++)
        heap[i] = otro.heap[i];  // copia punteros, no estados
}

MinHeap& MinHeap::operator=(const MinHeap& otro) {
    if (this == &otro) return *this;
    delete[] heap;
    tamano    = otro.tamano;
    capacidad = otro.capacidad;
    heap = new Estado*[capacidad];
    for (int i = 0; i < tamano; i++)
        heap[i] = otro.heap[i];
    return *this;
}

MinHeap::~MinHeap() {
    delete[] heap; // solo el arreglo de punteros; los Estado* son del Solver
}

// Sube el elemento en la posición i hasta que se restaure la propiedad de heap.
// O(log n) en el peor caso, O(1) si ya está en la posición correcta.
void MinHeap::siftUp(int i) {
    // Mientras el elemento no sea la raíz y su padre tenga f mayor, intercambiar.
    while (i > 0) {
        int padre = (i - 1) / 2;
        // Si el padre ya tiene f menor o igual, el heap está correcto.
        if (heap[padre]->getF() <= heap[i]->getF()) { 
            break;}


        // intercambiar
        Estado* tmp = heap[padre];
        heap[padre] = heap[i];
        heap[i] = tmp;
        i = padre;
    }
}

// Baja el elemento en la posición i hasta que se restaure la propiedad de heap.
// O(log n) en el peor caso, O(1) si ya está en la
void MinHeap::siftDown(int i) {
    // Mientras el elemento tenga al menos un hijo, comparar con los hijos y bajar si es mayor.
    while (true) {
        int menor = i;
        int izq   = 2 * i + 1;
        int der   = 2 * i + 2;


        // Comparar con el hijo izquierdo y derecho, si existen, para encontrar el menor.
        if (izq < tamano && heap[izq]->getF() < heap[menor]->getF())
            menor = izq;
        if (der < tamano && heap[der]->getF() < heap[menor]->getF())
            menor = der;

        // Si el menor es el elemento actual, el heap está correcto.
        if (menor == i) break;

        // intercambiar con el menor
        Estado* tmp  = heap[i];
        heap[i] = heap[menor];
        heap[menor]  = tmp;
        i = menor;
    }
}

// Inserta un nuevo estado en el heap, manteniendo la propiedad de heap.
void MinHeap::push(Estado* e) {
    if (tamano >= capacidad) {
        // Duplicar capacidad para mantener inserción amortizada O(1).
        // A* puede encolar cientos de miles de estados, así que evitamos
        // reallocations frecuentes a costa de pico de memoria 2×.
        capacidad *= 2;
        Estado** nuevo = new Estado*[capacidad];
        for (int i = 0; i < tamano; i++) {
            nuevo[i] = heap[i];
        }
            
        delete[] heap;
        heap = nuevo;
    }
    heap[tamano++] = e;
    siftUp(tamano - 1);
}

Estado* MinHeap::pop() {
    if (estaVacio()) return nullptr;
    Estado* min = heap[0];
    // Mover el último elemento a la raíz y bajar para restaurar el heap.
    // Esta es la operación estándar de pop en un heap binario: O(log n).
    heap[0] = heap[--tamano];
    if (tamano > 0) siftDown(0);
    return min;
}

bool MinHeap::estaVacio() const {
    return tamano == 0;
}

int MinHeap::getTamano() const {
    return tamano;
}
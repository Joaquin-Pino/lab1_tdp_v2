// minheap.h
#pragma once
#include "../estado/estado.h"

// Cola de prioridad mínima sobre Estado*, ordenada por f = g + h.
// Implementada como heap binario en arreglo dinámico (sin STL).
//
// Regla de ownership: el MinHeap NO es dueño de los Estado*.
// Solo almacena punteros. La memoria de los estados la gestiona el Solver.
class MinHeap {
private:
    Estado** heap;
    int tamano;
    int capacidad;

    // Sube el elemento en la posición i hasta que se restaure la propiedad de heap.
    void siftUp(int i);
    // Baja el elemento en la posición i hasta que se restaure la propiedad de heap.
    void siftDown(int i);

public:
    MinHeap(int capacidad);
    MinHeap(const MinHeap& otro);
    MinHeap& operator=(const MinHeap& otro);
    ~MinHeap();

    void push(Estado* e);
    Estado* pop();
    bool estaVacio() const;
    int getTamano() const;
};
// minheap.h
#pragma once
#include "../estado/estado.h"

class MinHeap {
private:
    Estado** heap;
    int tamano;
    int capacidad;

    void siftUp(int i);
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
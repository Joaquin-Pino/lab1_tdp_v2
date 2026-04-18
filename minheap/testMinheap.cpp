// testMinHeap.cpp
#include "minheap.h"
#include "../estado/estado.h"
#include <iostream>

int testsPasados = 0;
int testsFallados = 0;

void verificar(bool condicion, const char* nombreTest) {
    if (condicion) {
        std::cout << "[OK] " << nombreTest << std::endl;
        testsPasados++;
    } else {
        std::cout << "[FALLO] " << nombreTest << std::endl;
        testsFallados++;
    }
}

Estado* crearEstado(int f) {
    coordenada* pos = new coordenada[1];
    pos[0] = {0, 0};
    int* col   = new int[0];
    short* lar = new short[0];
    Estado* e  = new Estado(1, 0, 0, pos, col, lar, 0, 0, 0, nullptr, "");
    e->setF(f);
    delete[] pos;
    delete[] col;
    delete[] lar;
    return e;
}

int main() {
    std::cout << "===== TEST MINHEAP =====" << std::endl;

    // ── básico ──
    std::cout << "\n-- básico --" << std::endl;
    {
        MinHeap h(10);
        verificar(h.estaVacio(),    "heap vacío al inicio");
        verificar(h.getTamano()==0, "tamaño 0 al inicio");

        Estado* e1 = crearEstado(5);
        Estado* e2 = crearEstado(2);
        Estado* e3 = crearEstado(8);

        h.push(e1);
        h.push(e2);
        h.push(e3);

        verificar(!h.estaVacio(),   "no vacío después de push");
        verificar(h.getTamano()==3, "tamaño 3 después de 3 pushes");

        // pop debe retornar el de menor f
        Estado* p1 = h.pop();
        verificar(p1->getF() == 2, "primer pop retorna f=2");
        Estado* p2 = h.pop();
        verificar(p2->getF() == 5, "segundo pop retorna f=5");
        Estado* p3 = h.pop();
        verificar(p3->getF() == 8, "tercer pop retorna f=8");
        verificar(h.estaVacio(),   "vacío después de todos los pops");

        delete e1; delete e2; delete e3;
    }

    // ── orden con muchos elementos ──
    std::cout << "\n-- orden con muchos elementos --" << std::endl;
    {
        MinHeap h(5);
        int valores[] = {10, 3, 7, 1, 5, 9, 2, 8, 4, 6};
        Estado* estados[10];

        for (int i = 0; i < 10; i++) {
            estados[i] = crearEstado(valores[i]);
            h.push(estados[i]);
        }

        // debe salir en orden ascendente
        int anterior = -1;
        bool ordenCorrecto = true;
        while (!h.estaVacio()) {
            Estado* e = h.pop();
            if (e->getF() < anterior) ordenCorrecto = false;
            anterior = e->getF();
        }
        verificar(ordenCorrecto, "pop siempre retorna el menor f");

        for (int i = 0; i < 10; i++)
            delete estados[i];
    }

    // ── pop en heap vacío ──
    std::cout << "\n-- pop en heap vacío --" << std::endl;
    {
        MinHeap h(5);
        verificar(h.pop() == nullptr, "pop en vacío retorna nullptr");
    }

    // ── crecimiento dinámico ──
    std::cout << "\n-- crecimiento dinámico --" << std::endl;
    {
        MinHeap h(2);  // capacidad inicial pequeña
        Estado* estados[20];
        for (int i = 0; i < 20; i++) {
            estados[i] = crearEstado(20 - i);
            h.push(estados[i]);
        }
        verificar(h.getTamano() == 20, "creció dinámicamente a 20 elementos");
        verificar(h.pop()->getF() == 1, "menor elemento es 1 después de crecer");

        while (!h.estaVacio()) h.pop();
        for (int i = 0; i < 20; i++)
            delete estados[i];
    }

    // ── constructor de copia ──
    std::cout << "\n-- constructor de copia --" << std::endl;
    {
        MinHeap original(5);
        Estado* e1 = crearEstado(3);
        Estado* e2 = crearEstado(1);
        original.push(e1);
        original.push(e2);

        MinHeap copia(original);
        verificar(copia.getTamano() == original.getTamano(), "copia tiene mismo tamaño");
        verificar(copia.pop()->getF() == 1, "copia mantiene orden del heap");

        delete e1; delete e2;
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
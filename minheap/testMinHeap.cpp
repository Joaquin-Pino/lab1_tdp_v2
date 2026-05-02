// testMinHeap.cpp
#include "minheap.h"
#include "../estado/estado.h"
#include <iostream>

int testsPasados = 0;
int testsFallados = 0;

void verificar(bool cond, const char* nombre) {
    if (cond) {
        std::cout << "[OK] " << nombre << std::endl;
        testsPasados++;
    } else {
        std::cout << "[FALLO] " << nombre << std::endl;
        testsFallados++;
    }
}

// crea un estado mínimo con f dado (1 pieza, tablero 1x1)
Estado* crearEstado(int f) {
    coordenada pos[1] = {{0, 0}};
    int   cc[1] = {0};
    short ls[1] = {0};
    short oc[1]; oc[0] = -1;
    Estado* e = new Estado(1, 0, 0, pos, cc, ls, 0u, 0, 0, 1, 1, nullptr, "", oc);
    e->setF(f);
    return e;
}

int main() {
    std::cout << "===== TEST MINHEAP =====" << std::endl;

    // -- básico --
    std::cout << "\n-- básico --" << std::endl;
    {
        MinHeap h(10);
        verificar(h.estaVacio(),    "vacío al inicio");
        verificar(h.getTamano()==0, "tamaño 0 al inicio");

        Estado* e1 = crearEstado(5);
        Estado* e2 = crearEstado(2);
        Estado* e3 = crearEstado(8);

        h.push(e1);
        h.push(e2);
        h.push(e3);

        verificar(!h.estaVacio(),   "no vacío después de push");
        verificar(h.getTamano()==3, "tamaño 3 después de 3 pushes");

        Estado* p1 = h.pop();
        verificar(p1->getF() == 2, "primer pop: f=2 (mínimo)");
        Estado* p2 = h.pop();
        verificar(p2->getF() == 5, "segundo pop: f=5");
        Estado* p3 = h.pop();
        verificar(p3->getF() == 8, "tercer pop: f=8");
        verificar(h.estaVacio(),   "vacío tras 3 pops");

        delete e1; delete e2; delete e3;
    }

    // -- orden con muchos elementos --
    std::cout << "\n-- orden con 10 elementos --" << std::endl;
    {
        MinHeap h(5);
        int vals[] = {7, 3, 9, 1, 5, 4, 8, 2, 6, 10};
        Estado* estados[10];
        for (int i = 0; i < 10; i++) {
            estados[i] = crearEstado(vals[i]);
            h.push(estados[i]);
        }

        int anterior = -1;
        bool ordenCorrecto = true;
        while (!h.estaVacio()) {
            Estado* e = h.pop();
            if (e->getF() < anterior) ordenCorrecto = false;
            anterior = e->getF();
        }
        verificar(ordenCorrecto, "pop siempre da el menor f");

        for (int i = 0; i < 10; i++) delete estados[i];
    }

    // -- pop en heap vacío --
    std::cout << "\n-- pop en vacío --" << std::endl;
    {
        MinHeap h(4);
        verificar(h.pop() == nullptr, "pop en vacío retorna nullptr");
    }

    // -- crecimiento dinámico --
    std::cout << "\n-- crecimiento dinámico --" << std::endl;
    {
        MinHeap h(2);
        const int N = 30;
        Estado* estados[N];
        for (int i = 0; i < N; i++) {
            estados[i] = crearEstado(N - i);
            h.push(estados[i]);
        }
        verificar(h.getTamano() == N, "creció correctamente a N elementos");

        Estado* primero = h.pop();
        verificar(primero->getF() == 1, "mínimo es 1 después de crecer");

        while (!h.estaVacio()) h.pop();
        for (int i = 0; i < N; i++) delete estados[i];
    }

    // -- constructor de copia --
    std::cout << "\n-- constructor de copia --" << std::endl;
    {
        MinHeap original(5);
        Estado* e1 = crearEstado(4);
        Estado* e2 = crearEstado(1);
        Estado* e3 = crearEstado(7);
        original.push(e1);
        original.push(e2);
        original.push(e3);

        MinHeap copia(original);
        verificar(copia.getTamano() == original.getTamano(), "copia tiene mismo tamaño");
        verificar(copia.pop()->getF() == 1, "copia mantiene orden correcto");

        delete e1; delete e2; delete e3;
    }

    // -- insertar f=0 --
    std::cout << "\n-- f=0 --" << std::endl;
    {
        MinHeap h(4);
        Estado* ea = crearEstado(0);
        Estado* eb = crearEstado(3);
        h.push(eb);
        h.push(ea);
        verificar(h.pop()->getF() == 0, "f=0 sale primero");
        delete ea; delete eb;
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

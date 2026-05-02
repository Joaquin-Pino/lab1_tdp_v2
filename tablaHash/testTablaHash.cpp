// testTablaHash.cpp
#include "tablaHash.h"
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

Estado* crearEstado(int x, int y, int step = 0) {
    coordenada pos[1] = {{x, y}};
    int   cc[1] = {0};
    short ls[1] = {0};
    short oc[1]; oc[0] = -1;
    Estado* e = new Estado(1, 0, 0, pos, cc, ls, 0u, step, 0, 1, 1, nullptr, "", oc);
    return e;
}

int main() {
    std::cout << "===== TEST TABLA HASH =====" << std::endl;

    // -- básico --
    std::cout << "\n-- básico --" << std::endl;
    {
        TablaHash th(100);
        verificar(th.getTamano() == 0, "tamaño 0 al inicio");

        Estado* e1 = crearEstado(1, 1);
        // existe devuelve false antes de insertar
        verificar(!th.existe(e1), "no existe antes de insertar");

        th.insertar(e1);
        verificar(th.getTamano() == 1, "tamaño 1 después de insertar");

        // para verificar existe, usamos un estado con misma posición
        Estado* probe = crearEstado(1, 1);
        verificar(th.existe(probe), "existe después de insertar");
        delete probe;

        th.liberarEstados();
    }

    // -- múltiples estados distintos --
    std::cout << "\n-- múltiples estados distintos --" << std::endl;
    {
        TablaHash th(100);

        Estado* e1 = crearEstado(1, 1);
        Estado* e2 = crearEstado(2, 2);
        Estado* e3 = crearEstado(3, 3);

        th.insertar(e1);
        th.insertar(e2);
        th.insertar(e3);

        verificar(th.getTamano() == 3, "tamaño 3");

        Estado* probe1 = crearEstado(1, 1);
        Estado* probe2 = crearEstado(2, 2);
        Estado* probe3 = crearEstado(3, 3);
        Estado* probe4 = crearEstado(4, 4);

        verificar(th.existe(probe1),  "e1 existe");
        verificar(th.existe(probe2),  "e2 existe");
        verificar(th.existe(probe3),  "e3 existe");
        verificar(!th.existe(probe4), "e4 no existe");

        delete probe1; delete probe2; delete probe3; delete probe4;
        th.liberarEstados();
    }

    // -- stepUsed no se considera para igualdad --
    std::cout << "\n-- stepUsed ignorado en igualdad --" << std::endl;
    {
        TablaHash th(100);

        // dos estados con la misma posición pero distinto stepUsed
        // deben considerarse iguales (por diseño del A*)
        Estado* e1 = crearEstado(5, 3, 0);
        th.insertar(e1);

        Estado* e2 = crearEstado(5, 3, 10);
        verificar(th.existe(e2), "misma pos distinto step → igual para el closed set");
        delete e2;

        th.liberarEstados();
    }

    // -- colisiones (capacidad 1 fuerza colisión) --
    std::cout << "\n-- colisiones --" << std::endl;
    {
        TablaHash th(1);

        Estado* e1 = crearEstado(1, 1);
        Estado* e2 = crearEstado(2, 2);
        Estado* e3 = crearEstado(3, 3);

        th.insertar(e1);
        th.insertar(e2);
        th.insertar(e3);

        verificar(th.getTamano() == 3, "maneja colisiones: tamaño 3");

        Estado* p1 = crearEstado(1, 1);
        Estado* p2 = crearEstado(2, 2);
        Estado* p3 = crearEstado(3, 3);

        verificar(th.existe(p1), "e1 encontrado con colisiones");
        verificar(th.existe(p2), "e2 encontrado con colisiones");
        verificar(th.existe(p3), "e3 encontrado con colisiones");

        delete p1; delete p2; delete p3;
        th.liberarEstados();
    }

    // -- rehashing dinámico --
    std::cout << "\n-- rehashing --" << std::endl;
    {
        TablaHash th(4);  // capacidad pequeña → rehash rápido

        const int N = 20;
        for (int i = 0; i < N; i++) {
            Estado* e = crearEstado(i, i * 2);
            th.insertar(e);
        }
        verificar(th.getTamano() == N, "20 estados después del rehash");

        // verificar que algunos estados siguen siendo encontrados
        Estado* pa = crearEstado(0, 0);
        Estado* pb = crearEstado(N-1, (N-1)*2);
        verificar(th.existe(pa), "primer estado encontrado post-rehash");
        verificar(th.existe(pb), "último estado encontrado post-rehash");
        delete pa; delete pb;

        th.liberarEstados();
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

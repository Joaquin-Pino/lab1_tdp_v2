// testTablaHash.cpp
#include "tablaHash.h"
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

Estado* crearEstado(int x, int y, int stepUsed = 0) {
    coordenada* pos = new coordenada[1];
    pos[0] = {x, y};
    int*   col = new int[0];
    short* lar = new short[0];
    Estado* e = new Estado(1, 0, 0, pos, col, lar, 0, stepUsed, 0, nullptr, "");
    delete[] pos;
    delete[] col;
    delete[] lar;
    return e;
}

int main() {
    std::cout << "===== TEST TABLA HASH =====" << std::endl;

    // ── básico ──
    std::cout << "\n-- básico --" << std::endl;
    {
        TablaHash th(100);
        verificar(th.getTamano() == 0, "tamaño 0 al inicio");

        Estado* e1 = crearEstado(1, 1);
        verificar(!th.existe(e1), "estado no existe antes de insertar");

        th.insertar(e1);
        verificar(th.getTamano() == 1, "tamaño 1 después de insertar");
        verificar(th.existe(e1),  "estado existe después de insertar");

        delete e1;
    }

    // ── estados distintos ──
    std::cout << "\n-- estados distintos --" << std::endl;
    {
        TablaHash th(100);

        Estado* e1 = crearEstado(1, 1);
        Estado* e2 = crearEstado(2, 2);
        Estado* e3 = crearEstado(3, 3);

        th.insertar(e1);
        th.insertar(e2);
        th.insertar(e3);

        verificar(th.getTamano() == 3, "tamaño 3 con 3 estados distintos");
        verificar(th.existe(e1), "e1 existe");
        verificar(th.existe(e2), "e2 existe");
        verificar(th.existe(e3), "e3 existe");

        Estado* e4 = crearEstado(4, 4);
        verificar(!th.existe(e4), "e4 no existe");

        delete e1; delete e2; delete e3; delete e4;
    }

    // ── no duplicar ──
    std::cout << "\n-- no duplicar --" << std::endl;
    {
        TablaHash th(100);

        Estado* e1 = crearEstado(1, 1);
        Estado* e2 = crearEstado(1, 1);  // mismo estado

        th.insertar(e1);
        th.insertar(e2);  // no debe insertarse

        verificar(th.getTamano() == 1, "no inserta duplicados");

        delete e1; delete e2;
    }

    // ── estados con mismo hash pero distintos ──
    std::cout << "\n-- colisiones --" << std::endl;
    {
        TablaHash th(1);  // capacidad 1 fuerza todas las colisiones

        Estado* e1 = crearEstado(1, 1);
        Estado* e2 = crearEstado(2, 2);
        Estado* e3 = crearEstado(3, 3);

        th.insertar(e1);
        th.insertar(e2);
        th.insertar(e3);

        verificar(th.getTamano() == 3, "maneja colisiones correctamente");
        verificar(th.existe(e1), "e1 existe con colisiones");
        verificar(th.existe(e2), "e2 existe con colisiones");
        verificar(th.existe(e3), "e3 existe con colisiones");

        delete e1; delete e2; delete e3;
    }

    // ── stepUsed diferente → estado distinto ──
    std::cout << "\n-- stepUsed diferente --" << std::endl;
    {
        TablaHash th(100);

        Estado* e1 = crearEstado(1, 1, 0);
        Estado* e2 = crearEstado(1, 1, 5);  // misma posición, distinto step

        th.insertar(e1);
        verificar(!th.existe(e2), "mismo pos distinto step → estado distinto");

        delete e1; delete e2;
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
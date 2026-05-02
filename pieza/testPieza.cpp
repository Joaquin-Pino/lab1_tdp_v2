// testPieza.cpp
#include "pieza.h"
#include <iostream>
#include <stdexcept>

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

int main() {
    std::cout << "===== TEST PIEZA =====" << std::endl;

    // -- constructor default --
    std::cout << "\n-- constructor default --" << std::endl;
    {
        Pieza vacia;
        verificar(vacia.getId() == -1,            "id es -1");
        verificar(vacia.getAncho() == -1,          "ancho es -1");
        verificar(vacia.getAlto() == -1,           "alto es -1");
        verificar(vacia.getColor() == -1,          "color es -1");
        verificar(vacia.getGeometria() == nullptr, "geometria es nullptr");
    }

    // -- constructor con parámetros y getters --
    std::cout << "\n-- constructor con parámetros --" << std::endl;
    {
        bool* geom = new bool[6]{true, true, false, true, true, false};
        coordenada pos = {2, 5};
        Pieza p(3, 3, 2, 'b', pos, geom);

        verificar(p.getId() == 3,           "id correcto");
        verificar(p.getAncho() == 3,        "ancho correcto");
        verificar(p.getAlto() == 2,         "alto correcto");
        verificar(p.getColor() == 'b',      "color correcto");
        verificar(p.getPosInicial().x == 2, "pos inicial x");
        verificar(p.getPosInicial().y == 5, "pos inicial y");
        verificar(p.getGeometria() == geom, "geometria apunta al original");
    }

    // -- getCelda en pieza rectangular --
    std::cout << "\n-- getCelda rectangular --" << std::endl;
    {
        bool* geom = new bool[4]{true, true, true, true};
        coordenada pos = {0, 0};
        Pieza p(1, 2, 2, 'a', pos, geom);

        verificar(p.getCelda(0, 0), "celda (0,0)");
        verificar(p.getCelda(1, 0), "celda (1,0)");
        verificar(p.getCelda(0, 1), "celda (0,1)");
        verificar(p.getCelda(1, 1), "celda (1,1)");
    }

    // -- getCelda en geometría con huecos --
    std::cout << "\n-- getCelda no rectangular (L) --" << std::endl;
    {
        // 1 0
        // 1 1
        bool* geom = new bool[4]{true, false, true, true};
        coordenada pos = {0, 0};
        Pieza p(2, 2, 2, 'c', pos, geom);

        verificar(p.getCelda(0, 0) == true,  "(0,0) activa");
        verificar(p.getCelda(1, 0) == false, "(1,0) vacía");
        verificar(p.getCelda(0, 1) == true,  "(0,1) activa");
        verificar(p.getCelda(1, 1) == true,  "(1,1) activa");
    }

    // -- getCelda fuera de rango lanza excepción --
    std::cout << "\n-- getCelda fuera de rango --" << std::endl;
    {
        bool* geom = new bool[2]{true, true};
        coordenada pos = {0, 0};
        Pieza p(0, 2, 1, 'a', pos, geom);

        bool ok = false;
        try { p.getCelda(10, 0); }
        catch (const std::out_of_range&) { ok = true; }
        verificar(ok, "x fuera de rango lanza out_of_range");

        ok = false;
        try { p.getCelda(0, -1); }
        catch (const std::out_of_range&) { ok = true; }
        verificar(ok, "y negativo lanza out_of_range");

        ok = false;
        try { p.getCelda(-1, 0); }
        catch (const std::out_of_range&) { ok = true; }
        verificar(ok, "x negativo lanza out_of_range");
    }

    // -- constructor de copia --
    std::cout << "\n-- constructor de copia --" << std::endl;
    {
        bool* geom = new bool[2]{true, false};
        coordenada pos = {1, 3};
        Pieza original(4, 2, 1, 'd', pos, geom);
        Pieza copia(original);

        verificar(copia.getId() == original.getId(),               "copia: id");
        verificar(copia.getAncho() == original.getAncho(),         "copia: ancho");
        verificar(copia.getColor() == original.getColor(),         "copia: color");
        verificar(copia.getGeometria() != original.getGeometria(), "copia: punteros distintos");
        verificar(copia.getCelda(0, 0) == original.getCelda(0, 0),"copia: geom[0] ok");
        verificar(copia.getCelda(1, 0) == original.getCelda(1, 0),"copia: geom[1] ok");
    }

    // -- operador de asignación --
    std::cout << "\n-- asignación --" << std::endl;
    {
        bool* g1 = new bool[1]{true};
        bool* g2 = new bool[4]{true, false, false, true};
        coordenada p1 = {0, 0};
        coordenada p2 = {3, 7};

        Pieza a(1, 1, 1, 'x', p1, g1);
        Pieza b(2, 2, 2, 'y', p2, g2);
        a = b;

        verificar(a.getId() == 2,                        "asig: id actualizado");
        verificar(a.getAncho() == 2,                     "asig: ancho actualizado");
        verificar(a.getGeometria() != b.getGeometria(),  "asig: punteros independientes");
        verificar(a.getCelda(0, 0) == b.getCelda(0, 0), "asig: geom copiada");

        a = a;  // autoasignación
        verificar(a.getId() == 2, "autoasignación no corrompe");
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

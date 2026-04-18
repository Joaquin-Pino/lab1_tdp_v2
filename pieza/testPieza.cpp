#include "pieza.h"
#include <iostream>
#include <stdexcept>

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

bool* crearGeometria(int alto, int ancho, bool valor) {
    bool* geom = new bool[alto * ancho];
    for (int i = 0; i < alto * ancho; i++)
        geom[i] = valor;
    return geom;
}

// geometría en L:
// 1 0
// 1 1
bool* crearGeometriaL() {
    bool* geom = new bool[4];
    geom[0] = true;  geom[1] = false;
    geom[2] = true;  geom[3] = true;
    return geom;
}

int main() {
    std::cout << "===== TEST PIEZA =====" << std::endl;

    // ── constructor default ──
    std::cout << "\n-- Constructor default --" << std::endl;
    {
        Pieza vacia;
        verificar(vacia.getId() == -1,           "id es -1");
        verificar(vacia.getAncho() == -1,         "ancho es -1");
        verificar(vacia.getAlto() == -1,          "alto es -1");
        verificar(vacia.getColor() == -1,         "color es -1");
        verificar(vacia.getGeometria() == nullptr,"geometria es nullptr");
    }

    // ── constructor con parámetros ──
    std::cout << "\n-- Constructor con parámetros --" << std::endl;
    {
        bool* geom = crearGeometria(2, 3, true);
        coordenada pos = {1, 2};
        Pieza p(1, 3, 2, 5, pos, geom);

        verificar(p.getId() == 1,               "id correcto");
        verificar(p.getAncho() == 3,            "ancho correcto");
        verificar(p.getAlto() == 2,             "alto correcto");
        verificar(p.getColor() == 5,            "color correcto");
        verificar(p.getPosInicial().x == 1,     "posInicial.x correcto");
        verificar(p.getPosInicial().y == 2,     "posInicial.y correcto");
        verificar(p.getGeometria() == geom,     "geometria apunta al arreglo original");

        //delete[] geom;
    }

    // ── getCelda geometría llena ──
    std::cout << "\n-- getCelda geometria llena --" << std::endl;
    {
        bool* geom = crearGeometria(2, 3, true);
        coordenada pos = {0, 0};
        Pieza p(1, 3, 2, 1, pos, geom);

        verificar(p.getCelda(0, 0) == true, "getCelda(0,0) true");
        verificar(p.getCelda(1, 0) == true, "getCelda(1,0) true");
        verificar(p.getCelda(2, 0) == true, "getCelda(2,0) true");
        verificar(p.getCelda(0, 1) == true, "getCelda(0,1) true");
        verificar(p.getCelda(2, 1) == true, "getCelda(2,1) true");

        //delete[] geom;
    }

    // ── getCelda geometría en L ──
    std::cout << "\n-- getCelda geometria en L --" << std::endl;
    {
        bool* geom = crearGeometriaL();
        coordenada pos = {0, 0};
        Pieza p(2, 2, 2, 2, pos, geom);

        verificar(p.getCelda(0, 0) == true,  "L(0,0) true");
        verificar(p.getCelda(1, 0) == false, "L(1,0) false");
        verificar(p.getCelda(0, 1) == true,  "L(0,1) true");
        verificar(p.getCelda(1, 1) == true,  "L(1,1) true");

        //delete[] geom;
    }

    // ── getCelda fuera de rango ──
    std::cout << "\n-- getCelda fuera de rango --" << std::endl;
    {
        bool* geom = crearGeometria(2, 2, true);
        coordenada pos = {0, 0};
        Pieza p(1, 2, 2, 1, pos, geom);

        bool excepcion = false;
        try { p.getCelda(10, 10); }
        catch (const std::out_of_range&) { excepcion = true; }
        verificar(excepcion, "lanza out_of_range con (10,10)");

        excepcion = false;
        try { p.getCelda(-1, 0); }
        catch (const std::out_of_range&) { excepcion = true; }
        verificar(excepcion, "lanza out_of_range con x negativo");

        excepcion = false;
        try { p.getCelda(0, -1); }
        catch (const std::out_of_range&) { excepcion = true; }
        verificar(excepcion, "lanza out_of_range con y negativo");

        //delete[] geom;
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
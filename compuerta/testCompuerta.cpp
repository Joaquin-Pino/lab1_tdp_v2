// testCompuerta.cpp
#include "compuerta.h"
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

int main() {
    std::cout << "===== TEST COMPUERTA =====" << std::endl;

    // -- constructor default --
    std::cout << "\n-- constructor default --" << std::endl;
    {
        Compuerta c;
        verificar(c.getId() == -1,    "id -1 por defecto");
        verificar(c.getTamano() == 0, "tamaño 0 por defecto");
        verificar(c.getPaso() == 0,   "paso 0 por defecto");
    }

    // -- constructor con parámetros --
    std::cout << "\n-- constructor y getters --" << std::endl;
    {
        coordenada pos = {4, 2};
        Compuerta c(0, pos, 3, true, 'a', 'c', 2);

        verificar(c.getId() == 0,       "id");
        verificar(c.getPos().x == 4,    "pos x");
        verificar(c.getPos().y == 2,    "pos y");
        verificar(c.getTamano() == 3,   "tamaño");
        verificar(c.getEsVertical(),    "es vertical");
        verificar(c.getCi() == 'a',     "color inicial");
        verificar(c.getCf() == 'c',     "color final");
        verificar(c.getPaso() == 2,     "paso");
    }

    // -- aceptaBloque --
    std::cout << "\n-- aceptaBloque --" << std::endl;
    {
        coordenada pos = {0, 0};
        Compuerta c(1, pos, 3, false, 'b', 'b', 0);

        verificar(c.aceptaBloque('b', 2, 'b'),  "bloque 2 en compuerta 3 mismo color");
        verificar(c.aceptaBloque('b', 3, 'b'),  "bloque exacto al tamaño");
        verificar(!c.aceptaBloque('b', 4, 'b'), "bloque mayor que compuerta");
        verificar(!c.aceptaBloque('a', 2, 'b'), "color bloque distinto al actual");
        verificar(!c.aceptaBloque('b', 2, 'c'), "color actual distinto al bloque");
        verificar(!c.aceptaBloque('x', 1, 'y'), "ningún color coincide");
    }

    // -- compuerta fija (Ci == Cf) --
    std::cout << "\n-- compuerta fija --" << std::endl;
    {
        coordenada pos = {1, 1};
        Compuerta c(2, pos, 2, true, 'z', 'z', 0);

        verificar(c.getCi() == c.getCf(),       "ci == cf");
        verificar(c.aceptaBloque('z', 1, 'z'),  "pieza 1 pasa por compuerta 2");
        verificar(c.aceptaBloque('z', 2, 'z'),  "pieza 2 pasa exacta");
        verificar(!c.aceptaBloque('z', 3, 'z'), "pieza 3 no pasa");
    }

    // -- compuerta dinámica --
    std::cout << "\n-- compuerta dinámica (colores distintos) --" << std::endl;
    {
        coordenada pos = {0, 0};
        Compuerta c(3, pos, 5, true, 'a', 'b', 1);

        verificar(c.aceptaBloque('a', 3, 'a'),  "color 'a' pasa con actual 'a'");
        verificar(!c.aceptaBloque('b', 3, 'a'), "color 'b' no pasa con actual 'a'");
        verificar(c.aceptaBloque('b', 3, 'b'),  "color 'b' pasa con actual 'b'");
        verificar(!c.aceptaBloque('a', 3, 'b'), "color 'a' no pasa con actual 'b'");
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

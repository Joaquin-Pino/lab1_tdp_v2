// testSalida.cpp
#include "salida.h"
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
    std::cout << "===== TEST SALIDA =====" << std::endl;

    // -- constructor default --
    std::cout << "\n-- constructor default --" << std::endl;
    {
        Salida s;
        verificar(s.getId() == -1,    "id -1 por defecto");
        verificar(s.getColor() == -1, "color -1 por defecto");
        verificar(s.getLi() == -1,    "Li -1 por defecto");
    }

    // -- constructor con parámetros --
    std::cout << "\n-- constructor y getters --" << std::endl;
    {
        coordenada pos = {7, 4};
        Salida s(0, 'a', pos, false, 2, 5, 1);

        verificar(s.getId() == 0,          "id correcto");
        verificar(s.getColor() == 'a',     "color correcto");
        verificar(s.getPos().x == 7,       "pos x");
        verificar(s.getPos().y == 4,       "pos y");
        verificar(!s.getEsHorizontal(),    "es vertical");
        verificar(s.getLi() == 2,          "Li");
        verificar(s.getLf() == 5,          "Lf");
        verificar(s.getPaso() == 1,        "paso");
    }

    // -- salida horizontal fija --
    std::cout << "\n-- salida horizontal fija --" << std::endl;
    {
        coordenada pos = {0, 3};
        Salida s(1, 'b', pos, true, 4, 4, 0);

        verificar(s.getEsHorizontal(),       "es horizontal");
        verificar(s.getLi() == s.getLf(),    "salida fija (Li == Lf)");
        verificar(s.getPaso() == 0,          "paso 0");
    }

    // -- aceptaBloque --
    std::cout << "\n-- aceptaBloque --" << std::endl;
    {
        coordenada pos = {0, 0};
        Salida s(0, 'a', pos, true, 1, 6, 1);

        // el método solo compara tamano <= largoActual, no usa el color
        verificar(s.aceptaBloque(3, 4),   "bloque 3 con largo 4");
        verificar(s.aceptaBloque(4, 4),   "bloque exacto al largo");
        verificar(!s.aceptaBloque(5, 4),  "bloque mayor al largo");
        verificar(s.aceptaBloque(1, 1),   "bloque mínimo");
        verificar(!s.aceptaBloque(2, 1),  "bloque 2 con largo 1");
        verificar(s.aceptaBloque(6, 6),   "bloque 6 con largo 6");
        verificar(!s.aceptaBloque(7, 6),  "bloque 7 con largo 6");
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

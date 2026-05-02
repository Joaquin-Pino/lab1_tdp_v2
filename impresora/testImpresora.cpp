// testImpresora.cpp
// tests de impresora: verifican que las funciones no crashean y producen alguna salida
#include "impresora.h"
#include "../parser/parser.h"
#include "../solver/solver.h"
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
    std::cout << "===== TEST IMPRESORA =====" << std::endl;

    Parser p("simple1.txt");
    Tablero* tablero = p.construirTablero();

    if (!tablero) {
        std::cout << "  simple1.txt no encontrado, saltando todos los tests" << std::endl;
        std::cout << "\n===== RESULTADO =====" << std::endl;
        std::cout << "Pasados: " << testsPasados << std::endl;
        std::cout << "Fallados: " << testsFallados << std::endl;
        return 0;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();

    // -- imprimirTablero --
    std::cout << "\n-- imprimirTablero --" << std::endl;
    {
        std::cout << "Tablero base:" << std::endl;
        Impresora::imprimirTablero(*tablero);
        verificar(true, "imprimirTablero no crashea");
    }

    // -- imprimirEstado --
    std::cout << "\n-- imprimirEstado --" << std::endl;
    {
        std::cout << "Estado inicial:" << std::endl;
        Impresora::imprimirEstado(*tablero, *estadoInicial);
        verificar(true, "imprimirEstado no crashea");
    }

    // -- imprimirSolucion y imprimirSolucionPasoAPaso --
    std::cout << "\n-- imprimirSolucion --" << std::endl;
    {
        Solver solver(tablero);
        Estado** solucion = solver.resolver(estadoInicial);

        if (solucion) {
            std::cout << "String de solución: ";
            Impresora::imprimirSolucion(*tablero, solucion);
            verificar(true, "imprimirSolucion no crashea");

            // crear estado fresco para paso a paso (el anterior lo consumió el solver)
            Parser p2("simple1.txt");
            Tablero* t2 = p2.construirTablero();
            if (t2) {
                Solver solver2(t2);
                Estado* e2 = t2->crearEstadoInicial();
                Estado** sol2 = solver2.resolver(e2);
                if (sol2) {
                    std::cout << "\nSolución paso a paso:" << std::endl;
                    Impresora::imprimirSolucionPasoAPaso(*t2, sol2);
                    verificar(true, "imprimirSolucionPasoAPaso no crashea");
                    delete[] sol2;
                }
                delete t2;
            }

            delete[] solucion;
        } else {
            std::cout << "  no se encontró solución para imprimir" << std::endl;
        }
    }

    delete tablero;

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

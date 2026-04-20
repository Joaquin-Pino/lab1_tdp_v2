// solver/testSolver.cpp
#include "solver.h"
#include "../parser/parser.h"
#include "../impresora/impresora.h"
#include <iostream>
#include <cstring>

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

// ─────────────────────────────────────────
void testSimple1() {
    std::cout << "\n-- simple1.cfg --" << std::endl;

    Parser parser("tableroMedio.cfg");
    Tablero* tablero = parser.construirTablero();
    verificar(tablero != nullptr, "tablero cargado correctamente");
    if (!tablero) return;

    Estado* estadoInicial = tablero->crearEstadoInicial();
    verificar(estadoInicial != nullptr, "estado inicial creado");

    std::cout << "Tablero:" << std::endl;
    tablero->imprimir();

    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);
    // no liberar estadoInicial — el Solver es dueño

    verificar(solucion != nullptr, "solución encontrada");

    if (solucion) {
        Impresora::imprimirSolucion(*tablero, solucion);
        delete[] solucion;  // solo el arreglo de punteros, no los estados
    } else {
        std::cout << "No se encontró solución" << std::endl;
    }

    delete tablero;
    // estadoInicial lo libera el Solver en su destructor
}
// ─────────────────────────────────────────
void testSinSolucion() {
    std::cout << "\n-- sin solución --" << std::endl;

    // tablero donde la pieza no puede alcanzar la salida
    // pieza color 1, salida color 2 → nunca puede salir
    Parser parser("sinsolucion.cfg");
    Tablero* tablero = parser.construirTablero();

    if (!tablero) {
        std::cout << "  archivo sinsolucion.cfg no encontrado, saltando test" << std::endl;
        return;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();
    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);

    verificar(solucion == nullptr, "retorna nullptr cuando no hay solución");

    // delete estadoInicial;
    delete tablero;
}

// ─────────────────────────────────────────
void testEstadoInicial() {
    std::cout << "\n-- estado inicial del solver --" << std::endl;

    Parser parser("simple1.cfg");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) return;

    Estado* e = tablero->crearEstadoInicial();

    verificar(e->getStepUsed() == 0,        "stepUsed inicial = 0");
    verificar(!e->jugoTerminado(tablero->getNumPiezas()), "juego no terminado al inicio");
    verificar(e->getPosPiezas()[0].x == 4,  "pieza en x=4");
    verificar(e->getPosPiezas()[0].y == 4,  "pieza en y=4");

    // delete e;
    delete tablero;
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST SOLVER =====" << std::endl;

    testEstadoInicial();
    testSimple1();
    testSinSolucion();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
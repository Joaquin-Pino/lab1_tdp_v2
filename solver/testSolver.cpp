// testSolver.cpp
#include "solver.h"
#include "../parser/parser.h"
#include "../impresora/impresora.h"
#include "../verificador/verificador.h"
#include <iostream>
#include <cstring>

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

// ─────────────────────────────────────────
void testSimple1() {
    std::cout << "\n-- simple1.txt --" << std::endl;

    Parser parser("simple1.txt");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) {
        std::cout << "  archivo simple1.txt no encontrado, saltando" << std::endl;
        return;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();

    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);

    verificar(solucion != nullptr, "solución encontrada");

    if (solucion) {
        // contar estados en la solución
        int n = 0;
        while (solucion[n] != nullptr) n++;
        verificar(n >= 2, "solución tiene al menos 2 estados");

        // estado final debe tener juego terminado
        Estado* final = solucion[n - 1];
        verificar(final->jugoTerminado(tablero->getNumPiezas()),
                  "estado final: todas las piezas salieron");
        verificar(final->getStepUsed() <= tablero->getStepLimit(),
                  "solución dentro del stepLimit");

        std::cout << "  Solución: ";
        Impresora::imprimirSolucion(*tablero, solucion);
        delete[] solucion;
    }

    delete tablero;
}

// ─────────────────────────────────────────
void testSinSolucion() {
    std::cout << "\n-- sinsolucion.txt --" << std::endl;

    Parser parser("sinsolucion.txt");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) {
        std::cout << "  archivo sinsolucion.txt no encontrado, saltando" << std::endl;
        return;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();

    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);

    verificar(solucion == nullptr, "retorna nullptr cuando no hay solución");

    delete tablero;
}

// ─────────────────────────────────────────
void testSolucionVerificable() {
    std::cout << "\n-- solución verificable con Verificador --" << std::endl;

    Parser parser("simple1.txt");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) {
        std::cout << "  archivo simple1.txt no encontrado, saltando" << std::endl;
        return;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();

    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);

    if (!solucion) {
        verificar(false, "no se encontró solución para verificar");
        delete tablero;
        return;
    }

    // construir string de movimientos desde la solución
    char secuencia[2000] = "";
    for (int i = 1; solucion[i] != nullptr; i++) {
        const char* mov = solucion[i]->getMovimiento();
        if (strlen(secuencia) > 0) strcat(secuencia, " ");
        strncat(secuencia, mov, sizeof(secuencia) - strlen(secuencia) - 1);
    }

    delete[] solucion;

    // crear estado inicial fresco para el verificador
    Estado* estadoVerif = tablero->crearEstadoInicial();
    Verificador verificador(tablero);

    verificar(verificador.verificarSolucion(secuencia, estadoVerif),
              "la solución generada pasa la verificación");

    delete estadoVerif;
    delete tablero;
}

// ─────────────────────────────────────────
void testEstadoInicial() {
    std::cout << "\n-- estado inicial correcto --" << std::endl;

    Parser parser("simple1.txt");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) return;

    Estado* e = tablero->crearEstadoInicial();

    verificar(e->getStepUsed() == 0,  "stepUsed inicial = 0");
    verificar(e->getPiezasSalidas() == 0, "ninguna pieza salida");
    verificar(!e->jugoTerminado(tablero->getNumPiezas()), "juego no terminado");
    verificar(e->getPosPiezas()[0].x == 1, "pieza x=1");
    verificar(e->getPosPiezas()[0].y == 1, "pieza y=1");

    // no liberar estadoInicial — lo libera el solver
    Solver solver(tablero);
    solver.resolver(e);

    delete tablero;
}

// ─────────────────────────────────────────
// Carga ejemplo1.txt (mapa con compuerta dinámica), resuelve y verifica que la
// solución encontrada es correcta (pasa el Verificador).
void testSolucionConCompuerta() {
    std::cout << "\n-- solución con compuerta (ejemplo1.txt) --" << std::endl;

    Parser parser("ejemplo1.txt");
    Tablero* tablero = parser.construirTablero();
    if (!tablero) {
        std::cout << "  ejemplo1.txt no encontrado, saltando" << std::endl;
        return;
    }

    Estado* estadoInicial = tablero->crearEstadoInicial();

    Solver solver(tablero);
    Estado** solucion = solver.resolver(estadoInicial);

    verificar(solucion != nullptr, "solver encuentra solución en mapa con compuerta");

    if (solucion) {
        int n = 0;
        while (solucion[n] != nullptr) n++;

        Estado* estadoFinal = solucion[n - 1];
        verificar(estadoFinal->jugoTerminado(tablero->getNumPiezas()),
                  "estado final: todas las piezas salieron (con compuerta)");
        verificar(estadoFinal->getStepUsed() <= tablero->getStepLimit(),
                  "solución con compuerta dentro del stepLimit");

        // construir string de movimientos y verificarlo
        char secuencia[4000] = "";
        for (int i = 1; solucion[i] != nullptr; i++) {
            const char* mov = solucion[i]->getMovimiento();
            if (strlen(secuencia) > 0) strcat(secuencia, " ");
            strncat(secuencia, mov, sizeof(secuencia) - strlen(secuencia) - 1);
        }
        delete[] solucion;

        Estado* estadoVerif = tablero->crearEstadoInicial();
        Verificador verif(tablero);
        verificar(verif.verificarSolucion(secuencia, estadoVerif),
                  "solución con compuerta pasa el Verificador");
        delete estadoVerif;
    }

    delete tablero;
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST SOLVER =====" << std::endl;

    testEstadoInicial();
    testSimple1();
    testSinSolucion();
    testSolucionVerificable();
    testSolucionConCompuerta();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

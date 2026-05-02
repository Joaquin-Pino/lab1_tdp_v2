// testVerificador.cpp
#include "../verificador/verificador.h"
#include "../parser/parser.h"
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

// simple1.txt: pieza 1 (color 'a'), 2x2 en (1,1)
// salida: color 'a', X=7, Y=4, vertical, LI=LF=4 (fija)
// para salir: mover pieza a (5,4) → R1,4 D1,3 → S1

int main() {
    std::cout << "===== TEST VERIFICADOR =====" << std::endl;

    Parser p("simple1.txt");
    Tablero* tablero = p.construirTablero();

    if (!tablero) {
        std::cout << "  simple1.txt no encontrado, saltando todos los tests" << std::endl;
        std::cout << "\n===== RESULTADO =====" << std::endl;
        std::cout << "Pasados: " << testsPasados << std::endl;
        std::cout << "Fallados: " << testsFallados << std::endl;
        return 0;
    }

    Verificador verif(tablero);

    // -- secuencia válida --
    std::cout << "\n-- secuencia válida --" << std::endl;
    {
        // R1,4: mueve pieza 1 derecha 4 → (5,1)
        // D1,3: mueve pieza 1 abajo 3  → (5,4)
        // S1:   pieza 1 sale por (7,4)
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("R1,4 D1,3 S1", e);
        verificar(ok, "R1,4 D1,3 S1 es válida");
        delete e;
    }

    // -- secuencia con movimiento extra antes de salir --
    std::cout << "\n-- variante válida --" << std::endl;
    {
        // R1,4 D1,2 D1,1 S1 (misma solución, splits en pasos)
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("R1,4 D1,2 D1,1 S1", e);
        verificar(ok, "R1,4 D1,2 D1,1 S1 es válida (misma ruta, distinto split)");
        delete e;
    }

    // -- movimiento en pared --
    std::cout << "\n-- movimiento inválido --" << std::endl;
    {
        Estado* e = tablero->crearEstadoInicial();
        // U1,1: pieza en (1,1), arriba hay pared (fila 0) → falla
        bool ok = verif.verificarSolucion("U1,1", e);
        verificar(!ok, "U1,1 falla (pared arriba)");
        delete e;
    }

    // -- secuencia incompleta (pieza no sale) --
    std::cout << "\n-- secuencia incompleta --" << std::endl;
    {
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("R1,3", e);
        verificar(!ok, "solo moverse sin salir no es válido");
        delete e;
    }

    // -- ID de pieza que no existe --
    std::cout << "\n-- pieza inexistente --" << std::endl;
    {
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("R9,1", e);
        verificar(!ok, "pieza id=9 no existe");
        delete e;
    }

    // -- string vacío --
    std::cout << "\n-- string vacío --" << std::endl;
    {
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("", e);
        verificar(!ok, "string vacío no es solución válida");
        delete e;
    }

    // -- salida prematura (pieza no está en posición) --
    std::cout << "\n-- salida desde posición incorrecta --" << std::endl;
    {
        Estado* e = tablero->crearEstadoInicial();
        bool ok = verif.verificarSolucion("S1", e);
        verificar(!ok, "S1 desde (1,1) falla (pieza no adyacente a salida)");
        delete e;
    }

    delete tablero;

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

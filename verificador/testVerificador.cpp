// testVerificador.cpp
#include "../verificador/verificador.h"
#include "../parser/parser.h"
#include "../solver/solver.h"
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

    // ── portal con compuerta (ejemplo1.txt) ──────────────────────────────
    // Verifica que el Verificador aplica correctamente el cruce por portal:
    // usa el Solver para obtener una solución válida con compuerta y comprueba
    // que la misma secuencia de movimientos la acepta el Verificador.
    std::cout << "\n-- portal con compuerta (ejemplo1.txt) --" << std::endl;
    {
        Parser p2("ejemplo1.txt");
        Tablero* t2 = p2.construirTablero();

        if (!t2) {
            std::cout << "  ejemplo1.txt no encontrado, saltando" << std::endl;
        } else {
            Verificador verif2(t2);

            // obtener solución del solver
            Estado* estadoS = t2->crearEstadoInicial();
            Solver solver(t2);
            Estado** sol = solver.resolver(estadoS);

            verificar(sol != nullptr, "solver encuentra solución para mapa con compuerta");

            if (sol) {
                // construir string de movimientos
                char seq[4000] = "";
                for (int i = 1; sol[i] != nullptr; i++) {
                    const char* m = sol[i]->getMovimiento();
                    if (strlen(seq) > 0) strcat(seq, " ");
                    strncat(seq, m, sizeof(seq) - strlen(seq) - 1);
                }
                delete[] sol;

                // el verificador debe aceptar exactamente esa secuencia
                Estado* eV = t2->crearEstadoInicial();
                verificar(verif2.verificarSolucion(seq, eV),
                          "secuencia con portal pasa el verificador");
                delete eV;
            }

            // el verificador debe rechazar intentar sacar una pieza desde la posición inicial
            {
                Estado* eX = t2->crearEstadoInicial();
                verificar(!verif2.verificarSolucion("S1", eX),
                          "S1 desde posición inicial falla en mapa con compuerta");
                delete eX;
            }

            delete t2;
        }
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

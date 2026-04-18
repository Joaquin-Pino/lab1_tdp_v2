// testParser.cpp
#include "parser.h"
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

int main() {
    std::cout << "===== TEST PARSER =====" << std::endl;

    // ── archivo no existe ──
    std::cout << "\n-- archivo inexistente --" << std::endl;
    {
        Parser p("noexiste.cfg");
        Tablero* t = p.construirTablero();
        verificar(t == nullptr, "retorna nullptr si archivo no existe");
    }

    // ── SIMPLE1 ──
    std::cout << "\n-- simple1.cfg --" << std::endl;
    {
        Parser p("simple1.cfg");
        Tablero* t = p.construirTablero();

        verificar(t != nullptr,            "tablero no es null");
        verificar(t->getW() == 8,          "WIDTH = 8");
        verificar(t->getH() == 8,          "HEIGHT = 8");
        verificar(t->getStepLimit() == 50, "STEP_LIMIT = 50");
        verificar(t->getNumPiezas() == 1,  "1 bloque");
        verificar(t->getNumSalidas() == 1, "1 salida");
        verificar(t->getNumCompuertas() == 0, "0 compuertas");

        // verificar paredes en el borde
        celda* mat = t->getMatriz();
        verificar(mat[0*8+0].tipo == PARED, "esquina (0,0) es PARED");
        verificar(mat[7*8+7].tipo == PARED, "esquina (7,7) es PARED");
        verificar(mat[1*8+1].tipo == VACIA, "interior (1,1) es VACIA");

        // verificar pieza
        Pieza* piezas = t->getPiezas();
        verificar(piezas[0].getId() == 1,      "pieza id = 1");
        verificar(piezas[0].getAncho() == 2,   "pieza ancho = 2");
        verificar(piezas[0].getAlto() == 2,    "pieza alto = 2");
        verificar(piezas[0].getColor() == 'a', "pieza color = 'a'");
        verificar(piezas[0].getPosInicial().x == 4, "pieza INIT_X = 4");
        verificar(piezas[0].getPosInicial().y == 4, "pieza INIT_Y = 4");

        // verificar geometría 2x2 llena
        verificar(piezas[0].getCelda(0,0) == true, "geometria(0,0) = true");
        verificar(piezas[0].getCelda(1,0) == true, "geometria(1,0) = true");
        verificar(piezas[0].getCelda(0,1) == true, "geometria(0,1) = true");
        verificar(piezas[0].getCelda(1,1) == true, "geometria(1,1) = true");

        // verificar salida
        Salida* salidas = t->getSalidas();
        verificar(salidas[0].getColor() == 'a',       "salida color = 'a'");
        verificar(salidas[0].getPos().x == 2,          "salida X = 2");
        verificar(salidas[0].getPos().y == 7,          "salida Y = 7");
        verificar(salidas[0].getEsHorizontal() == true, "salida ORIENTATION = H");
        verificar(salidas[0].getLi() == 2,             "salida LI = 2");
        verificar(salidas[0].getLf() == 2,             "salida LF = 2");

        // verificar celda de salida en la matriz
        verificar(mat[7*8+2].tipo == SALIDA, "celda (7,2) es SALIDA");

        // verificar estado inicial
        Estado* e = t->crearEstadoInicial();
        verificar(e != nullptr,                    "estado inicial no es null");
        verificar(e->getPosPiezas()[0].x == 4,     "estado pos pieza x = 4");
        verificar(e->getPosPiezas()[0].y == 4,     "estado pos pieza y = 4");
        verificar(e->getLargoSalidas()[0] == 2,    "estado largo salida = 2");
        verificar(!e->piezaYaSalio(0),             "pieza no ha salido");
        verificar(!e->jugoTerminado(1),            "juego no terminado");

        std::cout << "\nTablero cargado:" << std::endl;
        t->imprimir();

        delete e;
        delete t;
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
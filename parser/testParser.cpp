// testParser.cpp
#include "parser.h"
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
    std::cout << "===== TEST PARSER =====" << std::endl;

    // -- archivo no existe --
    std::cout << "\n-- archivo inexistente --" << std::endl;
    {
        Parser p("noexiste_123.txt");
        Tablero* t = p.construirTablero();
        verificar(t == nullptr, "retorna nullptr si archivo no existe");
    }

    // -- simple1.txt --
    std::cout << "\n-- simple1.txt --" << std::endl;
    {
        Parser p("simple1.txt");
        Tablero* t = p.construirTablero();

        if (!t) {
            std::cout << "  archivo simple1.txt no encontrado, saltando" << std::endl;
        } else {
            verificar(t != nullptr,               "tablero no es null");
            verificar(t->getW() == 8,             "WIDTH = 8");
            verificar(t->getH() == 8,             "HEIGHT = 8");
            verificar(t->getStepLimit() == 50,    "STEP_LIMIT = 50");
            verificar(t->getNumPiezas() == 1,     "1 pieza");
            verificar(t->getNumSalidas() == 1,    "1 salida");
            verificar(t->getNumCompuertas() == 0, "0 compuertas");

            // borde superior todo paredes
            celda* mat = t->getMatriz();
            for (int x = 0; x < 8; x++)
                verificar(mat[0*8+x].tipo == PARED, "fila 0 son paredes");

            verificar(mat[1*8+1].tipo == VACIA, "interior (1,1) es VACIA");

            // pieza 1: color 'a', 2x2, posición inicial (1,1)
            Pieza* piezas = t->getPiezas();
            verificar(piezas[0].getId() == 1,           "pieza id = 1");
            verificar(piezas[0].getAncho() == 2,        "pieza ancho = 2");
            verificar(piezas[0].getAlto() == 2,         "pieza alto = 2");
            verificar(piezas[0].getColor() == 'a',      "pieza color = 'a'");
            verificar(piezas[0].getPosInicial().x == 1, "pieza INIT_X = 1");
            verificar(piezas[0].getPosInicial().y == 1, "pieza INIT_Y = 1");

            verificar(piezas[0].getCelda(0,0), "geometria(0,0) activa");
            verificar(piezas[0].getCelda(1,0), "geometria(1,0) activa");
            verificar(piezas[0].getCelda(0,1), "geometria(0,1) activa");
            verificar(piezas[0].getCelda(1,1), "geometria(1,1) activa");

            // salida: color 'a', X=7, Y=4, vertical, fija LI=LF=4
            Salida* salidas = t->getSalidas();
            verificar(salidas[0].getColor() == 'a',        "salida color = 'a'");
            verificar(salidas[0].getPos().x == 7,          "salida X = 7");
            verificar(salidas[0].getPos().y == 4,          "salida Y = 4");
            verificar(!salidas[0].getEsHorizontal(),       "salida es vertical");
            verificar(salidas[0].getLi() == 4,             "salida LI = 4");
            verificar(salidas[0].getLf() == 4,             "salida LF = 4");
            verificar(salidas[0].getPaso() == 0,           "salida STEP = 0");

            // celda de salida colocada en la matriz (row=4, col=7)
            verificar(mat[4*8+7].tipo == SALIDA, "celda (4,7) es SALIDA");

            // estado inicial
            Estado* e = t->crearEstadoInicial();
            verificar(e != nullptr,                "estado inicial no es null");
            verificar(e->getPosPiezas()[0].x == 1, "estado: pieza x = 1");
            verificar(e->getPosPiezas()[0].y == 1, "estado: pieza y = 1");
            verificar(e->getLargoSalidas()[0] == 4,"estado: largo salida = Li = 4");
            verificar(!e->piezaYaSalio(0),         "ninguna pieza ha salido");
            verificar(!e->jugoTerminado(1),        "juego no terminado");

            delete e;
            delete t;
        }
    }

    // -- tableroMedio.txt (tiene compuerta) --
    std::cout << "\n-- tableroMedio.txt --" << std::endl;
    {
        Parser p("tableroMedio.txt");
        Tablero* t = p.construirTablero();

        if (!t) {
            std::cout << "  archivo tableroMedio.txt no encontrado, saltando" << std::endl;
        } else {
            verificar(t->getW() == 10,            "WIDTH = 10");
            verificar(t->getH() == 10,            "HEIGHT = 10");
            verificar(t->getNumPiezas() == 2,     "2 piezas");
            verificar(t->getNumSalidas() == 2,    "2 salidas");
            verificar(t->getNumCompuertas() == 1, "1 compuerta");

            Compuerta* comp = t->getCompuertas();
            verificar(comp[0].getPos().x == 4,   "compuerta x = 4");
            verificar(comp[0].getPos().y == 4,   "compuerta y = 4");
            verificar(comp[0].getEsVertical(),   "compuerta es vertical");
            verificar(comp[0].getPaso() == 1,    "compuerta paso = 1");

            delete t;
        }
    }

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

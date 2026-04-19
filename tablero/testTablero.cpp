#include "tablero.h"
#include "../pieza/pieza.h"
#include "../salida/salida.h"
#include "../compuerta/compuerta.h"
#include "../estado/estado.h"
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

// ─────────────────────────────────────────
// helpers
// ─────────────────────────────────────────

celda* crearMatriz5x5() {
    int w = 5, h = 5;
    celda* mat = new celda[h * w];
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            bool esBorde = (i == 0 || i == h-1 || j == 0 || j == w-1);
            bool esSalida = (i == 1 && j == w-1);
            if (esSalida)
                mat[i * w + j] = {SALIDA, 0};
            else if (esBorde)
                mat[i * w + j] = {PARED, -1};
            else
                mat[i * w + j] = {VACIA, -1};
        }
    }
    return mat;
}

bool* geomSolida(int alto, int ancho) {
    bool* g = new bool[alto * ancho];
    for (int i = 0; i < alto * ancho; i++) g[i] = true;
    return g;
}

// ─────────────────────────────────────────
void testConstructorEImprimir() {
    std::cout << "\n-- Constructor e imprimir --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 2, 2, 1);

    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);

    verificar(t.getW() == 5,          "getW correcto");
    verificar(t.getH() == 5,          "getH correcto");
    verificar(t.getStepLimit() == 50, "getStepLimit correcto");
    verificar(t.getNumPiezas() == 1,  "numPiezas correcto");
    verificar(t.getNumSalidas() == 1, "numSalidas correcto");

    std::cout << "Tablero (debe mostrar S en (1,4)):" << std::endl;
    t.imprimir();

    // verificar que la celda de salida es correcta
    celda* m = t.getMatriz();
    verificar(m[1*5+4].tipo == SALIDA, "celda (1,4) es SALIDA");
    verificar(m[0*5+0].tipo == PARED,  "celda (0,0) es PARED");
    verificar(m[1*5+1].tipo == VACIA,  "celda (1,1) es VACIA");

    // delete[] g0;
}

// ─────────────────────────────────────────
void testCrearEstadoInicial() {
    std::cout << "\n-- crearEstadoInicial --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {2, 3}, g0);  // posicion inicial (2,3)

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 3, 2, 1);

    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    verificar(e != nullptr,                "estado no es null");
    verificar(e->getPosPiezas()[0].x == 2, "posInicial.x = 2");
    verificar(e->getPosPiezas()[0].y == 3, "posInicial.y = 3");
    verificar(e->getLargoSalidas()[0] == 3,"largoSalida inicial = Li = 3");
    verificar(e->getStepUsed() == 0,       "stepUsed = 0");
    verificar(e->getPiezasSalidas() == 0,  "ninguna pieza salida");
    verificar(!e->piezaYaSalio(0),         "pieza 0 no ha salido");
    verificar(!e->jugoTerminado(1),        "juego no terminado");

    delete e;
    // delete[] g0;
}

// ─────────────────────────────────────────
void testPiezaPuedeMoverse() {
    std::cout << "\n-- piezaPuedeMoverse --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    // pieza en (1,1): pared arriba en (0,1) y pared izquierda en (1,0)
    verificar( t.piezaPuedeMoverse(0, DERECHA,    *e), "puede moverse derecha");
    verificar( t.piezaPuedeMoverse(0, ABAJO,      *e), "puede moverse abajo");
    verificar(!t.piezaPuedeMoverse(0, ARRIBA,     *e), "bloqueada por pared arriba");
    verificar(!t.piezaPuedeMoverse(0, IZQUIERDA,  *e), "bloqueada por pared izquierda");

    delete e;
    // delete[] g0;
}

// ─────────────────────────────────────────
void testColisionEntrePiezas() {
    std::cout << "\n-- colision entre piezas --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    bool* g1      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[2];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);
    piezas[1]     = Pieza(1, 1, 1, 2, {2, 1}, g1);  // justo a la derecha

    Salida* salidas = new Salida[0];
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 2, 0, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    std::cout << "pos pieza 0: " << e->getPosPiezas()[0].x 
              << "," << e->getPosPiezas()[0].y << std::endl;
    std::cout << "pos pieza 1: " << e->getPosPiezas()[1].x 
              << "," << e->getPosPiezas()[1].y << std::endl;

    verificar(!t.piezaPuedeMoverse(0, DERECHA,    *e), "p0 bloqueada por p1 derecha");
    verificar(!t.piezaPuedeMoverse(1, IZQUIERDA,  *e), "p1 bloqueada por p0 izquierda");
    verificar( t.piezaPuedeMoverse(0, ABAJO,      *e), "p0 puede ir abajo");
    verificar( t.piezaPuedeMoverse(1, ABAJO,      *e), "p1 puede ir abajo");

    delete e;
    // delete[] g0;
    // delete[] g1;
}

// ─────────────────────────────────────────
void testPiezaPuedeSalir() {
    std::cout << "\n-- piezaPuedeSalir --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    // lejos de la salida
    verificar(!t.piezaPuedeSalir(0, *e), "no puede salir desde (1,1)");

    // mover manualmente a (3,1) — adyacente a la salida en columna 4
    e->getPosPiezas()[0].x = 3;
    e->getPosPiezas()[0].y = 1;
    verificar(t.piezaPuedeSalir(0, *e), "puede salir desde (3,1)");

    // color incorrecto — salida color 1, pieza color 2
    bool* g1      = geomSolida(1, 1);
    Pieza* p2     = new Pieza[1];
    p2[0]         = Pieza(0, 1, 1, 2, {3, 1}, g1);  // color 2
    celda* mat2   = crearMatriz5x5();
    Salida* sal2  = new Salida[1];
    sal2[0]       = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* c2 = new Compuerta[0];
    Tablero t2(mat2, p2, sal2, c2, 1, 1, 0, 5, 5, 50);
    Estado* e2 = t2.crearEstadoInicial();
    verificar(!t2.piezaPuedeSalir(0, *e2), "no puede salir con color incorrecto");

    delete e;
    delete e2;
    // delete[] g0;
    // delete[] g1;
}

// ─────────────────────────────────────────
void testHeuristica() {
    std::cout << "\n-- calcularHeuristica --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    // pieza (1,1), salida (4,1) → Manhattan = 3
    verificar(t.calcularHeuristica(*e) == 3, "heuristica desde (1,1) = 3");

    e->getPosPiezas()[0].x = 3;
    verificar(t.calcularHeuristica(*e) == 1, "heuristica desde (3,1) = 1");

    e->setPiezasSalidas(0b1);
    verificar(t.calcularHeuristica(*e) == 0, "heuristica 0 cuando pieza salio");

    delete e;
    // delete[] g0;
}

// ─────────────────────────────────────────
void testConstructorCopia() {
    std::cout << "\n-- constructor de copia --" << std::endl;

    celda* mat    = crearMatriz5x5();
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);
    Salida* sal   = new Salida[1];
    sal[0]        = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero original(mat, piezas, sal, comp, 1, 1, 0, 5, 5, 50);
    Tablero copia(original);

    verificar(copia.getW()          == original.getW(),         "W igual");
    verificar(copia.getH()          == original.getH(),         "H igual");
    verificar(copia.getStepLimit()  == original.getStepLimit(), "stepLimit igual");
    verificar(copia.getNumPiezas()  == original.getNumPiezas(), "numPiezas igual");
    verificar(copia.getMatriz()     != original.getMatriz(),    "matriz distinta en memoria");
    verificar(copia.getPiezas()     != original.getPiezas(),    "piezas distinta en memoria");

    // delete[] g0;
}

void testEsSalidaValida() {
    std::cout << "\n-- esSalidaValida --" << std::endl;

    celda* mat = crearMatriz5x5();
    // mat tiene SALIDA en (1,4) con id=0
    
    bool* g0      = geomSolida(1, 1);
    Pieza* piezas = new Pieza[1];
    piezas[0]     = Pieza(0, 1, 1, 1, {1, 1}, g0);

    Salida* salidas = new Salida[1];
    salidas[0]      = Salida(0, 1, {4, 1}, false, 2, 2, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, 5, 5, 50);
    Estado* e = t.crearEstadoInicial();

    // celda SALIDA con color correcto y tamaño correcto
    verificar(t.esSalidaValida(1, 4, piezas[0], *e),
              "celda (1,4) es salida valida para pieza color 1");

    // celda VACIA → no es salida
    verificar(!t.esSalidaValida(1, 1, piezas[0], *e),
              "celda (1,1) VACIA no es salida valida");

    // celda PARED → no es salida
    verificar(!t.esSalidaValida(0, 0, piezas[0], *e),
              "celda (0,0) PARED no es salida valida");

    // color incorrecto
    bool* g1      = geomSolida(1, 1);
    Pieza* p2     = new Pieza[1];
    p2[0]         = Pieza(1, 1, 1, 2, {1, 1}, g1);  // color 2, salida es color 1
    verificar(!t.esSalidaValida(1, 4, p2[0], *e),
              "color incorrecto no es salida valida");

    delete e;
    delete[] p2;
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST TABLERO =====" << std::endl;

    testConstructorEImprimir();
    testCrearEstadoInicial();
    testPiezaPuedeMoverse();
    testColisionEntrePiezas();
    testEsSalidaValida(); 
    testPiezaPuedeSalir();
    testHeuristica();
    testConstructorCopia();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
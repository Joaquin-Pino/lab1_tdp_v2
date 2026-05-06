// testEstado.cpp
#include "estado.h"
#include "../pieza/pieza.h"
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

short* crearOcupacion(int w, int h) {
    short* oc = new short[w * h];
    for (int i = 0; i < w * h; i++) oc[i] = -1;
    return oc;
}

coordenada* crearPosiciones(int n) {
    coordenada* pos = new coordenada[n];
    for (int i = 0; i < n; i++) { pos[i].x = i; pos[i].y = i * 2; }
    return pos;
}

// ─────────────────────────────────────────
void testConstructorDefault() {
    std::cout << "\n-- constructor default --" << std::endl;

    Estado e;
    verificar(e.getNumPiezas() == 0, "numPiezas 0");
    verificar(e.getStepUsed() == 0, "stepUsed 0");
    verificar(e.getF() == 0, "f 0");
    verificar(e.getH() == 0, "h 0");
    verificar(e.getParent() == nullptr, "parent nulo");
    verificar(e.getPiezasSalidas() == 0, "piezasSalidas 0");
    verificar(e.getMovimientoEncoded() == 0, "movimiento 0");
}

// ─────────────────────────────────────────
void testConstructorParametros() {
    std::cout << "\n-- constructor con parámetros --" << std::endl;

    int W = 5, H = 5;
    coordenada* pos = crearPosiciones(3);
    short* oc  = crearOcupacion(W, H);
    unsigned short mov = Estado::codificarMovimiento('R', 1);

    Estado e(3, pos, 0b101u, 5, 10, W, H, nullptr, mov, oc);

    verificar(e.getNumPiezas() == 3, "numPiezas 3");
    verificar(e.getStepUsed() == 5, "stepUsed 5");
    verificar(e.getH() == 10, "h 10");
    verificar(e.getF() == 15, "f = g + h = 15");
    verificar(e.getPiezasSalidas() == 0b101u, "piezasSalidas correcto");

    verificar(e.getPosPieza(0).x == 0 && e.getPosPieza(0).y == 0, "pos[0]");
    verificar(e.getPosPieza(1).x == 1 && e.getPosPieza(1).y == 2, "pos[1]");
    verificar(e.getPosPieza(2).x == 2 && e.getPosPieza(2).y == 4, "pos[2]");

    verificar(e.getOcupacion() != oc, "deep copy ocupacion");
    verificar(e.getOcupacion()[0] == -1, "ocupacion[0] = -1");

    char dir; int extId;
    e.decodificarMovimiento(dir, extId);
    verificar(dir == 'R' && extId == 1, "movimiento copiado");

    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testConstructorCopia() {
    std::cout << "\n-- constructor de copia --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos = crearPosiciones(2);
    short* oc  = crearOcupacion(W, H);
    unsigned short mov = Estado::codificarMovimiento('D', 2);

    Estado original(2, pos, 0b01u, 3, 7, W, H, nullptr, mov, oc);
    Estado copia(original);

    verificar(copia.getNumPiezas() == original.getNumPiezas(), "numPiezas igual");
    verificar(copia.getStepUsed() == original.getStepUsed(), "stepUsed igual");
    verificar(copia.getF() == original.getF(), "f igual");
    verificar(copia.getPiezasSalidas() == original.getPiezasSalidas(), "piezasSalidas igual");

    verificar(copia.getOcupacion() != original.getOcupacion(), "ocupacion en memoria distinta");
    verificar(copia.getPosPieza(0).x == original.getPosPieza(0).x, "contenido pos[0] igual");
    verificar(copia.getOcupacion()[0] == original.getOcupacion()[0], "contenido ocupacion igual");

    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testOperadorAsignacion() {
    std::cout << "\n-- operador = --" << std::endl;

    int W = 3, H = 3;
    coordenada* pos1 = crearPosiciones(1);
    short* oc1  = crearOcupacion(W, H);

    coordenada* pos2 = crearPosiciones(2);
    short* oc2  = crearOcupacion(W, H);

    Estado e1(1, pos1, 0u, 2, 5, W, H, nullptr, Estado::codificarMovimiento('U',1), oc1);
    Estado e2(2, pos2, 0b11u, 4, 8, W, H, nullptr, Estado::codificarMovimiento('L',3), oc2);

    e2 = e1;

    verificar(e2.getNumPiezas() == 1, "numPiezas actualizado");
    verificar(e2.getStepUsed() == 2, "stepUsed actualizado");
    verificar(e2.getPosPieza(0).x == e1.getPosPieza(0).x, "contenido copiado");

    e1 = e1;
    verificar(e1.getNumPiezas() == 1, "autoasignación no corrompe");

    delete[] pos1; delete[] oc1;
    delete[] pos2; delete[] oc2;
}

// ─────────────────────────────────────────
void testBitmask() {
    std::cout << "\n-- bitmask piezasSalidas --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos = crearPosiciones(4);
    short* oc  = crearOcupacion(W, H);

    Estado e(4, pos, 0u, 0, 0, W, H, nullptr, 0, oc);

    verificar(!e.piezaYaSalio(0), "pieza 0 no salió");
    verificar(!e.piezaYaSalio(3), "pieza 3 no salió");
    verificar(!e.jugoTerminado(4), "juego no terminado");

    e.setPiezasSalidas(0b0101u);
    verificar(e.piezaYaSalio(0), "pieza 0 salió");
    verificar(!e.piezaYaSalio(1), "pieza 1 no salió");
    verificar(e.piezaYaSalio(2), "pieza 2 salió");
    verificar(!e.piezaYaSalio(3), "pieza 3 no salió");
    verificar(!e.jugoTerminado(4), "2 piezas aún no es todo");

    e.setPiezasSalidas(0b1111u);
    verificar(e.jugoTerminado(4), "4 piezas → terminado");

    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testMoverPieza() {
    std::cout << "\n-- moverPieza --" << std::endl;

    int W = 5, H = 5;
    coordenada* pos = new coordenada[1];
    pos[0] = {2, 2};
    short* oc  = crearOcupacion(W, H);
    oc[2 * W + 2] = 0;

    Estado* e = new Estado(1, pos, 0u, 0, 0, W, H, nullptr, 0, oc);

    bool* geom = new bool[1]{true};
    Pieza pieza(0, 1, 1, 'a', {0, 0}, geom);

    e->moverPieza(0, 1, 0, pieza, W);

    verificar(e->getPosPieza(0).x == 3, "pos x actualizada");
    verificar(e->getPosPieza(0).y == 2, "pos y sin cambio");
    verificar(e->getStepUsed() == 1, "stepUsed incrementado");
    verificar(e->getOcupacion()[2*W + 2] == -1, "celda anterior liberada");
    verificar(e->getOcupacion()[2*W + 3] == 0, "nueva celda marcada");

    e->moverPieza(0, 0, -1, pieza, W);
    verificar(e->getPosPieza(0).y == 1, "mover arriba: pos y = 1");
    verificar(e->getStepUsed() == 2, "stepUsed 2 después de 2 movimientos");

    delete e;
    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testSacarPieza() {
    std::cout << "\n-- sacarPieza --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos = new coordenada[1];
    pos[0] = {1, 1};
    short* oc  = crearOcupacion(W, H);
    oc[1 * W + 1] = 0;

    Estado* e = new Estado(1, pos, 0u, 0, 0, W, H, nullptr, 0, oc);

    bool* geom = new bool[1]{true};
    Pieza pieza(0, 1, 1, 'a', {0, 0}, geom);

    e->sacarPieza(0, pieza, W);

    verificar(e->piezaYaSalio(0), "pieza marcada como salida");
    verificar(e->getOcupacion()[1*W + 1] == -1, "celda liberada");
    verificar(e->getStepUsed() == 0, "stepUsed no cambia al sacar");
    verificar(e->jugoTerminado(1), "juego terminado");

    delete e;
    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testClonarYMover() {
    std::cout << "\n-- clonarYMover --" << std::endl;

    int W = 5, H = 5;
    coordenada* pos = new coordenada[1];
    pos[0] = {1, 1};
    short* oc  = crearOcupacion(W, H);
    oc[1 * W + 1] = 0;

    Estado* base = new Estado(1, pos, 0u, 0, 0, W, H, nullptr, 0, oc);

    bool* geom = new bool[1]{true};
    Pieza pieza(0, 1, 1, 'a', {0, 0}, geom);

    Estado* clon = base->clonarYMover(0, 1, 0, pieza, W);

    verificar(clon->getPosPieza(0).x == 2, "clon movido a x=2");
    verificar(base->getPosPieza(0).x == 1, "base sin cambios");
    verificar(clon->getStepUsed() == 1, "clon stepUsed=1");
    verificar(base->getStepUsed() == 0, "base stepUsed=0");
    verificar(clon != base, "son objetos distintos");
    verificar(clon->getOcupacion() != base->getOcupacion(), "ocupacion independiente");

    delete base;
    delete clon;
    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testEliminarOcupacion() {
    std::cout << "\n-- eliminarOcupacion --" << std::endl;

    int W = 3, H = 3;
    coordenada* pos = new coordenada[1];
    pos[0] = {0, 0};
    short* oc  = crearOcupacion(W, H);

    Estado* e = new Estado(1, pos, 0u, 0, 0, W, H, nullptr, 0, oc);

    verificar(e->getOcupacion() != nullptr, "ocupacion existe antes");
    e->eliminarOcupacion();
    verificar(e->getOcupacion() == nullptr, "ocupacion nullptr después");

    delete e;
    delete[] pos; delete[] oc;
}

// ─────────────────────────────────────────
void testHash() {
    std::cout << "\n-- generarHash --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos1 = crearPosiciones(2);
    coordenada* pos2 = crearPosiciones(2);
    pos2[0].x = 99;

    short* oc  = crearOcupacion(W, H);

    Estado e1(2, pos1, 0u, 0, 0, W, H, nullptr, 0, oc);
    Estado e2(2, pos2, 0u, 0, 0, W, H, nullptr, 0, oc);
    Estado e3(e1);

    verificar(e1.generarHash() == e3.generarHash(), "mismo estado → mismo hash");
    verificar(e1.generarHash() != e2.generarHash(), "estados distintos → hash distinto");
    verificar(e1.igualA(e3), "e1 igualA e3");
    verificar(!e1.igualA(e2), "e1 !igualA e2");

    delete[] pos1; delete[] pos2; delete[] oc;
}

// ─────────────────────────────────────────
void testCodificarMovimiento() {
    std::cout << "\n-- codificar/decodificar movimiento --" << std::endl;

    char dir; int id;

    Estado e;
    e.setMovimientoEncoded(Estado::codificarMovimiento('R', 5));
    e.decodificarMovimiento(dir, id);
    verificar(dir == 'R' && id == 5, "R5 round-trip");

    e.setMovimientoEncoded(Estado::codificarMovimiento('S', 12));
    e.decodificarMovimiento(dir, id);
    verificar(dir == 'S' && id == 12, "S12 round-trip");

    e.setMovimientoEncoded(0);
    e.decodificarMovimiento(dir, id);
    verificar(dir == '\0', "sin movimiento → '\\0'");
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST ESTADO =====" << std::endl;

    testConstructorDefault();
    testConstructorParametros();
    testConstructorCopia();
    testOperadorAsignacion();
    testBitmask();
    testMoverPieza();
    testSacarPieza();
    testClonarYMover();
    testEliminarOcupacion();
    testHash();
    testCodificarMovimiento();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

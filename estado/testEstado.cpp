#include "estado.h"
#include "../common/coordenada.h"
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

coordenada* crearPosiciones(int n) {
    coordenada* pos = new coordenada[n];
    for (int i = 0; i < n; i++) {
        pos[i].x = i;
        pos[i].y = i * 2;
    }
    return pos;
}

int* crearColores(int n) {
    int* col = new int[n > 0 ? n : 1];
    for (int i = 0; i < n; i++) col[i] = i + 1;
    return col;
}

short* crearLargos(int n) {
    short* lar = new short[n > 0 ? n : 1];
    for (int i = 0; i < n; i++) lar[i] = (short)(i + 2);
    return lar;
}

int* crearOcupacion(int w, int h) {
    int* oc = new int[w * h];
    for (int i = 0; i < w * h; i++) oc[i] = -1;
    return oc;
}

// ─────────────────────────────────────────
void testConstructorDefault() {
    std::cout << "\n-- Constructor default --" << std::endl;

    Estado e;
    verificar(e.getNumPiezas() == 0,     "numPiezas es 0");
    verificar(e.getNumCompuertas() == 0, "numCompuertas es 0");
    verificar(e.getNumSalidas() == 0,    "numSalidas es 0");
    verificar(e.getStepUsed() == 0,      "stepUsed es 0");
    verificar(e.getF() == 0,             "f es 0");
    verificar(e.getH() == 0,             "h es 0");
    verificar(e.getParent() == nullptr,  "parent es null");
    verificar(e.getPiezasSalidas() == 0, "piezasSalidas es 0");
}

// ─────────────────────────────────────────
void testConstructorParametros() {
    std::cout << "\n-- Constructor con parámetros --" << std::endl;

    int W = 5, H = 5;
    coordenada* pos = crearPosiciones(3);
    int*   col = crearColores(2);
    short* lar = crearLargos(1);
    int*   oc  = crearOcupacion(W, H);

    Estado e(3, 2, 1, pos, col, lar, 0b101, 5, 10, W, H, nullptr, "R1,2", oc);

    verificar(e.getNumPiezas() == 3,         "numPiezas es 3");
    verificar(e.getNumCompuertas() == 2,     "numCompuertas es 2");
    verificar(e.getNumSalidas() == 1,        "numSalidas es 1");
    verificar(e.getStepUsed() == 5,          "stepUsed es 5");
    verificar(e.getH() == 10,                "h es 10");
    verificar(e.getF() == 15,                "f = g + h = 15");
    verificar(e.getPiezasSalidas() == 0b101, "piezasSalidas correcto");

    verificar(e.getPosPiezas() != pos,       "deep copy de posiciones");
    verificar(e.getPosPiezas()[0].x == 0,    "pos[0].x correcto");
    verificar(e.getPosPiezas()[1].x == 1,    "pos[1].x correcto");
    verificar(e.getPosPiezas()[2].y == 4,    "pos[2].y correcto");

    verificar(e.getColorCompuertas()[0] == 1, "colorCompuertas[0] correcto");
    verificar(e.getColorCompuertas()[1] == 2, "colorCompuertas[1] correcto");
    verificar(e.getLargoSalidas()[0] == 2,    "largoSalidas[0] correcto");

    // verificar ocupacion
    verificar(e.getOcupacion() != oc,         "deep copy de ocupacion");
    verificar(e.getOcupacion()[0] == -1,      "ocupacion[0] = -1");

    delete[] pos; delete[] col; delete[] lar; delete[] oc;
}

// ─────────────────────────────────────────
void testConstructorCopia() {
    std::cout << "\n-- Constructor de copia --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos = crearPosiciones(2);
    int*   col = crearColores(1);
    short* lar = crearLargos(1);
    int*   oc  = crearOcupacion(W, H);

    Estado original(2, 1, 1, pos, col, lar, 0b01, 3, 7, W, H, nullptr, "D2,1", oc);
    Estado copia(original);

    verificar(copia.getNumPiezas() == original.getNumPiezas(),        "numPiezas igual");
    verificar(copia.getStepUsed() == original.getStepUsed(),          "stepUsed igual");
    verificar(copia.getF() == original.getF(),                        "f igual");
    verificar(copia.getPiezasSalidas() == original.getPiezasSalidas(),"piezasSalidas igual");

    verificar(copia.getPosPiezas() != original.getPosPiezas(),        "posiciones en memoria distinta");
    verificar(copia.getColorCompuertas() != original.getColorCompuertas(), "compuertas en memoria distinta");
    verificar(copia.getLargoSalidas() != original.getLargoSalidas(),  "salidas en memoria distinta");
    verificar(copia.getOcupacion() != original.getOcupacion(),        "ocupacion en memoria distinta");

    verificar(copia.getPosPiezas()[0].x == original.getPosPiezas()[0].x, "pos[0].x igual");
    verificar(copia.getPosPiezas()[1].y == original.getPosPiezas()[1].y, "pos[1].y igual");
    verificar(copia.getOcupacion()[0] == original.getOcupacion()[0],     "ocupacion[0] igual");

    delete[] pos; delete[] col; delete[] lar; delete[] oc;
}

// ─────────────────────────────────────────
void testOperadorAsignacion() {
    std::cout << "\n-- Operador = --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos1 = crearPosiciones(2);
    int*   col1 = crearColores(1);
    short* lar1 = crearLargos(1);
    int*   oc1  = crearOcupacion(W, H);

    coordenada* pos2 = crearPosiciones(3);
    int*   col2 = crearColores(2);
    short* lar2 = crearLargos(2);
    int*   oc2  = crearOcupacion(W, H);

    Estado e1(2, 1, 1, pos1, col1, lar1, 0b01, 2, 5, W, H, nullptr, "U1,1", oc1);
    Estado e2(3, 2, 2, pos2, col2, lar2, 0b110, 4, 8, W, H, nullptr, "L3,2", oc2);

    e2 = e1;

    verificar(e2.getNumPiezas() == 2,   "numPiezas actualizado");
    verificar(e2.getStepUsed() == 2,    "stepUsed actualizado");
    verificar(e2.getPosPiezas() != e1.getPosPiezas(), "memoria distinta después de =");
    verificar(e2.getPosPiezas()[0].x == e1.getPosPiezas()[0].x, "contenido correcto");
    verificar(e2.getOcupacion() != e1.getOcupacion(), "ocupacion en memoria distinta");

    e1 = e1;
    verificar(e1.getNumPiezas() == 2, "autoasignacion no corrompe");

    delete[] pos1; delete[] col1; delete[] lar1; delete[] oc1;
    delete[] pos2; delete[] col2; delete[] lar2; delete[] oc2;
}

// ─────────────────────────────────────────
void testOcupacion() {
    std::cout << "\n-- Ocupacion --" << std::endl;

    int W = 5, H = 5;
    coordenada* pos = crearPosiciones(2);
    int*   col = crearColores(0);
    short* lar = crearLargos(0);
    int*   oc  = crearOcupacion(W, H);

    // simular pieza 0 en (0,0) y pieza 1 en (2,2)
    oc[0 * W + 0] = 0;
    oc[2 * W + 2] = 1;

    Estado e(2, 0, 0, pos, col, lar, 0, 0, 0, W, H, nullptr, "", oc);

    verificar(e.getOcupacion()[0 * W + 0] == 0,  "celda (0,0) ocupada por pieza 0");
    verificar(e.getOcupacion()[2 * W + 2] == 1,  "celda (2,2) ocupada por pieza 1");
    verificar(e.getOcupacion()[1 * W + 1] == -1, "celda (1,1) libre");

    delete[] pos; delete[] col; delete[] lar; delete[] oc;
}

// ─────────────────────────────────────────
void testBitmask() {
    std::cout << "\n-- Bitmask piezasSalidas --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos = crearPosiciones(4);
    int*   col = crearColores(0);
    short* lar = crearLargos(0);
    int*   oc  = crearOcupacion(W, H);

    Estado e(4, 0, 0, pos, col, lar, 0b0000, 0, 0, W, H, nullptr, "", oc);

    verificar(!e.piezaYaSalio(0), "pieza 0 no ha salido");
    verificar(!e.piezaYaSalio(1), "pieza 1 no ha salido");
    verificar(!e.jugoTerminado(4), "juego no terminado");

    e.setPiezasSalidas(0b0101);
    verificar(e.piezaYaSalio(0),   "pieza 0 salió");
    verificar(!e.piezaYaSalio(1),  "pieza 1 no salió");
    verificar(e.piezaYaSalio(2),   "pieza 2 salió");
    verificar(!e.piezaYaSalio(3),  "pieza 3 no salió");
    verificar(!e.jugoTerminado(4), "juego no terminado con 2 piezas");

    e.setPiezasSalidas(0b1111);
    verificar(e.jugoTerminado(4), "juego terminado con 4 piezas");

    delete[] pos; delete[] col; delete[] lar; delete[] oc;
}

// ─────────────────────────────────────────
void testHash() {
    std::cout << "\n-- generarHash --" << std::endl;

    int W = 4, H = 4;
    coordenada* pos1 = crearPosiciones(2);
    coordenada* pos2 = crearPosiciones(2);
    pos2[0].x = 99;

    int*   col = crearColores(1);
    short* lar = crearLargos(1);
    int*   oc  = crearOcupacion(W, H);

    Estado e1(2, 1, 1, pos1, col, lar, 0, 0, 0, W, H, nullptr, "", oc);
    Estado e2(2, 1, 1, pos2, col, lar, 0, 0, 0, W, H, nullptr, "", oc);
    Estado e3(e1);

    verificar(e1.generarHash() != e2.generarHash(), "hash distinto para estados distintos");
    verificar(e1.generarHash() == e3.generarHash(), "hash igual para estados iguales");

    delete[] pos1; delete[] pos2;
    delete[] col;  delete[] lar; delete[] oc;
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST ESTADO =====" << std::endl;

    testConstructorDefault();
    testConstructorParametros();
    testConstructorCopia();
    testOperadorAsignacion();
    testOcupacion();
    testBitmask();
    testHash();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;

    return (testsFallados == 0) ? 0 : 1;
}
// testTablero.cpp
#include "tablero.h"
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

// construye un tablero 5x5 con paredes en el borde y una salida en (4,1)
Tablero* hacerTableroSimple() {
    int W = 5, H = 5;
    celda* mat = new celda[W * H];

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            bool borde = (x == 0 || x == W-1 || y == 0 || y == H-1);
            mat[y * W + x] = {borde ? PARED : VACIA, -1};
        }
    }
    // salida en columna 4, fila 1
    mat[1 * W + 4] = {SALIDA, 0};

    bool* g = new bool[1]{true};
    Pieza* piezas  = new Pieza[1];
    piezas[0] = Pieza(0, 1, 1, 'a', {1, 1}, g);

    Salida* salidas = new Salida[1];
    salidas[0] = Salida(0, 'a', {4, 1}, false, 2, 2, 0);

    Compuerta* comp  = new Compuerta[0];

    return new Tablero(mat, piezas, salidas, comp, 1, 1, 0, W, H, 20);
}

// ─────────────────────────────────────────
void testGetters() {
    std::cout << "\n-- getters básicos --" << std::endl;

    Tablero* t = hacerTableroSimple();

    verificar(t->getW() == 5, "W = 5");
    verificar(t->getH() == 5, "H = 5");
    verificar(t->getStepLimit() == 20, "stepLimit = 20");
    verificar(t->getNumPiezas() == 1,  "1 pieza");
    verificar(t->getNumSalidas() == 1, "1 salida");
    verificar(t->getNumCompuertas() == 0, "0 compuertas");

    celda* m = t->getMatriz();
    verificar(m[0].tipo == PARED, "esquina (0,0) es PARED");
    verificar(m[1*5+1].tipo == VACIA,  "interior (1,1) es VACIA");
    verificar(m[1*5+4].tipo == SALIDA, "celda (1,4) es SALIDA");

    delete t;
}

// ─────────────────────────────────────────
void testCrearEstadoInicial() {
    std::cout << "\n-- crearEstadoInicial --" << std::endl;

    Tablero* t = hacerTableroSimple();
    Estado* e = t->crearEstadoInicial();

    verificar(e != nullptr, "estado no es null");
    verificar(e->getPosPiezas()[0].x == 1, "pos x = 1");
    verificar(e->getPosPiezas()[0].y == 1, "pos y = 1");
    verificar(e->getStepUsed() == 0, "stepUsed = 0");
    verificar(e->getPiezasSalidas() == 0, "ninguna pieza salió");
    verificar(!e->jugoTerminado(1),  "juego no terminado");

    // ocupacion correcta
    verificar(e->getOcupacion()[1*5 + 1] == 0, "ocupacion (1,1) = 0");
    verificar(e->getOcupacion()[0] == -1, "ocupacion (0,0) = -1");

    delete e;
    delete t;
}

// ─────────────────────────────────────────
void testPiezaPuedeMoverse() {
    std::cout << "\n-- piezaPuedeMoverse --" << std::endl;

    Tablero* t = hacerTableroSimple();
    Estado* e = t->crearEstadoInicial();  // pieza en (1,1)

    // paredes en fila 0 y columna 0
    verificar(!t->piezaPuedeMoverse(0, ARRIBA,*e), "bloqueada por pared arriba");
    verificar(!t->piezaPuedeMoverse(0, IZQUIERDA, *e), "bloqueada por pared izquierda");
    verificar( t->piezaPuedeMoverse(0, DERECHA, *e), "puede moverse derecha");
    verificar( t->piezaPuedeMoverse(0, ABAJO, *e), "puede moverse abajo");

    delete e;
    delete t;
}

// ─────────────────────────────────────────
void testColisionEntrePiezas() {
    std::cout << "\n-- colisión entre piezas --" << std::endl;

    int W = 5, H = 5;
    celda* mat = new celda[W * H];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            mat[y*W+x] = {(x==0||x==W-1||y==0||y==H-1) ? PARED : VACIA, -1};

    bool* g0 = new bool[1]{true};
    bool* g1 = new bool[1]{true};
    Pieza* piezas = new Pieza[2];
    piezas[0] = Pieza(0, 1, 1, 'a', {1, 2}, g0);
    piezas[1] = Pieza(1, 1, 1, 'b', {2, 2}, g1);

    Salida*    sal  = new Salida[0];
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, sal, comp, 2, 0, 0, W, H, 50);
    Estado* e = t.crearEstadoInicial();

    // piezas adyacentes en fila 2
    verificar(!t.piezaPuedeMoverse(0, DERECHA, *e), "pieza 0 bloqueada a la derecha por pieza 1");
    verificar(!t.piezaPuedeMoverse(1, IZQUIERDA, *e), "pieza 1 bloqueada a la izquierda por pieza 0");
    verificar( t.piezaPuedeMoverse(0, ABAJO, *e), "pieza 0 puede moverse abajo");
    verificar( t.piezaPuedeMoverse(1, ARRIBA, *e), "pieza 1 puede moverse arriba");

    delete e;
}

// ─────────────────────────────────────────
void testPiezaPuedeSalir() {
    std::cout << "\n-- piezaPuedeSalir --" << std::endl;

    Tablero* t = hacerTableroSimple();
    Estado* e = t->crearEstadoInicial();  // pieza en (1,1), salida en (4,1)

    // desde (1,1) la salida está lejos
    verificar(!t->piezaPuedeSalir(0, *e), "no puede salir desde (1,1)");

    // mover pieza a (3,1) → derecha adyacente a (4,1)
    e->getPosPiezas()[0].x = 3;
    e->getPosPiezas()[0].y = 1;
    verificar(t->piezaPuedeSalir(0, *e), "puede salir desde (3,1)");

    delete e;
    delete t;
}

// ─────────────────────────────────────────
void testCalcularLargoSalida() {
    std::cout << "\n-- calcularLargoSalida --" << std::endl;

    // salida que cambia: Li=1, Lf=3, paso=1
    int W = 5, H = 5;
    celda* mat = new celda[W * H];
    for (int i = 0; i < W*H; i++) mat[i] = {VACIA, -1};
    mat[1*W+4] = {SALIDA, 0};

    bool* g = new bool[1]{true};
    Pieza* piezas  = new Pieza[1];
    piezas[0] = Pieza(0, 1, 1, 'a', {1, 1}, g);
    Salida* salidas = new Salida[1];
    salidas[0] = Salida(0, 'a', {4, 1}, false, 1, 3, 1);
    Compuerta* comp = new Compuerta[0];

    Tablero t(mat, piezas, salidas, comp, 1, 1, 0, W, H, 50);
    Estado* e = t.crearEstadoInicial();

    verificar(t.calcularLargoSalida(0, *e) == 1, "largo en step=0 es Li=1");

    e->setStepUsed(2);
    verificar(t.calcularLargoSalida(0, *e) == 3, "largo en step=2 es 3");

    e->setStepUsed(4);
    int largo4 = t.calcularLargoSalida(0, *e);
    verificar(largo4 >= 1 && largo4 <= 3, "largo en step=4 está en rango [Li,Lf]");

    delete e;
}

// ─────────────────────────────────────────
void testCalcularColorCompuerta() {
    std::cout << "\n-- calcularColorCompuerta --" << std::endl;

    // tablero con compuerta que va de 'a' a 'b' cada step
    int W = 5, H = 5;
    celda* mat = new celda[W * H];
    for (int i = 0; i < W*H; i++) mat[i] = {VACIA, -1};
    mat[2*W+2] = {COMPUERTA, 0};

    bool* g = new bool[1]{true};
    Pieza*  piezas = new Pieza[1];
    piezas[0] = Pieza(0, 1, 1, 'a', {1, 1}, g);
    Salida*  salidas = new Salida[0];
    Compuerta* comp   = new Compuerta[1];
    comp[0]  = Compuerta(0, {2, 2}, 2, true, 'a', 'b', 1);

    Tablero t(mat, piezas, salidas, comp, 1, 0, 1, W, H, 50);
    Estado* e = t.crearEstadoInicial();

    // step=0: color = Ci = 'a'
    verificar(t.calcularColorCompuerta(0, *e) == 'a', "step=0 → color 'a'");

    e->setStepUsed(1);
    // step=1: pasosCiclo=1, ciclo=2, colorActual = 'a' + (1%2) = 'b'
    verificar(t.calcularColorCompuerta(0, *e) == 'b', "step=1 → color 'b'");

    e->setStepUsed(2);
    verificar(t.calcularColorCompuerta(0, *e) == 'a', "step=2 → vuelve a 'a'");

    delete e;
}

// ─────────────────────────────────────────
void testConstructorCopia() {
    std::cout << "\n-- constructor de copia --" << std::endl;

    Tablero* original = hacerTableroSimple();
    Tablero copia(*original);

    verificar(copia.getW() == original->getW(), "W igual");
    verificar(copia.getH() == original->getH(), "H igual");
    verificar(copia.getStepLimit() == original->getStepLimit(), "stepLimit igual");
    verificar(copia.getNumPiezas() == original->getNumPiezas(), "numPiezas igual");
    verificar(copia.getMatriz() != original->getMatriz(), "matriz independiente");
    verificar(copia.getPiezas() != original->getPiezas(), "piezas independientes");

    delete original;

    // la copia debe seguir siendo válida después de borrar el original
    verificar(copia.getW() == 5, "copia válida tras borrar original");
}

// ─────────────────────────────────────────
void testConParser() {
    std::cout << "\n-- tablero desde archivo --" << std::endl;

    Parser p("simple1.txt");
    Tablero* t = p.construirTablero();

    if (!t) {
        std::cout << "  archivo simple1.txt no encontrado, saltando" << std::endl;
        return;
    }

    verificar(t->getW() == 8,          "W = 8");
    verificar(t->getH() == 8,          "H = 8");
    verificar(t->getStepLimit() == 50, "stepLimit = 50");
    verificar(t->getNumPiezas() == 1,  "1 pieza");
    verificar(t->getNumSalidas() == 1, "1 salida");

    Estado* e = t->crearEstadoInicial();
    verificar(e->getPosPiezas()[0].x == 1, "pieza inicial x=1");
    verificar(e->getPosPiezas()[0].y == 1, "pieza inicial y=1");
    verificar(!t->piezaPuedeSalir(0, *e),  "pieza no puede salir desde (1,1)");

    delete e;
    delete t;
}

// ─────────────────────────────────────────
// Tablero 8×3 con una compuerta vertical fija de color 'a' en (3,1).
// Pieza 'a' 1×1 empieza en (2,1). Se usa para probar el portal.
void testPiezaPuedeCruzarCompuerta() {
    std::cout << "\n-- piezaPuedeCruzarCompuerta --" << std::endl;

    int W = 8, H = 3;
    celda* mat = new celda[W * H];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            mat[y*W+x] = {(y==0 || y==H-1 || x==0 || x==W-1) ? PARED : VACIA, -1};
    mat[1*W+3] = {COMPUERTA, 0};

    bool* g = new bool[1]{true};
    Pieza* piezas = new Pieza[1];
    piezas[0] = Pieza(1, 1, 1, 'a', {2, 1}, g); // 1×1, color='a', empieza en (2,1)

    Salida*    salidas = new Salida[0];
    Compuerta* comp    = new Compuerta[1];
    comp[0] = Compuerta(0, {3, 1}, 1, true, 'a', 'a', 0); // vertical, fija, color 'a'

    Tablero t(mat, piezas, salidas, comp, 1, 0, 1, W, H, 50);

    // --- portal DERECHA: pieza en (2,1), compuerta en (3,1) ---
    {
        Estado* e = t.crearEstadoInicial();

        verificar(!t.piezaPuedeMoverse(0, DERECHA, *e),
                  "compuerta bloquea movimiento normal DERECHA");

        int dx = 0, dy = 0;
        verificar(t.piezaPuedeCruzarCompuerta(0, DERECHA, *e, dx, dy),
                  "portal DERECHA válido: compuerta 'a' acepta pieza 'a'");
        verificar(dx == 2 && dy == 0,
                  "portal DERECHA: dx=2 (salta la celda de compuerta)");

        // bloquear celda de aterrizaje (4,1): el portal no puede completarse
        e->getOcupacion()[1*W+4] = 99; // obstáculo simulado
        verificar(!t.piezaPuedeCruzarCompuerta(0, DERECHA, *e, dx, dy),
                  "portal bloqueado: celda destino ocupada");

        delete e;
    }

    // tablero equivalente pero con compuerta de color 'b': pieza 'a' no puede cruzar
    {
        celda* mat2 = new celda[W * H];
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                mat2[y*W+x] = {(y==0||y==H-1||x==0||x==W-1) ? PARED : VACIA, -1};
        mat2[1*W+3] = {COMPUERTA, 0};

        bool* g2      = new bool[1]{true};
        Pieza* piezas2 = new Pieza[1];
        piezas2[0]    = Pieza(1, 1, 1, 'a', {2, 1}, g2);
        Salida*    sal2  = new Salida[0];
        Compuerta* comp2 = new Compuerta[1];
        comp2[0] = Compuerta(0, {3, 1}, 1, true, 'b', 'b', 0); // compuerta fija color 'b'

        Tablero t2(mat2, piezas2, sal2, comp2, 1, 0, 1, W, H, 50);
        Estado* e2 = t2.crearEstadoInicial();

        int dx = 0, dy = 0;
        verificar(!t2.piezaPuedeCruzarCompuerta(0, DERECHA, *e2, dx, dy),
                  "portal bloqueado: color de compuerta ('b') no coincide con pieza ('a')");
        delete e2;
    }

    // --- portal IZQUIERDA: pieza en (4,1), compuerta en (3,1) ---
    {
        Estado* base = t.crearEstadoInicial();
        // mover pieza de (2,1) a (4,1) sin pasar por validación
        Estado* e = base->clonarYMover(0, 2, 0, t.getPiezas()[0], W);
        delete base;

        verificar(!t.piezaPuedeMoverse(0, IZQUIERDA, *e),
                  "compuerta bloquea movimiento normal IZQUIERDA");

        int dx = 0, dy = 0;
        verificar(t.piezaPuedeCruzarCompuerta(0, IZQUIERDA, *e, dx, dy),
                  "portal IZQUIERDA válido");
        verificar(dx == -2 && dy == 0,
                  "portal IZQUIERDA: dx=-2");

        delete e;
    }
}

// ─────────────────────────────────────────
int main() {
    std::cout << "===== TEST TABLERO =====" << std::endl;

    testGetters();
    testCrearEstadoInicial();
    testPiezaPuedeMoverse();
    testColisionEntrePiezas();
    testPiezaPuedeSalir();
    testCalcularLargoSalida();
    testCalcularColorCompuerta();
    testConstructorCopia();
    testConParser();
    testPiezaPuedeCruzarCompuerta();

    std::cout << "\n===== RESULTADO =====" << std::endl;
    std::cout << "Pasados: " << testsPasados << std::endl;
    std::cout << "Fallados: " << testsFallados << std::endl;
    return (testsFallados == 0) ? 0 : 1;
}

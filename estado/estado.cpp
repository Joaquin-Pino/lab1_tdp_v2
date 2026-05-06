#include "estado.h"

// Por defecto = 1 ⇒ stepUsed%1 = 0 siempre. Tableros estáticos: stepUsed no discrimina.
// Tablero::crearEstadoInicial lo recalcula como LCM de los ciclos dinámicos.
int Estado::dynamicPeriod = 1;

static inline unsigned short empacarPos(int x, int y) {
    return (unsigned short)(((y & 0xFF) << 8) | (x & 0xFF));
}

Estado::Estado()
    : posPiezas(nullptr), ocupacion(nullptr), parent(nullptr),
      numPiezas(0), stepUsed(0), f(0), h(0), width(0), height(0),
      piezasSalidas(0), movimiento(0) {}

Estado::Estado(int numPiezas, const coordenada* posPiezasIn, unsigned int piezasSalidas,
               int stepUsed, int h, int width, int height, Estado* parent,
               unsigned short movimiento, const short* ocupacionIn)
    : parent(parent), numPiezas(numPiezas), stepUsed(stepUsed), f(stepUsed + h), h(h),
      width(width), height(height), piezasSalidas(piezasSalidas), movimiento(movimiento) {

    posPiezas = new unsigned short[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        posPiezas[i] = empacarPos(posPiezasIn[i].x, posPiezasIn[i].y);

    ocupacion = new short[width * height];
    for (int i = 0; i < width * height; i++)
        ocupacion[i] = ocupacionIn[i];
}

Estado::Estado(const Estado& otro)
    : parent(otro.parent), numPiezas(otro.numPiezas), stepUsed(otro.stepUsed),
      f(otro.f), h(otro.h), width(otro.width), height(otro.height),
      piezasSalidas(otro.piezasSalidas), movimiento(otro.movimiento) {

    posPiezas = new unsigned short[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        posPiezas[i] = otro.posPiezas[i];

    // ocupacion puede ser nullptr si el estado fuente fue insertado en closedSet
    // y se le liberó con eliminarOcupacion(); el clon hereda ese nullptr.
    if (otro.ocupacion) {
        ocupacion = new short[width * height];
        for (int i = 0; i < width * height; i++)
            ocupacion[i] = otro.ocupacion[i];
    } else {
        ocupacion = nullptr;
    }
}

Estado& Estado::operator=(const Estado& otro) {
    if (this == &otro) return *this;

    delete[] posPiezas;
    delete[] ocupacion;

    parent        = otro.parent;
    numPiezas     = otro.numPiezas;
    stepUsed      = otro.stepUsed;
    f             = otro.f;
    h             = otro.h;
    width         = otro.width;
    height        = otro.height;
    piezasSalidas = otro.piezasSalidas;
    movimiento    = otro.movimiento;

    posPiezas = new unsigned short[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        posPiezas[i] = otro.posPiezas[i];

    if (otro.ocupacion) {
        ocupacion = new short[width * height];
        for (int i = 0; i < width * height; i++)
            ocupacion[i] = otro.ocupacion[i];
    } else {
        ocupacion = nullptr;
    }

    return *this;
}

Estado::~Estado() {
    delete[] posPiezas;
    delete[] ocupacion;
}

coordenada Estado::getPosPieza(int i) const {
    unsigned short p = posPiezas[i];
    coordenada c;
    c.x = (int)(p & 0xFF);
    c.y = (int)((p >> 8) & 0xFF);
    return c;
}

void Estado::setPosPieza(int i, int x, int y) {
    posPiezas[i] = empacarPos(x, y);
}

unsigned short Estado::codificarMovimiento(char dirChar, int piezaExtId) {
    unsigned short dirCode;
    switch (dirChar) {
        case 'U': dirCode = 1; break;
        case 'D': dirCode = 2; break;
        case 'L': dirCode = 3; break;
        case 'R': dirCode = 4; break;
        case 'S': dirCode = 5; break;
        default:  dirCode = 0; break;
    }
    return (unsigned short)((piezaExtId & 0x1FFF) << 3) | dirCode;
}

void Estado::decodificarMovimiento(char& dirChar, int& piezaExtId) const {
    unsigned short dirCode = movimiento & 0x7;
    piezaExtId = (movimiento >> 3) & 0x1FFF;
    static const char tabla[6] = {'\0', 'U', 'D', 'L', 'R', 'S'};
    dirChar = (dirCode < 6) ? tabla[dirCode] : '\0';
}

bool Estado::piezaYaSalio(int idPieza) const {
    return (piezasSalidas & (1u << idPieza)) != 0;
}

bool Estado::jugoTerminado(int numPiezasTotal) const {
    return piezasSalidas == (1u << numPiezasTotal) - 1u;
}

unsigned int Estado::generarHash() const {
    unsigned int hash = 0;
    for (int i = 0; i < numPiezas; i++) {
        unsigned int p = posPiezas[i];
        // Mezclar primos grandes distintos por coord e índice para minimizar colisiones.
        hash ^= (p & 0xFFu) * 73856093u
              ^ ((p >> 8) & 0xFFu) * 19349663u
              ^ (unsigned int)(i * 83492791);
    }
    hash ^= piezasSalidas * 104729u;
    // stepUsed % dynamicPeriod discrimina la fase de las dinámicas (compuertas/salidas).
    // Si dynamicPeriod=1, este término es siempre 0 y no afecta el hash.
    hash ^= ((unsigned int)(stepUsed % dynamicPeriod)) * 2654435761u;
    return hash;
}

bool Estado::igualA(const Estado& otro) const {
    if (numPiezas != otro.numPiezas) return false;
    if (piezasSalidas != otro.piezasSalidas) return false;
    if ((stepUsed % dynamicPeriod) != (otro.stepUsed % dynamicPeriod)) return false;
    for (int i = 0; i < numPiezas; i++)
        if (posPiezas[i] != otro.posPiezas[i]) return false;
    return true;
}

void Estado::moverPieza(int id, int dx, int dy, const Pieza& pieza, int w) {
    coordenada pos = getPosPieza(id);

    // Limpiar la posición vieja antes de actualizar pos.
    for (int i = 0; i < pieza.getAlto(); i++) {
        for (int j = 0; j < pieza.getAncho(); j++) {
            if (!pieza.getCelda(j, i)) continue;
            ocupacion[(pos.y + i) * w + (pos.x + j)] = -1;
        }
    }

    pos.x += dx;
    pos.y += dy;

    for (int i = 0; i < pieza.getAlto(); i++) {
        for (int j = 0; j < pieza.getAncho(); j++) {
            if (!pieza.getCelda(j, i)) continue;
            ocupacion[(pos.y + i) * w + (pos.x + j)] = id;
        }
    }

    setPosPieza(id, pos.x, pos.y);

    stepUsed++;
    f = stepUsed + h;
}

void Estado::sacarPieza(int id, const Pieza& pieza, int w) {
    coordenada pos = getPosPieza(id);
    for (int i = 0; i < pieza.getAlto(); i++) {
        for (int j = 0; j < pieza.getAncho(); j++) {
            if (!pieza.getCelda(j, i)) continue;
            ocupacion[(pos.y + i) * w + (pos.x + j)] = -1;
        }
    }
    piezasSalidas |= (1u << id);
}

void Estado::eliminarOcupacion() {
    delete[] ocupacion;
    ocupacion = nullptr;
}

Estado* Estado::clonarYMover(int id, int dx, int dy, const Pieza& pieza, int w) const {
    Estado* clon = new Estado(*this);
    clon->moverPieza(id, dx, dy, pieza, w);
    return clon;
}

Estado* Estado::clonarYSacar(int id, const Pieza& pieza, int w) const {
    Estado* clon = new Estado(*this);
    clon->sacarPieza(id, pieza, w);
    return clon;
}

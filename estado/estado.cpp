#include <cstring>
#include "estado.h"

Estado::Estado() : numPiezas(0), numCompuertas(0), numSalidas(0), 
        posPiezas(nullptr), colorCompuertas(nullptr), largoSalidas(nullptr),
        piezasSalidas(0), stepUsed(0), f(0), h(0), parent(nullptr) {
    movimiento[0] = '\0';
}

Estado::Estado(int numPiezas, int numCompuertas, int numSalidas,
               coordenada* posPiezas, int* colorCompuertas, short* largoSalidas,
               unsigned int piezasSalidas, int stepUsed, int h, 
               Estado* parent, const char* movimiento)
    : numPiezas(numPiezas), numCompuertas(numCompuertas), numSalidas(numSalidas),
      piezasSalidas(piezasSalidas), stepUsed(stepUsed), h(h), parent(parent) {

    f = stepUsed + h;

    this->posPiezas = new coordenada[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        this->posPiezas[i] = posPiezas[i];

    this->colorCompuertas = new int[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        this->colorCompuertas[i] = colorCompuertas[i];

    this->largoSalidas = new short[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        this->largoSalidas[i] = largoSalidas[i];

    strncpy(this->movimiento, movimiento, 10);
    this->movimiento[9] = '\0';
}

Estado::Estado(const Estado& otro) : numPiezas(otro.numPiezas), numCompuertas(otro.numCompuertas), 
        numSalidas(otro.numSalidas), piezasSalidas(otro.piezasSalidas), stepUsed(otro.stepUsed), 
        f(otro.f), h(otro.h), parent(otro.parent) {
    // Copiar los arreglos, sin usar std::copy

    posPiezas = new coordenada[numPiezas];
    for (int i = 0; i < numPiezas; ++i) {
        posPiezas[i] = otro.posPiezas[i];
    }   
    colorCompuertas = new int[numCompuertas];
    for (int i = 0; i < numCompuertas; ++i) {
        colorCompuertas[i] = otro.colorCompuertas[i];
    }
    
    largoSalidas = new short[numSalidas];
    for (int i = 0; i < numSalidas; ++i) {
        largoSalidas[i] = otro.largoSalidas[i];
    }
    
    strncpy(movimiento, otro.movimiento, 10);
    this->movimiento[9] = '\0'; // asegurar terminacion

}

Estado& Estado::operator=(const Estado& otro) {
    if (this == &otro) return *this; // autoasignacion

    // Liberar memoria actual
    delete[] posPiezas;
    delete[] colorCompuertas;
    delete[] largoSalidas;

    // Copiar datos
    numPiezas = otro.numPiezas;
    numCompuertas = otro.numCompuertas;
    numSalidas = otro.numSalidas;
    piezasSalidas = otro.piezasSalidas;
    stepUsed = otro.stepUsed;
    f = otro.f;
    h = otro.h;
    parent = otro.parent;

    // Copiar los arreglos, sin usar std::copy
    posPiezas = new coordenada[numPiezas];
    for (int i = 0; i < numPiezas; ++i) {
        posPiezas[i] = otro.posPiezas[i];
    }   
    colorCompuertas = new int[numCompuertas];
    for (int i = 0; i < numCompuertas; ++i) {
        colorCompuertas[i] = otro.colorCompuertas[i];
    }
    
    largoSalidas = new short[numSalidas];
    for (int i = 0; i < numSalidas; ++i) {
        largoSalidas[i] = otro.largoSalidas[i];
    }
    
    strncpy(movimiento, otro.movimiento, 10);
    this->movimiento[9] = '\0'; // asegurar terminacion

    return *this;
}

Estado::~Estado() {
    delete[] posPiezas;
    delete[] colorCompuertas;
    delete[] largoSalidas;
}

bool Estado::piezaYaSalio(int idPieza) const {
    return (piezasSalidas & (1 << idPieza)) != 0;
}

bool Estado::jugoTerminado(int numPiezas) const {
    return piezasSalidas == (1U << numPiezas) - 1; // todas las piezas han salido
}

unsigned int Estado::generarHash() const {
    unsigned int hash = 0;

    for (int i = 0; i < numPiezas; i++) {
        hash ^= (unsigned int)(posPiezas[i].x * 73856093)
              ^ (unsigned int)(posPiezas[i].y * 19349663)
              ^ (unsigned int)(i * 83492791);  // el índice diferencia cada pieza
    }

    for (int i = 0; i < numCompuertas; i++) {
        hash ^= (unsigned int)(colorCompuertas[i] * 49979693)
              ^ (unsigned int)(i * 15485863);
    }

    for (int i = 0; i < numSalidas; i++) {
        hash ^= (unsigned int)(largoSalidas[i] * 32452843)
              ^ (unsigned int)(i * 25165843);
    }

    hash ^= piezasSalidas * 104729;

    return hash;
}

//getters
int Estado::getNumPiezas() const {
    return numPiezas;
}
int Estado::getNumCompuertas() const {
    return numCompuertas;
}
int Estado::getNumSalidas() const {
    return numSalidas;
}
coordenada* Estado::getPosPiezas() const {
    return posPiezas;
}
int* Estado::getColorCompuertas() const {
    return colorCompuertas;
}
short* Estado::getLargoSalidas() const {
    return largoSalidas;
}
unsigned int Estado::getPiezasSalidas() const {
    return piezasSalidas;
}
int Estado::getStepUsed() const {
    return stepUsed;
}
int Estado::getF() const {
    return f;
}
int Estado::getH() const {
    return h;
}
Estado* Estado::getParent() const {
    return parent;
}
const char* Estado::getMovimiento() const {
    return movimiento;
}   

void Estado::setPiezasSalidas(unsigned int nuevasPiezasSalidas) {
    piezasSalidas = nuevasPiezasSalidas;
}
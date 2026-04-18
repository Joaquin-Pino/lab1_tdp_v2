#include "salida.h"

Salida::Salida() : id(-1), color(-1), pos({0, 0}), esHorizontal(false), Li(-1), Lf(-1), paso(0) {}

Salida::Salida(int id, int color, coordenada pos, bool esHorizontal, short Li, short Lf, short paso) :
    id(id), color(color), pos(pos), esHorizontal(esHorizontal), Li(Li), Lf(Lf), paso(paso) {}

Salida::~Salida() {}

//getters
int Salida::getId() const {
    return id;
}

int Salida::getColor() const {
    return color;
}

coordenada Salida::getPos() const {
    return pos;
}

bool Salida::getEsHorizontal() const {
    return esHorizontal;
}

short Salida::getLi() const {
    return Li;
}

short Salida::getLf() const {
    return Lf;
}

short Salida::getPaso() const {
    return paso;
}

bool Salida::aceptaBloque(int tamano, int largoActual) const {
    return tamano <= largoActual;
}


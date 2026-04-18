#include "compuerta.h"

Compuerta::Compuerta() :id (-1), pos({0,0}), tamano(0), esVertical(false), Ci(0), Cf(0), paso(0) {}

Compuerta::Compuerta(int id, coordenada pos, short tamano, bool esVertical, short Ci, short Cf, short paso) : 
            id(id), pos(pos), tamano(tamano), esVertical(esVertical), Ci(Ci), Cf(Cf), paso(paso) {}

Compuerta::~Compuerta() {}

int Compuerta::getId() const {
    return id;
}
coordenada Compuerta::getPos() const {
    return pos;
}
short Compuerta::getTamano() const {
    return tamano;
}
bool Compuerta::getEsVertical() const {
    return esVertical;
}
short Compuerta::getCi() const {
    return Ci;
}
short Compuerta::getCf() const {
    return Cf;
}
short Compuerta::getPaso() const {
    return paso;
}

bool Compuerta::aceptaBloque(int colorBloque, int tamanoBloque, int colorActual) const {
    if (colorBloque != colorActual) return false;
    return tamanoBloque <= tamano;
}


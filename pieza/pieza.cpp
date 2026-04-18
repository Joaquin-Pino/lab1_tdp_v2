#include <stdexcept>    
#include "pieza.h"


Pieza::Pieza() : id(-1), ancho(-1), alto(-1), color(-1), posInicial({0, 0}), geometria(nullptr) {}

Pieza::Pieza(int id, short ancho, short alto, int color, coordenada posInicial, bool* geometria) 
    : id(id), ancho(ancho), alto(alto), color(color), posInicial(posInicial), geometria(geometria) {}

Pieza::~Pieza() {}

//getters
int Pieza::getId() const {
    return id;
}
short Pieza::getAncho() const {
    return ancho;
}
short Pieza::getAlto() const {
    return alto;
}
int Pieza::getColor() const {
    return color;
}
coordenada Pieza::getPosInicial() const {
    return posInicial;
}
bool* Pieza::getGeometria() const {
    return geometria;
}

bool Pieza::getCelda(int x, int y) const {
    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        throw std::out_of_range("Coordenadas fuera de los limites de la pieza");
    }
    return geometria[y * ancho + x];
}
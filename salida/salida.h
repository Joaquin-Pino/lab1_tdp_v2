#pragma once

#include "../common/coordenada.h"

class Salida{
private:
    int id;
    int color;
    coordenada pos;
    bool esHorizontal;
    short Li, Lf, paso;

public:
    Salida();
    Salida(int id, int color, coordenada pos, bool esHorizontal, short Li, short Lf, short paso);
    ~Salida();  

    //getters
    int getId() const;
    int getColor() const;
    coordenada getPos() const;
    bool getEsHorizontal() const;
    short getLi() const;
    short getLf() const;
    short getPaso() const;

    bool aceptaBloque(int tamano, int largoActual) const;

    coordenada getCeldaPos(int i) const;
};

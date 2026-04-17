#pragma once
#include "../common/coordenada.h"

class Compuerta{
private:
    int id;
    coordenada pos;
    short tamano;
    bool esVertical;
    short Ci, Cf, paso;

public:
    Compuerta();
    Compuerta(int id, coordenada pos, short tamano, bool esVertical, short Ci, short Cf, short paso);
    ~Compuerta();

    //getters
    int getId() const;
    coordenada getPos() const;
    short getTamano() const;
    bool getEsVertical() const;
    short getCi() const;
    short getCf() const;
    short getPaso() const;

    bool aceptaBloque(int tamano) const;

    coordenada getCeldaPos(int i) const;
};
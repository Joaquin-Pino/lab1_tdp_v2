#pragma once
#include "../common/coordenada.h"

class Pieza{
private:
    int id;
    short ancho, alto;
    int color;
    coordenada posInicial;
    bool* geometria; // solo es de lectura, no es neceario ser dueno
            // aplanamos el arreglo 2D para facilitar el acceso a la memoria y evitar problemas de punteros dobles

public:
    Pieza();
    Pieza(int id, short ancho, short alto, int color, coordenada posInicial, bool* geometria);
    ~Pieza();

    //getters
    int getId() const;
    short getAncho() const;
    short getAlto() const;
    int getColor() const;
    coordenada getPosInicial() const;
    bool* getGeometria() const;

    bool getCelda(int x, int y) const;
};
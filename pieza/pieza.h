#pragma once
#include "../common/coordenada.h"

class Pieza{
private:
    int id;
    short ancho, alto;
    int color;
    coordenada posInicial;
    bool* geometria; // solo es de lectura, necesario ser dueno
            // aplanamos el arreglo 2D para facilitar el acceso a la memoria y evitar problemas de punteros dobles

public:
    Pieza();
    Pieza(int id, short ancho, short alto, int color, coordenada posInicial, bool* geometria);
    // pieza toma ownership de geometria, se debe asegurar que el arreglo se libere correctamente
    ~Pieza();
    Pieza(const Pieza& otra);
    Pieza& operator=(const Pieza& otra);

    //getters
    int getId() const;
    short getAncho() const;
    short getAlto() const;
    int getColor() const;
    coordenada getPosInicial() const;
    bool* getGeometria() const;

    bool getCelda(int x, int y) const;
};
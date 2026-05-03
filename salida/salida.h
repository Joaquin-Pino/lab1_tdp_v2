#pragma once
#include "../common/coordenada.h"

// Representa un punto de salida del tablero.
// Las salidas son dinámicas: su largo oscila entre Li y Lf cada `paso` steps
// en forma de onda triangular (crece de Li a Lf y vuelve a Li, repetidamente).
// El largo actual en cada estado se calcula mediante Tablero::calcularLargoSalida.
class Salida{
private:
    int id;
    int color;          // color de pieza que puede usar esta salida (valor ASCII)
    coordenada pos;     // celda de inicio de la salida en el tablero
    bool esHorizontal;  // true: la salida se extiende en X; false: en Y
    short Li, Lf;       // largo mínimo y máximo de la salida (en celdas)
    short paso;         // cada cuántos steps se avanza un paso en la oscilación

public:
    Salida();
    Salida(int id, int color, coordenada pos, bool esHorizontal, short Li, short Lf, short paso);
    ~Salida();

    int getId() const;
    int getColor() const;
    coordenada getPos() const;
    bool getEsHorizontal() const;
    short getLi() const;
    short getLf() const;
    short getPaso() const;

    // Verifica si una pieza de tamaño `tamano` cabe en la salida con largo `largoActual`.
    // Solo comprueba dimensión; el color lo verifica el Tablero por separado.
    bool aceptaBloque(int tamano, int largoActual) const;
};

#pragma once
#include "../common/coordenada.h"

class Estado{
    // putneros a arreglos
    coordenada* posPiezas; 
    int* colorCompuertas;
    short* largoSalidas;

    unsigned int piezasSalidas; // bitmask de piezas que ya salieron

    int stepUsed;
    int f; // f = g + h, g = stepUsed
    int h; // heuristica, distancia de cada pieza a su salida

    Estado* parent; // para reconstruir el camino
    char movimiento[10]; // para reconstruir el camino, el movimiento que se hizo para llegar a este estado
    Estado();
    Estado(coordenada* posPiezas, int* colorCompuertas, short* largoSalidas, unsigned int piezasSalidas, int stepUsed, int h, Estado* parent, const char* movimiento);
    Estado(const Estado& otro); // constructor de copia
    Estado& operator=(const Estado& otro);
    ~Estado();

    bool piezaYaSalio(int idPieza) const;
    bool jugoTerminado(int numPiezas) const;
    unsigned int generarHash() const;
};
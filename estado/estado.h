#pragma once
#include "../common/coordenada.h"


class Estado{
private:
    int numPiezas;       // tamaño de posPiezas
    int numCompuertas;   // tamaño de colorCompuertas
    int numSalidas;      // tamaño de largoSalidas

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

    public:   
    Estado();
    Estado(int numPiezas, int numCompuertas, int numSalidas, coordenada* posPiezas, int* colorCompuertas, short* largoSalidas, unsigned int piezasSalidas, int stepUsed, int h, Estado* parent, const char* movimiento);
    Estado(const Estado& otro); // constructor de copia
    Estado& operator=(const Estado& otro);
    ~Estado();

    // getters
    int getNumPiezas() const;
    int getNumCompuertas() const;
    int getNumSalidas() const;
    coordenada* getPosPiezas() const;
    int* getColorCompuertas() const;
    short* getLargoSalidas() const;
    unsigned int getPiezasSalidas() const;
    int getStepUsed() const;
    int getF() const;
    int getH() const;
    Estado* getParent() const;
    const char* getMovimiento() const;

    void setPiezasSalidas(unsigned int nuevasPiezasSalidas);
    
    bool piezaYaSalio(int idPieza) const;
    bool jugoTerminado(int numPiezas) const;
    unsigned int generarHash() const;
};
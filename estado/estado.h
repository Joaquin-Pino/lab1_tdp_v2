#pragma once
#include "../common/coordenada.h"
#include "../pieza/pieza.h"


class Estado{
private:
    int numPiezas;       // tamaño de posPiezas
    int numCompuertas;   // tamaño de colorCompuertas
    int numSalidas;      // tamaño de largoSalidas

    // putneros a arreglos
    coordenada* posPiezas; 
    int* colorCompuertas;
    short* largoSalidas;
    short* ocupacion; // para saber si una posición del tablero está ocupada por una pieza

    unsigned int piezasSalidas; // bitmask de piezas que ya salieron

    int stepUsed;
    int f; // f = g + h, g = stepUsed
    int h; // heuristica, distancia de cada pieza a su salida
    int width; // ancho del tablero
    int height; // alto del tablero

    Estado* parent; // para reconstruir el camino
    char movimiento[10]; // para reconstruir el camino, el movimiento que se hizo para llegar a este estado

    public:   
    Estado();
    Estado(int numPiezas, int numCompuertas, int numSalidas, coordenada* posPiezas, 
            int* colorCompuertas, short* largoSalidas, unsigned int piezasSalidas, 
            int stepUsed, int h, int w, int height, Estado* parent, const char* movimiento, short* ocupacion);

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
    short* getOcupacion() const;

    void setF(int nuevoF);

    void setH(int h);
    void setStepUsed(int s);
    void setParent(Estado* p);
    void setMovimiento(const char* mov);
    void setPiezasSalidas(unsigned int ps);
    Estado* getParent() const;

    const char* getMovimiento() const;
    
    bool piezaYaSalio(int idPieza) const;
    bool jugoTerminado(int numPiezas) const;
    unsigned int generarHash() const;

    
    void moverPieza(int id, int dx, int dy, const Pieza& pieza, int w);
    void sacarPieza(int id, const Pieza& pieza, int w);
    void actualizarCompuerta(int idx, int color);
    void actualizarSalida(int idx, short largo);
};
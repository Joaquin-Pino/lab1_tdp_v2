// parser.h
#pragma once
#include "../tablero/tablero.h"
#include <cstdio>

class Parser {
private:
    FILE* archivo;
    char lineaActual[256];

    // datos intermedios
    int w, h, stepLimit;
    int numPiezas, numSalidas, numCompuertas;
    celda*     matriz;
    Pieza*     piezas;
    Salida*    salidas;
    Compuerta* compuertas;
    bool tieneLineaPendiente;

    bool leerLinea();
    bool esSeccion(const char* nombre);
    void parsearMeta();
    void parsearWall();
    void parsearBloques();
    void parsearSalidas();
    void parsearCompuertas();

public:
    Parser(const char* nombreArchivo);
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    ~Parser();

    Tablero* construirTablero();
};
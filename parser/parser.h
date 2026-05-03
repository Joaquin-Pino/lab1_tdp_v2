// parser.h
#pragma once
#include "../tablero/tablero.h"
#include <cstdio>

// Lee un archivo .cfg y construye el Tablero correspondiente.
// Se usa FILE* en lugar de ifstream para evitar dependencia de la STL.
//
// Regla de ownership: los arreglos intermedios (matriz, piezas, salidas, compuertas)
// son transferidos al Tablero en construirTablero(). Si construirTablero() no se llama,
// el destructor del Parser los libera.
// El Parser no es copiable porque gestiona un FILE* con estado único.
class Parser {
private:
    FILE* archivo;
    char lineaActual[256];

    int w, h, stepLimit;
    int numPiezas, numSalidas, numCompuertas;
    celda*     matriz;
    Pieza*     piezas;
    Salida*    salidas;
    Compuerta* compuertas;

    // Flag para que los parsers de secciones puedan "devolver" una línea al loop principal.
    // Cuando una sub-función consume una línea que empieza con '[', la guarda en lineaActual
    // y activa este flag en lugar de re-leerla, evitando saltarse encabezados de sección.
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

    // Parsea el archivo completo y devuelve un Tablero* en el heap.
    // El llamador es responsable de liberar el Tablero con delete.
    // Retorna nullptr si el archivo no pudo abrirse.
    Tablero* construirTablero();
};
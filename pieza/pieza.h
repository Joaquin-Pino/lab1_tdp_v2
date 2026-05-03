#pragma once
#include "../common/coordenada.h"

// Representa un bloque del juego con su forma, color y posición de partida.
// Es dato puro: no tiene lógica de movimiento; solo describe cómo es la pieza.
// El Tablero y el Estado la consultan para saber qué celdas ocupa y de qué color es.
//
// La geometría se almacena como arreglo bool aplanado (fila-mayor) para evitar
// punteros dobles y facilitar la copia por valor.
// Esta clase es dueña del arreglo geometria y lo libera en su destructor.
class Pieza{
private:
    int id;           // id externo del archivo .cfg (1, 2, 3...)
    short ancho, alto; // dimensiones del bounding box
    int color;         // color como entero (valor ASCII de la letra, ej: 'a' = 97)
    coordenada posInicial; // posición en el tablero al inicio del juego
    bool* geometria;  // geometria[y * ancho + x] = true si la celda (x,y) es parte de la pieza

public:
    Pieza();
    // El constructor toma ownership de geometria; no copiar el puntero externamente.
    Pieza(int id, short ancho, short alto, int color, coordenada posInicial, bool* geometria);
    ~Pieza();
    Pieza(const Pieza& otra);
    Pieza& operator=(const Pieza& otra);

    int getId() const;
    short getAncho() const;
    short getAlto() const;
    int getColor() const;
    coordenada getPosInicial() const;
    bool* getGeometria() const;

    // Devuelve si la celda local (x, y) es parte real de la pieza.
    // Permite piezas no rectangulares dentro de su bounding box.
    bool getCelda(int x, int y) const;
};

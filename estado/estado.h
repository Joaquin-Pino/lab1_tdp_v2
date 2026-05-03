#pragma once
#include "../common/coordenada.h"
#include "../pieza/pieza.h"

// Representa una configuración instantánea del tablero durante la búsqueda A*.
// Es el "nodo" del grafo implícito: encapsula todo lo que puede cambiar entre pasos
// (posiciones, compuertas, salidas, piezas salidas) sin duplicar los datos estáticos del Tablero.
//
// Regla de memoria: este objeto es dueño de posPiezas, colorCompuertas, largoSalidas y ocupacion.
// El puntero `parent` NO es propiedad — no se debe liberar desde aquí.
// La TablaHash sí toma ownership de los Estado* que almacena.
class Estado{
private:
    int numPiezas;       // tamaño de posPiezas
    int numCompuertas;   // tamaño de colorCompuertas
    int numSalidas;      // tamaño de largoSalidas

    coordenada* posPiezas;
    int* colorCompuertas;
    short* largoSalidas;
    // Mapa de ocupación: ocupacion[y*w + x] = id de la pieza en esa celda, -1 si vacía.
    // Se libera anticipadamente al insertar en closedSet (ver eliminarOcupacion),
    // porque ocupa width*height shorts por estado y domina el consumo de RAM con millones de nodos.
    short* ocupacion;

    // Bitmask: el bit i está encendido si la pieza i ya salió del tablero.
    // Permite verificar en O(1) si todas las piezas salieron sin iterar sobre el arreglo.
    unsigned int piezasSalidas;

    int stepUsed;
    int f; // f = g + h, donde g = stepUsed
    int h; // estimación heurística del costo restante hasta la meta
    int width;
    int height;

    Estado* parent; // nodo predecesor en A* (no owned)
    char movimiento[10]; // movimiento que generó este estado (ej: "R3,1", "S2")

    public:
    Estado();
    Estado(int numPiezas, int numCompuertas, int numSalidas, coordenada* posPiezas,
            int* colorCompuertas, short* largoSalidas, unsigned int piezasSalidas,
            int stepUsed, int h, int w, int height, Estado* parent, const char* movimiento, short* ocupacion);

    Estado(const Estado& otro);
    Estado& operator=(const Estado& otro);
    ~Estado();

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

    // Hash determinístico del estado para la TablaHash.
    // Codifica posiciones, colores de compuertas, largos de salidas y piezas salidas.
    // No incluye stepUsed ni f/h: dos estados con misma configuración pero diferente g
    // se consideran iguales (el cerrado set descarta el más costoso).
    unsigned int generarHash() const;

    // Mueve la pieza una celda, actualizando el mapa de ocupación y sumando 1 a stepUsed.
    void moverPieza(int id, int dx, int dy, const Pieza& pieza, int w);

    // Marca la pieza como salida y limpia su ocupación.
    // No incrementa stepUsed: sacar una pieza es gratis en el modelo del juego.
    void sacarPieza(int id, const Pieza& pieza, int w);

    void actualizarCompuerta(int idx, int color);
    void actualizarSalida(int idx, short largo);

    // Libera el mapa de ocupación anticipadamente para ahorrar memoria.
    // Es seguro llamarlo en estados ya insertados en closedSet porque la igualdad
    // entre estados se compara por posPiezas + piezasSalidas + compuertas + salidas,
    // no por ocupacion (que es derivable de las posiciones).
    void eliminarOcupacion();

    // Devuelve un nuevo Estado con el movimiento ya aplicado (incrementa stepUsed).
    Estado* clonarYMover(int id, int dx, int dy, const Pieza& pieza, int w) const;

    // Devuelve un nuevo Estado con la pieza marcada como salida (no incrementa stepUsed).
    Estado* clonarYSacar(int id, const Pieza& pieza, int w) const;
};
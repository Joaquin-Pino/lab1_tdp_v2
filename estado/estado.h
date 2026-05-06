#pragma once
#include "../common/coordenada.h"
#include "../pieza/pieza.h"

// Representa una configuración instantánea del tablero durante la búsqueda A*.
// Es el "nodo" del grafo implícito: encapsula todo lo que puede cambiar entre pasos
// (posiciones de piezas y bitmask de salidas) sin duplicar los datos estáticos del Tablero.
//
// Diseño orientado a memoria: este objeto vive millones de veces en el closedSet,
// así que se evitan los caches derivables (colores de compuertas, largos de salidas:
// son función de stepUsed y se calculan on-demand desde Tablero) y se empacan
// los campos restantes (posiciones a 2 B por pieza, movimiento a 2 B).
//
// Regla de memoria: dueño de posPiezas y ocupacion. parent NO es propiedad.
class Estado {
private:
    // posPiezas[i] empaca la posición de la pieza i: byte bajo = x, byte alto = y.
    // Boards de hasta 256x256. numPiezas entradas, 2 B cada una.
    unsigned short* posPiezas;

    // Mapa de ocupación: ocupacion[y*w + x] = id de la pieza en esa celda, -1 si vacía.
    // Se libera anticipadamente al insertar en closedSet (ver eliminarOcupacion).
    short* ocupacion;

    // Predecesor en A* (no owned). La cadena se rompe al copiar para reconstruir el camino.
    Estado* parent;

    int numPiezas;
    int stepUsed;
    int f; // f = g + h, g = stepUsed
    int h;
    int width;
    int height;

    // Bitmask: bit i encendido si la pieza i ya salió.
    unsigned int piezasSalidas;

    // Movimiento que generó este estado. Empacado:
    //   bits 0-2: dir (0=NONE, 1=U, 2=D, 3=L, 4=R, 5=S)
    //   bits 3-15: id externo de la pieza (hasta 8191)
    unsigned short movimiento;

public:
    // Período de las dinámicas globales del Tablero actual (LCM de los ciclos de
    // compuertas y salidas oscilantes). Lo setea Tablero::crearEstadoInicial.
    // Se usa en hash/igualA para que dos estados con mismas posiciones pero distinto
    // stepUsed se distingan solo cuando difieren en su fase del ciclo dinámico.
    // Default 1 ⇒ tableros estáticos: stepUsed deja de discriminar.
    static int dynamicPeriod;

    Estado();
    Estado(int numPiezas, const coordenada* posPiezas, unsigned int piezasSalidas,
           int stepUsed, int h, int width, int height, Estado* parent,
           unsigned short movimiento, const short* ocupacion);
    Estado(const Estado& otro);
    Estado& operator=(const Estado& otro);
    ~Estado();

    int getNumPiezas() const { return numPiezas; }
    int getStepUsed()  const { return stepUsed; }
    int getF()         const { return f; }
    int getH()         const { return h; }
    unsigned int getPiezasSalidas() const { return piezasSalidas; }
    short* getOcupacion() const { return ocupacion; }
    Estado* getParent()   const { return parent; }
    unsigned short getMovimientoEncoded() const { return movimiento; }

    // Acceso a posiciones (con desempacado). No exponemos el arreglo crudo
    // para que el resto del código no dependa del formato empacado.
    coordenada getPosPieza(int i) const;
    void       setPosPieza(int i, int x, int y);

    void setF(int v)             { f = v; }
    void setH(int v)             { h = v; }
    void setStepUsed(int v)      { stepUsed = v; }
    void setParent(Estado* p)    { parent = p; }
    void setPiezasSalidas(unsigned int ps) { piezasSalidas = ps; }
    void setMovimientoEncoded(unsigned short m) { movimiento = m; }

    // Codifica un par (dirChar, idExterno) en el formato empacado de `movimiento`.
    // dirChar válido: 'U','D','L','R','S'. Cualquier otro valor → código 0 (NONE).
    static unsigned short codificarMovimiento(char dirChar, int piezaExtId);

    // Desempaca el movimiento en (dirChar, idExterno). dirChar='\0' si es NONE.
    void decodificarMovimiento(char& dirChar, int& piezaExtId) const;

    bool piezaYaSalio(int idPieza) const;
    bool jugoTerminado(int numPiezas) const;

    // Hash determinístico. Codifica posiciones, piezasSalidas y stepUsed % dynamicPeriod.
    unsigned int generarHash() const;

    // Igualdad estructural (no compara stepUsed/f/h ni movimiento; sí
    // stepUsed % dynamicPeriod para discriminar fases del ciclo dinámico).
    bool igualA(const Estado& otro) const;

    // Mueve la pieza una celda, actualiza ocupación y suma 1 a stepUsed.
    void moverPieza(int id, int dx, int dy, const Pieza& pieza, int w);

    // Marca la pieza como salida y limpia su ocupación. No incrementa stepUsed.
    void sacarPieza(int id, const Pieza& pieza, int w);

    // Libera el mapa de ocupación anticipadamente. Es seguro tras insertar en closedSet
    // porque la igualdad no depende de ocupacion (es derivable de las posiciones).
    void eliminarOcupacion();

    Estado* clonarYMover(int id, int dx, int dy, const Pieza& pieza, int w) const;
    Estado* clonarYSacar(int id, const Pieza& pieza, int w) const;
};

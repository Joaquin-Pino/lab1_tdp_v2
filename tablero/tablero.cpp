#include "tablero.h"
#include <iostream>
#include <climits>

Tablero::Tablero() : matriz(nullptr), piezas(nullptr), salidas(nullptr),
    compuertas(nullptr), numPiezas(0), numSalidas(0),
    numCompuertas(0), w(0), h(0), stepLimit(0) {}

// Toma ownership de todos los punteros: el Tablero los libera en su destructor.
Tablero::Tablero(celda* matriz, Pieza* piezas, Salida* salidas,
                 Compuerta* compuertas, int numPiezas, int numSalidas,
                 int numCompuertas, int w, int h, int stepLimit)
    : matriz(matriz), piezas(piezas), salidas(salidas), compuertas(compuertas),
      numPiezas(numPiezas), numSalidas(numSalidas), numCompuertas(numCompuertas),
      w(w), h(h), stepLimit(stepLimit) {}

Tablero::Tablero(const Tablero& otro)
    : numPiezas(otro.numPiezas), numSalidas(otro.numSalidas),
      numCompuertas(otro.numCompuertas), w(otro.w), h(otro.h),
      stepLimit(otro.stepLimit) {

    // Copiar la matriz de celdas estáticas (w*h celdas).
    matriz = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        matriz[i] = otro.matriz[i];

    // Copiar cada pieza usando su operator= (que duplica la geometría).
    piezas = new Pieza[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        piezas[i] = otro.piezas[i];

    salidas = new Salida[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        salidas[i] = otro.salidas[i];

    compuertas = new Compuerta[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        compuertas[i] = otro.compuertas[i];
}

Tablero& Tablero::operator=(const Tablero& otro) {
    if (this == &otro) return *this;

    // Liberar memoria actual antes de reasignar.
    delete[] matriz;
    delete[] piezas;
    delete[] salidas;
    delete[] compuertas;

    numPiezas = otro.numPiezas;
    numSalidas = otro.numSalidas;
    numCompuertas = otro.numCompuertas;
    w = otro.w;
    h = otro.h;
    stepLimit = otro.stepLimit;

    matriz = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        matriz[i] = otro.matriz[i];

    piezas = new Pieza[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        piezas[i] = otro.piezas[i];

    salidas = new Salida[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        salidas[i] = otro.salidas[i];

    compuertas = new Compuerta[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        compuertas[i] = otro.compuertas[i];

    return *this;
}

Tablero::~Tablero() {
    delete[] matriz;
    delete[] piezas;
    delete[] salidas;
    delete[] compuertas;
}

celda* Tablero::getMatriz() const { return matriz; }

// Devuelve una copia heap del arreglo de celdas para que el llamador pueda modificarla
// sin afectar el Tablero original (usado por Impresora::imprimirEstado).
celda* Tablero::getCopiaMatriz() const {
    celda* copia = new celda[w * h];
    for (int i = 0; i < w * h; i++)
        copia[i] = matriz[i];
    return copia;
}

Pieza*     Tablero::getPiezas()     const { return piezas; }
Salida*    Tablero::getSalidas()    const { return salidas; }
Compuerta* Tablero::getCompuertas() const { return compuertas; }
int Tablero::getNumPiezas()    const { return numPiezas; }
int Tablero::getNumSalidas()   const { return numSalidas; }
int Tablero::getNumCompuertas()const { return numCompuertas; }
int Tablero::getW()            const { return w; }
int Tablero::getH()            const { return h; }
int Tablero::getStepLimit()    const { return stepLimit; }

bool Tablero::piezaPuedeMoverse(int id, direccion dir, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];

    // Iterar sobre todas las celdas del bounding box de la pieza.
    // Solo procesamos las celdas que realmente forman parte de la pieza (getCelda == true).
    for (int i = 0; i < pieza.getAlto(); i++) {
        for (int j = 0; j < pieza.getAncho(); j++) {
            if (!pieza.getCelda(j, i)) continue;

            // Calcular la celda destino en la dirección deseada.
            int fila = pos.y + i;
            int columna = pos.x + j;

            switch (dir) {
                case ARRIBA: fila--; break;
                case ABAJO: fila++; break;
                case IZQUIERDA: columna--; break;
                case DERECHA: columna++; break;
            }

            // Si la celda destino sale del tablero, el movimiento es inválido.
            if (fila < 0 || fila >= h || columna < 0 || columna >= w)
                return false;

            // Consultar la matriz de ocupación del Estado (no la estática del Tablero)
            // porque refleja las posiciones actuales de todas las piezas.
            short* ocupacion = estado.getOcupacion();
            // Si hay otra pieza diferente en la celda destino, bloqueamos.
            // Ignoramos celdas ocupadas por la misma pieza (id) ya que son celdas
            // que la pieza ya ocupa y que quedarán libres tras el movimiento.
            if (ocupacion[fila * w + columna] != -1 && ocupacion[fila * w + columna] != id)
                return false;

            // Verificar el tipo de celda estática del tablero.
            celda& c = matriz[fila * w + columna];
            if (c.tipo == PARED) return false;

            if (c.tipo == COMPUERTA) {
                // El color de la compuerta se evalúa al momento de llegada (stepUsed + 1),
                // no al stepUsed actual, porque el movimiento incrementará el contador en 1.
                Compuerta& cp = compuertas[c.id];
                int colorActual;
                if (cp.getPaso() == 0) {
                    // Compuerta fija: siempre tiene el color inicial.
                    colorActual = cp.getCi();
                } else {
                    // Calcular en qué punto del ciclo estará la compuerta tras el movimiento.
                    int ciclo = cp.getCf() - cp.getCi() + 1;
                    int pasoEval = estado.getStepUsed() + 1;
                    int pc = pasoEval / cp.getPaso();
                    colorActual = cp.getCi() + (pc % ciclo);
                }
                // El tamaño relevante es la dimensión perpendicular a la dirección de movimiento:
                // movimiento vertical → la pieza debe caber en ancho; horizontal → en alto.
                int tamano = (dir == ARRIBA || dir == ABAJO) ? pieza.getAncho() : pieza.getAlto();

                if (!cp.aceptaBloque(pieza.getColor(), tamano, colorActual))
                    return false;
            }
        }
    }

    return true;
}

bool Tablero::piezaPuedeSalir(int id, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];

    // Revisar la celda inmediatamente por encima de cada celda de la fila superior de la pieza.
    for (int j = 0; j < pieza.getAncho(); j++) {
        if (!pieza.getCelda(j, 0)) continue; // celda no forma parte de la pieza
        
        int fila = pos.y - 1;   // fila justo encima de la pieza
        int columna = pos.x + j;
        
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // Revisar la celda inmediatamente por debajo de la fila inferior.
    for (int j = 0; j < pieza.getAncho(); j++) {
        if (!pieza.getCelda(j, pieza.getAlto()-1)) continue;
        int fila    = pos.y + pieza.getAlto(); // fila justo debajo de la pieza
        int columna = pos.x + j;
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // Revisar la celda a la izquierda de la columna izquierda.
    for (int i = 0; i < pieza.getAlto(); i++) {
        if (!pieza.getCelda(0, i)) continue;
        int fila    = pos.y + i;
        int columna = pos.x - 1; // columna a la izquierda de la pieza
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    // Revisar la celda a la derecha de la columna derecha.
    for (int i = 0; i < pieza.getAlto(); i++) {
        if (!pieza.getCelda(pieza.getAncho()-1, i)) continue;
        int fila    = pos.y + i;
        int columna = pos.x + pieza.getAncho(); // columna a la derecha de la pieza
        if (fila >= 0 && fila < h && columna >= 0 && columna < w)
            if (esSalidaValida(fila, columna, pieza, estado)) return true;
    }

    return false;
}

bool Tablero::esSalidaValida(int fila, int columna,
                              const Pieza& pieza, const Estado& estado) {
    celda& c = matriz[fila * w + columna];
    if (c.tipo != SALIDA) return false;

    Salida& salida = salidas[c.id];

    // Verificar que la salida acepta el color de esta pieza.
    if (salida.getColor() != pieza.getColor()) return false;

    // Calcular el largo actual de la salida según el stepUsed del estado
    // (puede haber oscilado desde el largo inicial del mapa).
    int largoActual = calcularLargoSalida(c.id, estado);

    // La dimensión relevante de la pieza es la que va a entrar en la salida:
    // salida horizontal → la pieza avanza en X, su ancho debe caber en el largo;
    // salida vertical   → la pieza avanza en Y, su alto debe caber en el largo.
    int tamano = salida.getEsHorizontal() ? pieza.getAncho() : pieza.getAlto();

    return salida.aceptaBloque(tamano, largoActual);
}

int Tablero::calcularColorCompuerta(int idx, const Estado& estado) {
    Compuerta& c = compuertas[idx];
    if (c.getPaso() == 0) return c.getCi(); // compuerta estática

    // El color avanza un nivel por cada `paso` steps, ciclando entre Ci y Cf.
    int ciclo = c.getCf() - c.getCi() + 1;
    int pasosCiclo = estado.getStepUsed() / c.getPaso();
    return c.getCi() + (pasosCiclo % ciclo);
}

int Tablero::calcularLargoSalida(int idx, const Estado& estado) const {
    Salida& s = salidas[idx];
    if (s.getLi() == s.getLf()) return s.getLi(); // salida de largo fijo

    // El largo oscila entre Li y Lf en forma de onda triangular:
    // crece de Li a Lf en `rango` pasos, luego vuelve de Lf a Li en otros `rango` pasos.
    // `pos` es la posición dentro del ciclo completo de ida y vuelta (2 * rango pasos).
    int rango = s.getLf() - s.getLi();
    int pasosCiclo = estado.getStepUsed() / s.getPaso();
    int pos = pasosCiclo % (2 * rango);

    if (pos <= rango)
        return s.getLi() + pos;          // fase de crecimiento
    else
        return s.getLf() - (pos - rango); // fase de decrecimiento
}

Estado* Tablero::crearEstadoInicial() const {
    // Copiar las posiciones iniciales de cada pieza desde el Tablero.
    coordenada* posiciones = new coordenada[numPiezas];
    for (int i = 0; i < numPiezas; i++)
        posiciones[i] = piezas[i].getPosInicial();

    // El color inicial de cada compuerta es Ci (el que tiene en el step 0).
    int* coloresCompuertas = new int[numCompuertas];
    for (int i = 0; i < numCompuertas; i++)
        coloresCompuertas[i] = compuertas[i].getCi();

    // El largo inicial de cada salida es Li (el que tiene en el step 0).
    short* largosSalidas = new short[numSalidas];
    for (int i = 0; i < numSalidas; i++)
        largosSalidas[i] = salidas[i].getLi();

    // Construir la matriz de ocupación inicial: -1 en todas las celdas, luego
    // marcar las celdas que ocupa cada pieza con su id.
    short* ocupacion = new short[w * h];
    for (int i = 0; i < w * h; i++)
        ocupacion[i] = -1;

    for (int id = 0; id < numPiezas; id++) {
        Pieza& pieza  = piezas[id];
        coordenada pos = posiciones[id];
        for (int i = 0; i < pieza.getAlto(); i++) {
            for (int j = 0; j < pieza.getAncho(); j++) {
                if (!pieza.getCelda(j, i)) continue;
                int fila    = pos.y + i;
                int columna = pos.x + j;
                ocupacion[fila * w + columna] = id;
            }
        }
    }

    // El constructor de Estado copia internamente todos los arreglos,
    // por lo que podemos liberar los temporales justo después.
    Estado* estado = new Estado(numPiezas, numCompuertas, numSalidas,
                                posiciones, coloresCompuertas, largosSalidas,
                                0, 0, 0, w, h, nullptr, "", ocupacion);
    delete[] posiciones;
    delete[] coloresCompuertas;
    delete[] largosSalidas;
    delete[] ocupacion;

    return estado;
}

void Tablero::imprimir() const {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            celda& c = matriz[i * w + j];
            switch (c.tipo) {
                case VACIA:     std::cout << "."; break;
                case PARED:     std::cout << "#"; break;
                case COMPUERTA: std::cout << "C"; break;
                case PIEZA:     std::cout << "B"; break;
                case SALIDA:    std::cout << "S"; break;
                default:        std::cout << "?"; break;
            }
        }
        std::cout << "\n";
    }
}

bool Tablero::piezaPodriaSalir(Pieza& pieza, Salida& salida) const {
    // Verifica si la pieza podría caber en la salida en el mejor escenario posible,
    // es decir, cuando la salida esté en su largo máximo (max(Li, Lf)).
    // Usado por la heurística para descartar salidas a las que la pieza nunca podrá entrar.
    int tamanoPieza  = salida.getEsHorizontal() ? pieza.getAncho() : pieza.getAlto();
    int largoEventual = (salida.getLi() > salida.getLf()) ? salida.getLi() : salida.getLf();
    return tamanoPieza <= largoEventual;
}

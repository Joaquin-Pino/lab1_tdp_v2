#include "tablero.h"
#include <iostream>
#include <climits>
#include <cctype>

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

            // Las compuertas son impassables con movimiento normal de 1 celda.
            // El cruce se hace con el movimiento de portal (piezaPuedeCruzarCompuerta).
            if (c.tipo == COMPUERTA) return false;

            // verificar que la pieza no pase por salidas, las salidas de otros colores son paredes sólidas.
            if (c.tipo == SALIDA) {
                Salida& s = salidas[c.id];
                if (s.getColor() != pieza.getColor())
                    return false;
            }
        }
    }

    return true;
}

bool Tablero::piezaPuedeSalir(int id, const Estado& estado) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];
    int pw = pieza.getAncho();
    int ph = pieza.getAlto();

    // Iterar sobre cada salida del tablero buscando una que la pieza pueda usar.
    // Se itera por salidas (no por celdas del borde de la pieza) para poder verificar
    // alineación completa: la pieza debe estar TOTALMENTE dentro del rango de la salida,
    // no solo tener una celda del borde solapada.
    for (int idx = 0; idx < numSalidas; idx++) {
        Salida& s = salidas[idx];

        // Filtro rápido por color: descartar salidas de otro color sin más cálculos.
        if (s.getColor() != pieza.getColor()) continue;

        // Largo dinámico de la salida en el step actual del estado.
        int largo = calcularLargoSalida(idx, estado);
        coordenada sp = s.getPos();

        if (s.getEsHorizontal()) {
            // Salida horizontal: ocupa las columnas [sp.x, sp.x + largo - 1] en la fila sp.y.
            // La pieza puede salir DESLIZÁNDOSE VERTICALMENTE a través de ella si:
            //   - Está justo arriba (sp.y == pos.y - 1) → sale hacia arriba
            //   - Está justo abajo (sp.y == pos.y + ph) → sale hacia abajo
            bool adyacente = (sp.y == pos.y - 1) || (sp.y == pos.y + ph);
            if (!adyacente) continue;

            // Alineación completa: el rango de columnas de la pieza [pos.x, pos.x + pw - 1]
            // debe estar ENTERAMENTE contenido en el rango de la salida [sp.x, sp.x + largo - 1].
            // Esto garantiza que toda la pieza puede pasar por la apertura sin chocar con paredes.
            bool alineado = (pos.x >= sp.x) && (pos.x + pw <= sp.x + largo);
            if (alineado) return true;
        } else {
            // Salida vertical: ocupa las filas [sp.y, sp.y + largo - 1] en la columna sp.x.
            // La pieza puede salir DESLIZÁNDOSE HORIZONTALMENTE a través de ella si:
            //   - Está justo a la izquierda (sp.x == pos.x - 1) → sale hacia la izquierda
            //   - Está justo a la derecha (sp.x == pos.x + pw) → sale hacia la derecha
            bool adyacente = (sp.x == pos.x - 1) || (sp.x == pos.x + pw);
            if (!adyacente) continue;

            // Alineación completa: el rango de filas de la pieza [pos.y, pos.y + ph - 1]
            // debe estar enteramente contenido en el rango de la salida [sp.y, sp.y + largo - 1].
            bool alineado = (pos.y >= sp.y) && (pos.y + ph <= sp.y + largo);
            if (alineado) return true;
        }
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

bool Tablero::piezaPuedeCruzarCompuerta(int id, direccion dir, const Estado& estado, int& dxSalto, int& dySalto) {
    Pieza& pieza = piezas[id];
    coordenada pos = estado.getPosPiezas()[id];
    int pw = pieza.getAncho(), ph = pieza.getAlto();
    short* ocupacion = estado.getOcupacion();

    int dx = 0, dy = 0;
    switch (dir) {
        case DERECHA:   dx = 1;  break;
        case IZQUIERDA: dx = -1; break;
        case ABAJO:     dy = 1;  break;
        case ARRIBA:    dy = -1; break;
    }

    int compuertaId = -1;
    int gatePos = -1; // columna (dx≠0) o fila (dy≠0) donde está la compuerta


    // if (!piezaPuedeMoverse(id, dir, estado)) return false;
    // Verificar que el "próximo paso" en dirección dir sea válido:
    // ninguna celda activa debe chocarse con una pared u otra pieza,
    // y al menos una debe dar con una celda de compuerta.
    for (int i = 0; i < ph; i++) {
        for (int j = 0; j < pw; j++) {
            if (!pieza.getCelda(j, i)) continue;

            int nFila = pos.y + i + dy;
            int nCol  = pos.x + j + dx;

            if (nFila < 0 || nFila >= h || nCol < 0 || nCol >= w) return false;

            short ocup = ocupacion[nFila * w + nCol];
            if (ocup != -1 && ocup != id) return false; // otra pieza bloquea

            celda& c = matriz[nFila * w + nCol];
            if (c.tipo == COMPUERTA) {
                compuertaId = c.id;
                gatePos = (dx != 0) ? nCol : nFila;
            }
        }
    }
     // el movimiento normal ya está bloqueado

    if (compuertaId == -1) return false; // no hay compuerta en el próximo paso

    // Verificar que la compuerta acepta la pieza
    Compuerta& cp = compuertas[compuertaId];
    int colorActual;
    if (cp.getPaso() == 0) {
        colorActual = cp.getCi();
    } else {
        int ciclo = cp.getCf() - cp.getCi() + 1;
        int pasoEval = estado.getStepUsed() + 1;
        int pc = pasoEval / cp.getPaso();
        colorActual = cp.getCi() + (pc % ciclo);
    }
    int tamano = (dir == ARRIBA || dir == ABAJO) ? pw : ph;
    if (!cp.aceptaBloque(pieza.getColor(), tamano, colorActual)) return false;

    // Calcular posición de aterrizaje: la pieza queda "pegada" a la compuerta del otro lado
    int newPosX = pos.x, newPosY = pos.y;
    if      (dx > 0) newPosX = gatePos + 1;      // borde izquierdo del bbox justo tras la compuerta
    else if (dx < 0) newPosX = gatePos - pw;      // borde derecho del bbox justo antes de la compuerta (por la izquierda)
    else if (dy > 0) newPosY = gatePos + 1;
    else             newPosY = gatePos - ph;

    // Verificar que la posición de aterrizaje es válida
    for (int i = 0; i < ph; i++) {
        for (int j = 0; j < pw; j++) {
            if (!pieza.getCelda(j, i)) continue;

            int nFila = newPosY + i;
            int nCol  = newPosX + j;

            if (nFila < 0 || nFila >= h || nCol < 0 || nCol >= w) return false;

            short ocup = ocupacion[nFila * w + nCol];
            if (ocup != -1 && ocup != id) return false;

            celda& c = matriz[nFila * w + nCol];
            if (c.tipo == PARED) return false;
            if (c.tipo == COMPUERTA) return false; // no puede aterrizar dentro de otra compuerta
            if (c.tipo == SALIDA && salidas[c.id].getColor() != pieza.getColor()) return false;
        }
    }

    dxSalto = newPosX - pos.x;
    dySalto = newPosY - pos.y;
    return true;
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
                case SALIDA:
                    std::cout << (char)std::toupper(salidas[c.id].getColor());
                    break;
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

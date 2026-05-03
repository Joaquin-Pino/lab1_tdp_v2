#include "verificador.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

Verificador::Verificador(Tablero* t) : tablero(t) {}

Verificador::~Verificador() {}

bool Verificador::parsearToken(const char* str, int& idx,
                                char& dir, int& pieza, int& dist, bool& esSalida) {
    // Saltar espacios y tabuladores entre tokens.
    while (str[idx] == ' ' || str[idx] == '\t') idx++;

    if (str[idx] == '\0') return false; // fin de cadena, no hay más tokens

    char c = str[idx];
    if (c == 'S') {
        // Token de salida: formato "S<id>"
        esSalida = true;
        idx++; // consumir 'S'
        if (str[idx] < '0' || str[idx] > '9') return false; // debe seguir un número
        pieza = 0;
        while (str[idx] >= '0' && str[idx] <= '9') {
            pieza = pieza * 10 + (str[idx] - '0'); // leer dígitos del id
            idx++;
        }
        dir  = 'S';
        dist = 0;
        return true;
    }

    // Token de movimiento: formato "<U|D|L|R><id>,<dist>"
    if (c != 'U' && c != 'D' && c != 'L' && c != 'R') return false;
    dir      = c;
    esSalida = false;
    idx++; // consumir la dirección

    // Leer el id de pieza.
    if (str[idx] < '0' || str[idx] > '9') return false;
    pieza = 0;
    while (str[idx] >= '0' && str[idx] <= '9') {
        pieza = pieza * 10 + (str[idx] - '0');
        idx++;
    }

    // Debe seguir una coma separando el id de la distancia.
    if (str[idx] != ',') return false;
    idx++; // consumir ','

    // Leer la distancia.
    if (str[idx] < '0' || str[idx] > '9') return false;
    dist = 0;
    while (str[idx] >= '0' && str[idx] <= '9') {
        dist = dist * 10 + (str[idx] - '0');
        idx++;
    }
    if (dist <= 0) return false; // distancia cero o negativa es inválida

    return true;
}

bool Verificador::aplicarMovimiento(Estado* estado, char dir, int pieza, int dist) {
    // Buscar el índice interno de la pieza dado su id externo.
    int idxPieza = -1;
    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (tablero->getPiezas()[i].getId() == pieza) {
            idxPieza = i;
            break;
        }
    }
    if (idxPieza == -1) return false;          // id de pieza inexistente
    if (estado->piezaYaSalio(idxPieza)) return false; // pieza ya salió, no puede moverse

    // Convertir el carácter de dirección a la enumeración y deltas x/y.
    direccion d;
    int dx, dy;
    switch (dir) {
        case 'U': d = ARRIBA;    dx = 0;  dy = -1; break;
        case 'D': d = ABAJO;     dx = 0;  dy = 1;  break;
        case 'L': d = IZQUIERDA; dx = -1; dy = 0;  break;
        case 'R': d = DERECHA;   dx = 1;  dy = 0;  break;
        default: return false;
    }

    Pieza& p = tablero->getPiezas()[idxPieza];
    int w    = tablero->getW();

    // Simular los `dist` pasos de 1 celda uno por uno.
    // Esto permite validar cada paso individualmente y actualizar los elementos dinámicos
    // en el momento correcto (igual que lo hace el Solver).
    for (int paso = 0; paso < dist; paso++) {
        if (!tablero->piezaPuedeMoverse(idxPieza, d, *estado)) return false;
        estado->moverPieza(idxPieza, dx, dy, p, w);

        // Actualizar compuertas y salidas después de cada paso individual,
        // porque el stepUsed cambia en cada celda movida.
        for (int i = 0; i < tablero->getNumCompuertas(); i++)
            estado->actualizarCompuerta(i, tablero->calcularColorCompuerta(i, *estado));
        for (int i = 0; i < tablero->getNumSalidas(); i++)
            estado->actualizarSalida(i, tablero->calcularLargoSalida(i, *estado));
    }
    return true;
}

bool Verificador::aplicarSalida(Estado* estado, int pieza) {
    // Buscar el índice interno de la pieza dado su id externo.
    int idxPieza = -1;
    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (tablero->getPiezas()[i].getId() == pieza) {
            idxPieza = i;
            break;
        }
    }
    if (idxPieza == -1) return false;
    if (estado->piezaYaSalio(idxPieza)) return false;       // ya salió
    if (!tablero->piezaPuedeSalir(idxPieza, *estado)) return false; // no está en posición de salida

    Pieza& p = tablero->getPiezas()[idxPieza];
    int w    = tablero->getW();
    estado->sacarPieza(idxPieza, p, w);
    return true;
}

bool Verificador::verificarSolucion(const char* secuencia, const Estado* estadoInicial) {
    // Trabajar sobre una copia del estado inicial para no modificar el original.
    Estado* estado = new Estado(*estadoInicial);

    int idx = 0;
    while (secuencia[idx] != '\0') {
        char dir;
        int  pieza, dist;
        bool esSalida;

        // Leer el siguiente token; si el formato es inválido, la secuencia falla.
        if (!parsearToken(secuencia, idx, dir, pieza, dist, esSalida)) {
            delete estado;
            return false;
        }

        // Aplicar el token al estado actual.
        if (esSalida) {
            if (!aplicarSalida(estado, pieza)) {
                delete estado;
                return false;
            }
        } else {
            if (!aplicarMovimiento(estado, dir, pieza, dist)) {
                delete estado;
                return false;
            }
        }
    }

    // La secuencia es válida solo si todas las piezas salieron Y no se excedió el límite.
    bool valido = estado->jugoTerminado(tablero->getNumPiezas())
               && estado->getStepUsed() <= tablero->getStepLimit();

    delete estado;
    return valido;
}

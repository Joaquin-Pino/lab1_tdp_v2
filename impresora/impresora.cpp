#include "impresora.h"
#include <iostream>

void Impresora::imprimirTablero(const Tablero& Tablero){
    Tablero.imprimir();
}

void Impresora::imprimirEstado(const Tablero& tablero, const Estado& estado) {
    int w = tablero.getW();
    int h = tablero.getH();

    celda* matriz = tablero.getCopiaMatriz();
    Pieza*  piezas  = tablero.getPiezas();
    Salida* salidas = tablero.getSalidas();

    // limpiar celdas de salida de la copia — se remarcarán con largo actual
    for (int i = 0; i < w * h; i++) {
        if (matriz[i].tipo == SALIDA)
            matriz[i] = {VACIA, -1};
    }

    // marcar salidas con largo actual del estado
    for (int id = 0; id < tablero.getNumSalidas(); id++) {
        int largo = tablero.calcularLargoSalida(id, estado);
        Salida& s = salidas[id];
        for (int j = 0; j < largo; j++) {
            int fila    = s.getPos().y + (s.getEsHorizontal() ? 0 : j);  // vertical: y + j
            int columna = s.getPos().x + (s.getEsHorizontal() ? j : 0);  // vertical: x + 0
            if (fila >= 0 && fila < h && columna >= 0 && columna < w) {
                matriz[fila * w + columna].tipo = SALIDA;
                matriz[fila * w + columna].id   = id;
            }
        }
    }

    // colocar piezas en sus posiciones del estado
    for (int i = 0; i < tablero.getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue;
        int x = estado.getPosPiezas()[i].x;
        int y = estado.getPosPiezas()[i].y;
        for (int j = 0; j < piezas[i].getAlto(); j++) {
            for (int k = 0; k < piezas[i].getAncho(); k++) {
                if (!piezas[i].getCelda(k, j)) continue;
                int fila    = y + j;
                int columna = x + k;
                if (fila >= 0 && fila < h && columna >= 0 && columna < w) {
                    matriz[fila * w + columna].tipo = PIEZA;
                    matriz[fila * w + columna].id   = i;
                }
            }
        }
    }

    // imprimir
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            celda& c = matriz[i * w + j];
            switch (c.tipo) {
                case VACIA:     std::cout << "."; break;
                case PARED:     std::cout << "#"; break;
                case SALIDA:    std::cout << "S"; break;
                case COMPUERTA: std::cout << "C"; break;
                case PIEZA:
                    std::cout << (char)piezas[c.id].getColor();
                    break;
                default: std::cout << "?"; break;
            }
        }
        std::cout << "\n";
    }
    std::cout << std::endl;

    delete[] matriz;
}

void Impresora::imprimirSolucion(const Tablero& tablero,
                                  const Estado& estadoInicial,
                                  const char* solucion) {
    if (!solucion) {
        std::cout << "Juego sin solución" << std::endl;
        return;
    }

    std::cout << "Solución: " << solucion << std::endl;
    std::cout << "\nEstado inicial:" << std::endl;
    imprimirEstado(tablero, estadoInicial);

    // parsear la solución y aplicar movimientos
    // formato: R<id>,<celdas> o S<id>
    Estado* estadoActual = new Estado(estadoInicial);
    const char* ptr = solucion;

    while (*ptr != '\0') {
        char dir = *ptr++;
        int id = (*ptr++) - '0';

        if (dir == 'S') {
            // sacar pieza
            estadoActual->setPiezasSalidas(
                estadoActual->getPiezasSalidas() | (1u << id)
            );
            std::cout << "Pieza " << id << " sale:" << std::endl;
            imprimirEstado(tablero, *estadoActual);
        } else {
            // movimiento: saltar la coma
            if (*ptr == ',') ptr++;
            int celdas = atoi(ptr);
            while (*ptr != '\0' && *ptr != 'R' && *ptr != 'L' 
                   && *ptr != 'U' && *ptr != 'D' && *ptr != 'S')
                ptr++;

            // aplicar movimiento
            for (int c = 0; c < celdas; c++) {
                switch (dir) {
                    case 'R': estadoActual->getPosPiezas()[id].x++; break;
                    case 'L': estadoActual->getPosPiezas()[id].x--; break;
                    case 'U': estadoActual->getPosPiezas()[id].y--; break;
                    case 'D': estadoActual->getPosPiezas()[id].y++; break;
                }
            }
            estadoActual->setStepUsed(estadoActual->getStepUsed() + celdas);

            std::cout << "Movimiento " << dir << id << "," << celdas << ":" << std::endl;
            imprimirEstado(tablero, *estadoActual);
        }
    }

    delete estadoActual;
}
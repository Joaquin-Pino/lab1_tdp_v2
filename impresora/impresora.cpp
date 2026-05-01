#include "impresora.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

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

void Impresora::imprimirSolucion(const Tablero& tablero, Estado** solucion){
    (void)tablero;
    if (!solucion) {
        std::cout << "Juego sin solución" << std::endl;
        return;
    }

    int numPasos = 0;
    while (solucion[numPasos] != nullptr) numPasos++;

    if (numPasos == 0) {
        std::cout << "Juego sin solución" << std::endl;
        return;
    }

    // string de solución, consolidando movimientos consecutivos
    // de la misma pieza en la misma direccion
    std::cout << "Solución: ";

    char dirAcum = 0;
    int piezaAcum = -1;
    int countAcum = 0;
    for (int i = 1; i < numPasos; i++) {
        const char* mov = solucion[i]->getMovimiento();
        if (mov[0] == 'S') {
            if (countAcum > 0) {
                std::cout << dirAcum << piezaAcum << "," << countAcum;
                countAcum = 0;
                dirAcum = 0;
                piezaAcum = -1;
            }
            std::cout << mov;
            continue;
        }
        char dir = mov[0];
        int pieza = -1;
        int dist = 1;
        const char* coma = strchr(mov, ',');
        if (coma) {
            pieza = atoi(mov + 1);
            dist = atoi(coma + 1);
        } else {
            pieza = atoi(mov + 1);
        }

        if (dir == dirAcum && pieza == piezaAcum) {
            countAcum += dist;
        } else {
            if (countAcum > 0) {
                std::cout << dirAcum << piezaAcum << "," << countAcum;
            }
            dirAcum = dir;
            piezaAcum = pieza;
            countAcum = dist;
        }
    }
    if (countAcum > 0) {
        std::cout << dirAcum << piezaAcum << "," << countAcum;
    }
    std::cout << std::endl;
}

void Impresora::imprimirSolucionPasoAPaso(const Tablero& tablero, Estado** solucion){
    if (!solucion) {
        std::cout << "Juego sin solución" << std::endl;
        return;
    }

    int numPasos = 0;
    while (solucion[numPasos] != nullptr) numPasos++;

    if (numPasos == 0) {
        std::cout << "Juego sin solución" << std::endl;
        return;
    }

    imprimirSolucion(tablero, solucion);

    for (int i = 0; i < numPasos; i++) {
        if (i == 0)
            std::cout << "Estado inicial:" << std::endl;
        else
            std::cout << "Paso " << i << " - "
                      << solucion[i]->getMovimiento() << ":" << std::endl;
        imprimirEstado(tablero, *solucion[i]);
    }
}
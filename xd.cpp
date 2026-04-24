// ============================================================================
// Nueva implementación de Solver::generarVecinos
// ----------------------------------------------------------------------------
// Estrategia:
//   Para cada pieza activa y cada una de las 4 direcciones, deslizar
//   incrementalmente 1 celda a la vez verificando solo la "frontera nueva"
//   que la pieza ocuparía. Cada avance válido genera UN vecino con costo
//   g + distancia (Opción A: un vecino por cada distancia alcanzable).
//
//   El test de salida (sacarPieza) se hace para la posición actual ANTES
//   de intentar mover (una pieza puede estar ya en condición de salir sin
//   moverse más), y también para cada posición intermedia alcanzada durante
//   el slide (útil si al deslizar la pieza queda adyacente a su salida).
//
// Requiere que:
//   - Estado mantenga `ocupacion` (por ahora; se elimina en la siguiente
//     fase del plan).
//   - Tablero::getMatriz() devuelva el arreglo estático de celdas.
//   - Tablero::calcularColorCompuerta / calcularLargoSalida estén accesibles.
//
// Suposición sobre costo: mover N celdas cuesta N en stepUsed (confirmado).
// ============================================================================

int Solver::generarVecinos(Estado* actual) {
    int count = 0;
    int numPiezas = tablero->getNumPiezas();
    int w         = tablero->getW();
    int hTab      = tablero->getH();
    celda*    matriz     = tablero->getMatriz();
    Pieza*    piezas     = tablero->getPiezas();

    const int dx[4]            = { 0,  0, -1,  1};
    const int dy[4]            = {-1,  1,  0,  0};
    const char dirChar[4]      = {'U','D','L','R'};
    const direccion dirs[4]    = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};

    short* ocupacion = actual->getOcupacion();

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue;

        Pieza& pieza = piezas[id];
        coordenada posOrig = actual->getPosPiezas()[id];
        int pw = pieza.getAncho();
        int ph = pieza.getAlto();

        // ------------------------------------------------------------------
        // 1) ¿La pieza ya puede SALIR desde su posición actual?
        // ------------------------------------------------------------------
        if (tablero->piezaPuedeSalir(id, *actual)) {
            Estado* vecino = new Estado(*actual);
            vecino->sacarPieza(id, pieza, w);

            int hv = calcularHeuristica(*vecino);
            vecino->setH(hv);
            vecino->setF(vecino->getStepUsed() + hv);
            vecino->setParent(actual);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId());
            vecino->setMovimiento(mov);

            vecinosTemp[count++] = vecino;
            // No `continue`: una pieza que puede salir también puede querer
            // moverse primero (ej. para ceder paso). Dejamos que A* elija.
            // Si quieres podar agresivamente, descomenta la siguiente línea:
            // continue;
        }

        // ------------------------------------------------------------------
        // 2) Slide incremental en cada dirección.
        //    Para cada distancia d=1..max, verificar solo la FRONTERA NUEVA
        //    que la pieza ocuparía al avanzar 1 celda más.
        // ------------------------------------------------------------------
        for (int d = 0; d < 4; d++) {
            // Para esta dirección, iteraremos avanzando 1 celda a la vez.
            // Mantenemos la posición "virtual" (sin modificar el estado).
            int curX = posOrig.x;
            int curY = posOrig.y;

            for (int dist = 1; /* break internamente */ ; dist++) {
                int nextX = curX + dx[d];
                int nextY = curY + dy[d];

                // Chequear la frontera nueva: las celdas que la pieza pasaría
                // a ocupar al moverse a (nextX, nextY). Solo las celdas de
                // geometría '1' que no estaban antes en el footprint son
                // realmente nuevas, pero el chequeo se simplifica:
                // para cada celda '1' de la pieza en la nueva posición,
                // verificamos que sea transitable. Las celdas que YA ocupaba
                // la pieza en posición anterior se detectan por "ocupacion == id".
                bool bloqueado = false;
                bool pasaCompuerta = false;  // si la nueva frontera incluye compuerta
                // (reservado por si se quisiera cobrar extra o forzar parada)

                for (int i = 0; i < ph && !bloqueado; i++) {
                    for (int j = 0; j < pw && !bloqueado; j++) {
                        if (!pieza.getCelda(j, i)) continue;

                        int fila    = nextY + i;
                        int columna = nextX + j;

                        // Límites del tablero
                        if (fila < 0 || fila >= hTab || columna < 0 || columna >= w) {
                            bloqueado = true; break;
                        }

                        int linearIdx = fila * w + columna;

                        // Ocupada por otra pieza (id distinto y no vacía).
                        short ocup = ocupacion[linearIdx];
                        if (ocup != -1 && ocup != id) {
                            bloqueado = true; break;
                        }

                        // Obstáculos estáticos
                        celda& c = matriz[linearIdx];
                        if (c.tipo == PARED) {
                            bloqueado = true; break;
                        }

                        if (c.tipo == COMPUERTA) {
                            // Compuerta: verificar color actual y tamaño.
                            // stepUsed del vecino provisional = actual.stepUsed + dist
                            // pero calcularColorCompuerta usa estado.getStepUsed().
                            // Usamos el del estado actual + dist manualmente:
                            int pasoActual = actual->getStepUsed() + dist;
                            Compuerta& cp = tablero->getCompuertas()[c.id];
                            int colorCompuerta;
                            if (cp.getPaso() == 0) {
                                colorCompuerta = cp.getCi();
                            } else {
                                int ciclo = cp.getCf() - cp.getCi() + 1;
                                int pc = pasoActual / cp.getPaso();
                                colorCompuerta = cp.getCi() + (pc % ciclo);
                            }
                            int tamanoBloque = (d == ARRIBA || d == ABAJO) ? pw : ph;
                            if (!cp.aceptaBloque(pieza.getColor(), tamanoBloque, colorCompuerta)) {
                                bloqueado = true; break;
                            }
                            pasaCompuerta = true;
                        }

                        // Si es SALIDA en una celda que ahora ocuparía la pieza,
                        // tratamos la salida como VACIA para movimiento
                        // (la detección de "salir" es independiente).
                    }
                }

                if (bloqueado) break;

                // Avance válido: generar vecino en (nextX, nextY) con costo +dist.
                curX = nextX;
                curY = nextY;

                Estado* vecino = new Estado(*actual);
                // Mover la pieza 1 celda delta = (dx, dy) repetido `dist` veces
                // equivale a moverla (dx*dist, dy*dist) en un solo paso.
                // Pero moverPieza incrementa stepUsed en 1 — hay que corregirlo.
                vecino->moverPieza(id, dx[d] * dist, dy[d] * dist, pieza, w);
                // moverPieza sumó 1 a stepUsed; ajustamos a +dist total:
                vecino->setStepUsed(actual->getStepUsed() + dist);

                // Actualizar compuertas y salidas (dependen de stepUsed).
                // TODO (fase 2): eliminar estos arreglos del Estado y calcular
                // bajo demanda. Por ahora mantener consistencia.
                for (int i = 0; i < tablero->getNumCompuertas(); i++)
                    vecino->actualizarCompuerta(i,
                        tablero->calcularColorCompuerta(i, *vecino));
                for (int i = 0; i < tablero->getNumSalidas(); i++)
                    vecino->actualizarSalida(i,
                        tablero->calcularLargoSalida(i, *vecino));

                int hv = calcularHeuristica(*vecino);
                vecino->setH(hv);
                vecino->setF(vecino->getStepUsed() + hv);
                vecino->setParent(actual);

                char mov[10];
                snprintf(mov, 10, "%c%d,%d", dirChar[d], pieza.getId(), dist);
                vecino->setMovimiento(mov);

                vecinosTemp[count++] = vecino;

                // Chequeo de capacidad del buffer (por seguridad, tras ampliar
                // maxVecinos en el constructor).
                if (count >= maxVecinos) return count;
            }
        }
    }
    return count;
}
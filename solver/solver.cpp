// solver.cpp
#include "solver.h"
#include <cstring>
#include <iostream>

Solver::Solver(Tablero* t)
    : tablero(t), openSet(nullptr), closedSet(nullptr), vecinosTemp(nullptr) {
    int maxDim  = (t->getW() > t->getH()) ? t->getW() : t->getH();
    // En el peor caso, cada pieza puede salir (1 opción) o moverse en 4 direcciones
    // hasta el borde del tablero (maxDim pasos). Multiplicamos por el número de piezas.
    maxVecinos = t->getNumPiezas() * (4 * maxDim + 1);
    openSet = new MinHeap(1024);
    closedSet = new TablaHash(100003); // primo grande para distribuir bien los hashes iniciales
    vecinosTemp = new Estado*[maxVecinos];
}

Solver::~Solver() {
    // Vaciar openSet: los estados que quedaron sin expandir deben liberarse aquí
    // porque el MinHeap no es dueño de ellos.
    while (!openSet->estaVacio()) {
        Estado* e = openSet->pop();
        delete e;
    }
    // El closedSet SÍ es dueño de sus estados; liberarEstados() los borra todos.
    closedSet->liberarEstados();
    delete openSet;
    delete closedSet;
    delete[] vecinosTemp;
}

void Solver::prepararVecino(Estado* vecino, Estado* actual, char mov[10]) const {
    // Actualizar el color de cada compuerta según el stepUsed del vecino
    // (puede haber cambiado si el nuevo step cruzó un múltiplo del `paso`).
    for (int i = 0; i < tablero->getNumCompuertas(); i++)
        vecino->actualizarCompuerta(i, tablero->calcularColorCompuerta(i, *vecino));

    // Actualizar el largo de cada salida por la misma razón.
    for (int i = 0; i < tablero->getNumSalidas(); i++)
        vecino->actualizarSalida(i, tablero->calcularLargoSalida(i, *vecino));

    // Calcular h y f para que el heap pueda ordenar este vecino correctamente.
    int hv = calcularHeuristica(*vecino);
    vecino->setH(hv);
    vecino->setF(vecino->getStepUsed() + hv);

    // Registrar el estado padre para poder reconstruir el camino al llegar a la meta.
    vecino->setParent(actual);
    vecino->setMovimiento(mov);
}

int Solver::generarVecinos(Estado* actual) {
    int   count    = 0;
    int   numPiezas = tablero->getNumPiezas();
    int   w         = tablero->getW();
    Pieza* piezas   = tablero->getPiezas();

    // Tablas paralelas de las 4 direcciones para evitar repetir el switch en cada iteración.
    direccion dirs[4] = {ARRIBA, ABAJO, IZQUIERDA, DERECHA};
    int dx[4] = { 0,  0, -1,  1};
    int  dy[4] = {-1,  1,  0,  0};
    char dirChar[4] = {'U', 'D', 'L', 'R'};

    for (int id = 0; id < numPiezas; id++) {
        if (actual->piezaYaSalio(id)) continue; // pieza ya fuera del tablero, ignorar
        Pieza& pieza = piezas[id];

        if (tablero->piezaPuedeSalir(id, *actual)) {
            // Si la pieza puede salir ahora, generamos solo ese vecino y pasamos a la siguiente.
            // No generamos también movimientos: sacar es siempre preferible a seguir moviendo.
            Estado* vecino = actual->clonarYSacar(id, pieza, w);

            char mov[10];
            snprintf(mov, 10, "S%d", pieza.getId()); // formato de salida: "S<idExterno>"
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count; // protección contra overflow del buffer
            continue;
        }

        // Intentar mover la pieza 1 celda en cada dirección.
        for (int d = 0; d < 4; d++) {
            if (!tablero->piezaPuedeMoverse(id, dirs[d], *actual)) continue;

            Estado* vecino = actual->clonarYMover(id, dx[d], dy[d], pieza, w);

            char mov[10];
            snprintf(mov, 10, "%c%d,1", dirChar[d], pieza.getId()); // formato: "D<idExterno>,1"
            prepararVecino(vecino, actual, mov);

            vecinosTemp[count++] = vecino;
            if (count >= maxVecinos) return count;
        }
    }
    return count;
}

Estado** Solver::reconstruirCamino(Estado* final) {
    // Primera pasada: contar la cantidad de estados en la cadena parent.
    int numPasos = 0;
    Estado* actual = final;
    while (actual != nullptr) {
        numPasos++;
        actual = actual->getParent();
    }

    // Asignar el arreglo con un slot extra para el centinela nullptr al final.
    Estado** camino = new Estado*[numPasos + 1];

    // Segunda pasada: copiar los estados en orden cronológico (del inicial al final).
    // Seguimos la cadena de nuevo pero llenamos el arreglo de atrás hacia adelante.
    actual = final;
    for (int i = numPasos - 1; i >= 0; i--) {
        camino[i] = new Estado(*actual);    // copiar para que el camino sea independiente
        camino[i]->setParent(nullptr);      // romper la cadena en la copia
        actual = actual->getParent();
    }

    camino[numPasos] = nullptr; // centinela que indica el fin del arreglo
    return camino;
}

Estado** Solver::resolver(Estado* estadoInicial) {
    // Inicializar el estado inicial con su heurística y encolarlo.
    int h = calcularHeuristica(*estadoInicial);
    estadoInicial->setH(h);
    estadoInicial->setF(estadoInicial->getStepUsed() + h);
    estadoInicial->setParent(nullptr);
    openSet->push(estadoInicial);

    while (!openSet->estaVacio()) {
        Estado* actual = openSet->pop(); // extraer el estado con menor f

        // Si ya está en closedSet, fue procesado con un camino más corto anteriormente.
        // Descartar este duplicado.
        if (closedSet->existe(actual)) {
            delete actual;
            continue;
        }

        // Verificar si llegamos a la meta (todas las piezas salidas).
        if (actual->jugoTerminado(tablero->getNumPiezas())) {
            Estado** camino = reconstruirCamino(actual);
            delete actual; // el estado meta ya fue copiado en reconstruirCamino
            return camino;
        }

        // Poda por presupuesto: si ya usamos todos los steps disponibles,
        // este estado no puede generar vecinos válidos.
        if (actual->getStepUsed() >= tablero->getStepLimit()) {
            delete actual;
            continue;
        }

        // Insertar en closedSet antes de expandir para que los vecinos que generen
        // el mismo estado no lo inserten en openSet si ya quedó marcado como explorado.
        closedSet->insertar(actual);

        // Margen para la poda por f: tolera que la heurística sobreestime
        // hasta numPiezas/2 + 1 pasos (proporcional al término de bloqueos/2).
        int margen = tablero->getNumPiezas() / 2 + 1;

        int numVecinos = generarVecinos(actual);
        for (int i = 0; i < numVecinos; i++) {
            // Poda por g: el vecino ya excedió el presupuesto.
            if (vecinosTemp[i]->getStepUsed() > tablero->getStepLimit()) {
                delete vecinosTemp[i];
                continue;
            }
            // Poda por f con margen: el vecino no puede llegar a la meta dentro del presupuesto.
            if (vecinosTemp[i]->getF() > tablero->getStepLimit() + margen) {
                delete vecinosTemp[i];
                continue;
            }
            // Solo encolar si no está ya en closedSet.
            if (!closedSet->existe(vecinosTemp[i]))
                openSet->push(vecinosTemp[i]);
            else
                delete vecinosTemp[i];
        }

        // Liberar la matriz de ocupación del estado ya insertado en closedSet.
        // Esto ahorra cientos de bytes por estado en el closed set, que puede tener millones.
        // El estado sigue siendo útil para la reconstrucción del camino vía parent.
        actual->eliminarOcupacion();
    }

    return nullptr; // openSet vacío sin encontrar la meta: sin solución
}

int Solver::calcularHeuristica(const Estado& estado) const {
    int total = 0;

    for (int i = 0; i < tablero->getNumPiezas(); i++) {
        if (estado.piezaYaSalio(i)) continue; // pieza ya fuera, no contribuye al costo restante

        coordenada pos = estado.getPosPiezas()[i];
        Pieza& pieza   = tablero->getPiezas()[i];
        int pw = pieza.getAncho();
        int ph = pieza.getAlto();

        int mejorCosto = -1; // -1 significa que no se encontró ninguna salida compatible aún

        for (int j = 0; j < tablero->getNumSalidas(); j++) {
            Salida& salida = tablero->getSalidas()[j];
            if (salida.getColor() != pieza.getColor()) continue;        // color incompatible
            if (!tablero->piezaPodriaSalir(pieza, salida)) continue;    // pieza nunca cabrá

            coordenada ps = salida.getPos();

            // Calcular la distancia Manhattan desde el BORDE de la pieza hasta la salida,
            // no desde su centro. Si la salida está dentro del bounding box, la distancia es 0.
            int dx = 0, dy = 0;
            if  (ps.x < pos.x) {      
                dx = pos.x - ps.x;// salida a la izquierda
            } else if (ps.x > pos.x+pw-1)  {
                dx = ps.x - (pos.x + pw-1); // salida a la derecha
            }

            if (ps.y < pos.y) {
                dy = pos.y - ps.y;  // salida arriba
            } else if (ps.y > pos.y+ph-1)  {
                dy = ps.y - (pos.y + ph-1); // salida abajo
            }

            int dist = dx + dy;

            // Sumar penalización por piezas que bloquean el camino hacia esta salida.
            // Se divide entre 2 porque cada pieza bloqueante ya está siendo contada
            // en su propia heurística, y contarla doble sobreestimaría el costo total.
            int bloqueos = contarBloqueos(i, pos, ps, estado);
            int costo    = dist + bloqueos / 2;

            // Quedarse con la salida que da el menor costo estimado.
            if (mejorCosto == -1 || costo < mejorCosto)
                mejorCosto = costo;
        }

        // Si no hay ninguna salida viable, contribuir con 0 (mejor que bloquear la búsqueda).
        if (mejorCosto == -1) mejorCosto = 0;

        total += mejorCosto;
    }
    return total;
}

int Solver::contarBloqueos(int idPieza, coordenada pos,
                             const Estado& estado) const {
    // Versión sin salida explícita: encontrar primero la salida más cercana por Manhattan
    // y delegar en la versión con salida.
    Pieza& pieza = tablero->getPiezas()[idPieza];
    coordenada mejorSalida = {0, 0};
    int mejorDist = -1;
    for (int j = 0; j < tablero->getNumSalidas(); j++) {
        Salida& salida = tablero->getSalidas()[j];
        
        if (salida.getColor() != pieza.getColor()) continue;
        if (!tablero->piezaPodriaSalir(pieza, salida)) continue;
        
        int d = abs(pos.x - salida.getPos().x) + abs(pos.y - salida.getPos().y);
        if (mejorDist == -1 || d < mejorDist) {
            mejorDist  = d;
            mejorSalida = salida.getPos();
        }
    }
    if (mejorDist <= 0) return 0;
    return contarBloqueos(idPieza, pos, mejorSalida, estado);
}

int Solver::contarBloqueos(int idPieza, coordenada pos, coordenada posSalida,
                             const Estado& estado) const {
    int w = tablero->getW();
    short* ocupacion = estado.getOcupacion();
    celda* matriz = tablero->getMatriz();

    // Camino 1: ir primero en horizontal hasta posSalida.x, luego en vertical hasta posSalida.y.
    int bloqueosHorizPrimero = 0;
    {
        // Tramo horizontal: avanzar desde pos.x hacia posSalida.x a lo largo de la fila pos.y.
        int xIni   = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin   = posSalida.x;
        int paso   = pos.x < posSalida.x ? 1 : -1;
        short ultimoId = idPieza; // para no contar la misma pieza bloqueante más de una vez
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[pos.y * w + x];
            // Contar solo piezas distintas de la pieza actual y del último bloqueante detectado.
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza; // resetear al encontrar celda libre
            }
            celda& c = matriz[pos.y * w + x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; } // pared infranqueable
        }
        // Tramo vertical: avanzar desde pos.y hacia posSalida.y a lo largo de la columna posSalida.x.
        int yIni  = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin  = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        ultimoId  = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + posSalida.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosHorizPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + posSalida.x];
            if (c.tipo == PARED) { bloqueosHorizPrimero += 2; break; }
        }
    }

    // Camino 2: ir primero en vertical hasta posSalida.y, luego en horizontal hasta posSalida.x.
    int bloqueosVertPrimero = 0;
    {
        // Tramo vertical: avanzar desde pos.y hacia posSalida.y a lo largo de la columna pos.x.
        int yIni  = pos.y < posSalida.y ? pos.y + 1 : pos.y - 1;
        int yFin  = posSalida.y;
        int pasoY = pos.y < posSalida.y ? 1 : -1;
        short ultimoId = idPieza;
        for (int y = yIni; y != yFin + pasoY; y += pasoY) {
            if (y < 0 || y >= tablero->getH()) break;
            short ocup = ocupacion[y * w + pos.x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[y * w + pos.x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
        // Tramo horizontal: avanzar desde pos.x hacia posSalida.x a lo largo de la fila posSalida.y.
        int xIni = pos.x < posSalida.x ? pos.x + 1 : pos.x - 1;
        int xFin = posSalida.x;
        int paso = pos.x < posSalida.x ? 1 : -1;
        ultimoId = idPieza;
        for (int x = xIni; x != xFin + paso; x += paso) {
            if (x < 0 || x >= w) break;
            short ocup = ocupacion[posSalida.y * w + x];
            if (ocup != -1 && ocup != idPieza && ocup != ultimoId) {
                bloqueosVertPrimero++;
                ultimoId = ocup;
            } else if (ocup == -1 || ocup == idPieza) {
                ultimoId = idPieza;
            }
            celda& c = matriz[posSalida.y * w + x];
            if (c.tipo == PARED) { bloqueosVertPrimero += 2; break; }
        }
    }

    // Devolver el camino con menos bloqueos: tomar el recorrido más despejado
    // para no sobreestimar más de lo necesario.
    return bloqueosHorizPrimero < bloqueosVertPrimero ? bloqueosHorizPrimero : bloqueosVertPrimero;
}
